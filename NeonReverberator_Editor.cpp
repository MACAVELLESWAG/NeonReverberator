#include "NeonReverberator_Editor.h"

MatrixRainOverlay::MatrixRainOverlay()
{
    startTimerHz(25);
    for (int i = 0; i < 40; ++i)
    {
        drops.add({ (float)rng.nextInt(800), (float)rng.nextInt(300), 
                    2.0f + rng.nextFloat() * 4.0f,
                    juce::String::charToString((juce::juce_wchar)(0x30A0 + rng.nextInt(96))) });
    }
}

void MatrixRainOverlay::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lime.withAlpha(0.07f));
    for (auto& d : drops)
    {
        g.setFont(14.0f);
        g.drawText(d.text, (int)d.x, (int)d.y, 14, 14, juce::Justification::centred);
    }
}

void MatrixRainOverlay::timerCallback()
{
    for (auto& d : drops)
    {
        d.y += d.speed;
        if (d.y > getHeight()) d.y = -20;
        if (rng.nextFloat() < 0.08f)
            d.text = juce::String::charToString((juce::juce_wchar)(0x30A0 + rng.nextInt(96)));
    }
    repaint();
}

//==============================================================================
NeonReverberatorAudioProcessorEditor::NeonReverberatorAudioProcessorEditor(
    NeonReverberatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(400, 280);

    auto setupSlider = [&](juce::Slider& slider, const juce::String& paramID)
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);
    };

    setupSlider(decaySlider, "DECAY");
    setupSlider(dampingSlider, "DAMPING");
    setupSlider(preDelaySlider, "PREDELAY");
    setupSlider(mixSlider, "MIX");
    setupSlider(sizeSlider, "SIZE");
    setupSlider(glitchSlider, "GLITCH");

    decayAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DECAY", decaySlider);
    dampingAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DAMPING", dampingSlider);
    preDelayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PREDELAY", preDelaySlider);
    mixAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);
    sizeAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "SIZE", sizeSlider);
    glitchAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "GLITCH", glitchSlider);

    addAndMakeVisible(rainOverlay);
    rainOverlay.setBounds(getLocalBounds());
    rainOverlay.toBack();
}

NeonReverberatorAudioProcessorEditor::~NeonReverberatorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void NeonReverberatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0x0a0a1f));

    // faint neon grid
    g.setColour(juce::Colour(0x00f7ff).withAlpha(0.08f));
    for (int x = 0; x < getWidth(); x += 40)
        g.drawVerticalLine(x, 0.0f, (float)getHeight());
    for (int y = 0; y < getHeight(); y += 40)
        g.drawHorizontalLine(y, 0.0f, (float)getWidth());
}

void NeonReverberatorAudioProcessorEditor::resized()
{
    rainOverlay.setBounds(getLocalBounds());

    const int w = 52;
    const int startX = 35;
    int y = 55;

    decaySlider.setBounds(startX, y, w, 160);
    dampingSlider.setBounds(startX + 72, y, w, 160);
    preDelaySlider.setBounds(startX + 144, y, w, 160);

    y = 235;
    sizeSlider.setBounds(startX, y, w, 160);
    mixSlider.setBounds(startX + 72, y, w, 160);
    glitchSlider.setBounds(startX + 144, y, w, 160);
}