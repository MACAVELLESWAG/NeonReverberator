#include "NeonReverberator_Processor.h"
#include "NeonReverberator_Editor.h"

NeonReverberatorAudioProcessor::NeonReverberatorAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

NeonReverberatorAudioProcessor::~NeonReverberatorAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
NeonReverberatorAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay",
        juce::NormalisableRange<float>(0.1f, 8.0f, 0.01f), 2.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("DAMPING", "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("PREDELAY", "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("SIZE", "Size",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("GLITCH", "Glitch",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    return layout;
}

void NeonReverberatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, 2 };

    reverb.prepare(spec);
    preDelayL.prepare(spec);
    preDelayR.prepare(spec);
    glitchDelayL.prepare(spec);
    glitchDelayR.prepare(spec);

    preDelayL.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.2 + 512));
    preDelayR.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.2 + 512));
    glitchDelayL.setMaximumDelayInSamples(1024);
    glitchDelayR.setMaximumDelayInSamples(1024);

    reverb.reset();
    preDelayL.reset();
    preDelayR.reset();
    glitchDelayL.reset();
    glitchDelayR.reset();
}

void NeonReverberatorAudioProcessor::releaseResources() {}

bool NeonReverberatorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void NeonReverberatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getBypassParameter() && getBypassParameter()->getValue() > 0.5f)
        return;

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    const float mix     = *apvts.getRawParameterValue("MIX") / 100.0f;
    const float size    = *apvts.getRawParameterValue("SIZE");
    const float damping = *apvts.getRawParameterValue("DAMPING");
    const float predelayMs = *apvts.getRawParameterValue("PREDELAY");
    const float glitch  = *apvts.getRawParameterValue("GLITCH");

    juce::dsp::Reverb::Parameters params;
    params.roomSize = size;
    params.damping = damping;
    params.wetLevel = 1.0f;
    params.dryLevel = 0.0f;
    params.width = 1.0f;
    reverb.setParameters(params);

    const float preDelaySamples = static_cast<float>(getSampleRate() * predelayMs / 1000.0);

    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right[i];

        // Pre-delay
        float delayedL = preDelayL.popSample(0, preDelaySamples);
        float delayedR = preDelayR.popSample(0, preDelaySamples);
        preDelayL.pushSample(0, inL);
        preDelayR.pushSample(0, inR);

        // Glitch
        glitchUpdateCounter++;
        if (glitch > 0.0f && glitchUpdateCounter >= 441)
        {
            glitchUpdateCounter = 0;
            float gDelay = 1.0f + rng.nextFloat() * 5.0f * glitch;
            glitchDelayL.setDelay(gDelay);
            glitchDelayR.setDelay(gDelay);
        }

        // === Correct block processing for JUCE 8+ ===
        juce::AudioBuffer<float> reverbBuffer(2, 1);
        reverbBuffer.setSample(0, 0, delayedL);
        reverbBuffer.setSample(1, 0, delayedR);

        juce::dsp::AudioBlock<float> block(reverbBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        reverb.process(context);

        float wetL = reverbBuffer.getSample(0, 0);
        float wetR = reverbBuffer.getSample(1, 0);

        // Glitch feedback
        if (glitch > 0.0f)
        {
            float gL = glitchDelayL.popSample(0);
            float gR = glitchDelayR.popSample(0);
            glitchDelayL.pushSample(0, wetL * 0.18f * glitch);
            glitchDelayR.pushSample(0, wetR * 0.18f * glitch);
            wetL = wetL * 0.82f + gL;
            wetR = wetR * 0.82f + gR;
        }

        const float dryMix = 1.0f - mix;
        left[i]  = inL * dryMix + wetL * mix;
        right[i] = inR * dryMix + wetR * mix;
    }
}

juce::AudioProcessorEditor* NeonReverberatorAudioProcessor::createEditor()
{
    return new NeonReverberatorAudioProcessorEditor(*this);
}

void NeonReverberatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NeonReverberatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeonReverberatorAudioProcessor();
}