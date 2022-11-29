/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Laf.h"
#include "Params.h"

//==============================================================================
/**
*/

class EchoDlineAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EchoDlineAudioProcessorEditor (EchoDlineAudioProcessor&);
    ~EchoDlineAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    juce::Label hiCutLabel, loCutLabel, feedbackLabel, driveLabel, mixLabel, title, pitchLabel, flutterLabel;
    EchoDlineAudioProcessor& audioProcessor;
    MyTwoValueSlider twoValueSlider;
    CustomTwoValSliderLaf twoValLaf;
    MySlider syncedTimeSlider, timeSlider, mixSlider, feedbackSlider, driveSlider, pitchIntSlider, flutterSlider;
    MyButton syncToggle, pingpongToggle, reverseToggle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EchoDlineAudioProcessorEditor)
};

