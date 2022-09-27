/*
  ==============================================================================

  ==============================================================================
*/

#pragma once

class MyTwoValueSlider : public juce::Slider
{
public:
    MyTwoValueSlider(juce::Slider::SliderStyle style_, juce::RangedAudioParameter* minpar, juce::RangedAudioParameter* maxpar) :
        minAttach(*minpar, [this](float x) { setMinValue(x, juce::dontSendNotification); }),
        maxAttach(*maxpar, [this](float x) { setMaxValue(x, juce::dontSendNotification); })
    {
        setTextBoxStyle(NoTextBox, true, 0, 0);
        setSliderStyle(style_);
        setRange(minpar->getNormalisableRange().start, maxpar->getNormalisableRange().end);
        setSkewFactorFromMidPoint(3000.0);
        maxAttach.sendInitialUpdate();
        minAttach.sendInitialUpdate();
        
        onDragStart = [this]()
        {
            thumbThatWasDragged = getThumbBeingDragged();
            if (thumbThatWasDragged == 1)
                minAttach.beginGesture();
            else if (thumbThatWasDragged == 2)
                maxAttach.beginGesture();
        };
        onValueChange = [this]()
        {
            if (thumbThatWasDragged == 1)
                minAttach.setValueAsPartOfGesture(getMinValue());
            else if (thumbThatWasDragged == 2)
                maxAttach.setValueAsPartOfGesture(getMaxValue());
        };
        onDragEnd = [this]()
        {
            if (thumbThatWasDragged == 1)
                minAttach.endGesture();
            else if (thumbThatWasDragged == 2)
                maxAttach.endGesture();
        };
    }
private:
    juce::ParameterAttachment minAttach;
    juce::ParameterAttachment maxAttach;
    int thumbThatWasDragged = 0;
    CustomTwoValSliderLaf twoValLaf;
};

class MySlider : public juce::Slider
{
public:
    MySlider(juce::RangedAudioParameter* rapSlider, juce::String paraSuffix) : slider(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), attach(*rapSlider, slider, nullptr)
    
    {
        slider.setTextValueSuffix (paraSuffix);
        slider.setLookAndFeel(&customLafDial);

    };
    
    ~MySlider()
    {
        slider.setLookAndFeel(nullptr);
    }
    
    juce::Slider slider;
    juce::SliderParameterAttachment attach;
    CustomDial customLafDial;
private:
    
   
};

class MyButton : public juce::TextButton
{
public:
    MyButton(juce::RangedAudioParameter* rap, juce::String buttonName, juce::Slider& sliderSync, juce::Slider& sliderTime ) : button(buttonName), attach(*rap, button, nullptr)
    
    {
        button.setLookAndFeel(&customButton);
        button.setClickingTogglesState(true);
        button.onClick = [&]()
        {
            const auto message = button.getToggleState() ? "Sync" : "Time";
            button.setButtonText(message);
            sliderTime.setVisible(!button.getToggleState());
            sliderSync.setVisible(button.getToggleState());
        };
    };
    
    ~MyButton()
    {
        button.setLookAndFeel(nullptr);
    }
    
    juce::TextButton button;
    juce::ButtonParameterAttachment attach;
    CustomButtonLaf customButton;
private:
    
 
};


