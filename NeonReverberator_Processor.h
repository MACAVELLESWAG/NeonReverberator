#pragma once
#include <JuceHeader.h>

class NeonReverberatorAudioProcessor : public juce::AudioProcessor,
                                       private juce::ValueTree::Listener
{
public:
    NeonReverberatorAudioProcessor();
    ~NeonReverberatorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Neon Reverberator"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {}

    juce::dsp::Reverb reverb;

    // Fixed narrowing conversion
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayL{static_cast<int>(44100 * 0.2 + 512)};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayR{static_cast<int>(44100 * 0.2 + 512)};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> glitchDelayL{512};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> glitchDelayR{512};

    juce::Random rng;
    int glitchUpdateCounter = 0;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonReverberatorAudioProcessor)
};