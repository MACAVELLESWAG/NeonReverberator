#include "LookAndFeel.h"

NeonLookAndFeel::NeonLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0x00f7ff));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff00aa).withAlpha(0.3f));
    setColour(juce::Label::textColourId, juce::Colour(0x00ffaa));
    neonShadow = juce::DropShadow(juce::Colour(0x00f7ff), 8, {0, 0});
}

void NeonLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float, float,
                                       const juce::Slider::SliderStyle, juce::Slider& slider)
{
    // slim glowing bar
    auto track = juce::Rectangle<float>(x + width * 0.35f, y, width * 0.3f, height);
    g.setColour(juce::Colour(0x00f7ff).withAlpha(0.2f));
    g.fillRect(track);

    // filled portion
    float pos = sliderPos;
    auto fill = track.removeFromBottom(pos);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0x00f7ff), fill.getTopLeft(),
                                           juce::Colour(0xff00aa), fill.getBottomRight(), false));
    g.fillRect(fill);

    // holographic knob cap
    neonShadow.drawForRectangle(g, fill.removeFromTop(12).reduced(2).toNearestInt());
    g.setColour(juce::Colours::white);
    g.fillEllipse(fill.getCentreX() - 8, fill.getY() - 8, 16, 16);
}

void NeonLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(juce::Colour(0x00ffaa));
    g.setFont(juce::Font("Monospace", 13.0f, juce::Font::plain));
    g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred, false);
    // subtle scanline
    g.setColour(juce::Colours::white.withAlpha(0.03f));
    for (int i = 0; i < label.getHeight(); i += 3)
        g.drawHorizontalLine(i, 0, (float)label.getWidth());
}