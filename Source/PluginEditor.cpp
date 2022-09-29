/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EchoDlineAudioProcessorEditor::EchoDlineAudioProcessorEditor (EchoDlineAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), twoValueSlider(juce::Slider::SliderStyle::TwoValueHorizontal, p.apvts.getParameter("hp"), p.apvts.getParameter("lp")), syncedTimeSlider(p.apvts.getParameter("choice"), ""), timeSlider(p.apvts.getParameter("delayTime"), " ms"), mixSlider(p.apvts.getParameter("mix"), " %"), feedbackSlider(p.apvts.getParameter("feedback"), " %"), driveSlider(p.apvts.getParameter("drive"), ""),syncToggle(p.apvts.getParameter("sync"), "Sync", syncedTimeSlider.slider, timeSlider.slider)
{
    //BG
    setResizable (true, true);
    
    const float ratio = 4.0/ 3.0;
    setSize (p.getEditorWidth(), p.getEditorHeight());
    setResizeLimits (370,  juce::roundToInt (370.0 / ratio),
                         650, juce::roundToInt (650.0 / ratio));
    
    getConstrainer()->setFixedAspectRatio (ratio);
    
    
    //apvts
    for(auto* slider : {&syncedTimeSlider, &timeSlider, &mixSlider, &feedbackSlider, &driveSlider})
    {
        addAndMakeVisible(slider->slider);
    }
    timeSlider.slider.setVisible(false);
    
    addAndMakeVisible(syncToggle.button);
    addAndMakeVisible(twoValueSlider);
    twoValueSlider.setLookAndFeel(&twoValLaf);
    
    //Labels
    for(auto* label : {&hiCutLabel, &loCutLabel, &feedbackLabel, &driveLabel, &mixLabel, &title})
    {
        addAndMakeVisible(label);
        label->setFont (juce::Font (12.0f, juce::Font::bold));
    }
  
    hiCutLabel.setText("High Cut", juce::dontSendNotification);
    hiCutLabel.attachToComponent(&twoValueSlider, false);
    hiCutLabel.setJustificationType(juce::Justification::bottomRight);
    
    loCutLabel.setText("Low Cut", juce::dontSendNotification);
    loCutLabel.attachToComponent(&twoValueSlider, false);
    loCutLabel.setJustificationType(juce::Justification::bottomLeft);
    
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider.slider, false);
    feedbackLabel.setJustificationType(juce::Justification::centred);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.attachToComponent(&driveSlider.slider, false);
    driveLabel.setJustificationType(juce::Justification::centred);

    mixLabel.setText("Dry/Wet", juce::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider.slider, false);
    mixLabel.setJustificationType(juce::Justification::centred);
    
    title.setText ("F.W Echo v1.0", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::topLeft);
    title.setFont (juce::Font (10.0f));
    title.setInterceptsMouseClicks(false, false);
}

EchoDlineAudioProcessorEditor::~EchoDlineAudioProcessorEditor()
{
    twoValueSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void EchoDlineAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    
    g.fillAll (juce::Colour::fromFloatRGBA (0.08f, 0.08f, 0.08f, 1.0f));
   
}

void EchoDlineAudioProcessorEditor::resized()
{
    audioProcessor.setEditorSize (getWidth(), getHeight());
    
    juce::Rectangle<int> bounds = getLocalBounds();
    title.setBounds(bounds);
    juce::Rectangle<int> boundsUpper = bounds.removeFromTop(getHeight()/2);
    juce::Rectangle<int> boundsUpperLeft = boundsUpper.removeFromLeft(getWidth()/2);
    int margin = bounds.getWidth()/12;
    
    juce::Rectangle<int> syncToggleBounds = boundsUpperLeft.removeFromLeft(getWidth()/4).removeFromBottom(boundsUpperLeft.getHeight()/1.5);
    juce::Rectangle<int> syncToggleBoundsHalfHeight = syncToggleBounds.removeFromTop(syncToggleBounds.getHeight()/2);
    syncToggle.button.setBounds(syncToggleBoundsHalfHeight.reduced(margin/2));
    
    juce::Rectangle<int> timeSliderBounds = boundsUpperLeft;
    timeSlider.slider.setBounds(timeSliderBounds.reduced(0, margin/2));
    
    juce::Rectangle<int> syncSliderBounds = boundsUpperLeft;
    syncedTimeSlider.slider.setBounds(syncSliderBounds.reduced(0, margin/2));

    juce::Rectangle<int> sliderBounds = boundsUpper.removeFromBottom(getHeight()/2);
    juce::Rectangle<int> feedbackSliderBounds = sliderBounds.removeFromLeft(getWidth()/4);
    feedbackSlider.slider.setBounds(feedbackSliderBounds.reduced(0, margin));
    driveSlider.slider.setBounds(sliderBounds.reduced(0, margin));

    juce::Rectangle<int> mixBounds = bounds.removeFromRight(getWidth()/4);
    mixSlider.slider.setBounds(mixBounds.reduced(0, margin));
    
    twoValueSlider.setBounds(bounds.reduced(margin*1.5));
   
}
