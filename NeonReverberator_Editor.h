#pragma once
#include <JuceHeader.h>

class MatrixRainOverlay : public juce::Component, private juce::Timer
{
public:
    MatrixRainOverlay();
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
private:
    struct Drop { float x, y, speed; juce::String text; };
    juce::Array<Drop> drops;
    juce::Random rng;
};

class NeonReverberatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NeonReverberatorAudioProcessorEditor(NeonReverberatorAudioProcessor&);
    ~NeonReverberatorAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NeonReverberatorAudioProcessor& audioProcessor;
    NeonLookAndFeel customLookAndFeel;

    juce::Slider decaySlider, dampingSlider, preDelaySlider, mixSlider, sizeSlider, glitchSlider;
    juce::Label decayLabel, dampingLabel, preDelayLabel, mixLabel, sizeLabel, glitchLabel;
    juce::Label titleLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        decayAtt, dampingAtt, preDelayAtt, mixAtt, sizeAtt, glitchAtt;

    MatrixRainOverlay rainOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonReverberatorAudioProcessorEditor)
};