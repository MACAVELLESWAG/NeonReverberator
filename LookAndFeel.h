#pragma once
#include <JuceHeader.h>

class NeonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NeonLookAndFeel();

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float min, float max,
                          const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawLabel(juce::Graphics&, juce::Label&) override;

private:
    juce::DropShadow neonShadow;
};