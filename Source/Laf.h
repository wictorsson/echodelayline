/*
  ==============================================================================
    Laf.h
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CustomDial : public juce::LookAndFeel_V4
{
public:
    CustomDial();
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    
    juce::Label* createSliderTextBox (juce::Slider& slider) override;
private:
  
    juce::DropShadow shadowProperties;
    juce::DropShadowEffect dialShadow;
};

class CustomButtonLaf : public juce::LookAndFeel_V4

{
public:
  
    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override;
};

class CustomTwoValSliderLaf : public juce::LookAndFeel_V4

{
public:
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider&) override;
};
