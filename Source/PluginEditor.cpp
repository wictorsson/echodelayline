/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EchoDlineAudioProcessorEditor::EchoDlineAudioProcessorEditor (EchoDlineAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), twoValueSlider(juce::Slider::SliderStyle::TwoValueHorizontal, p.apvts.getParameter("hp"), p.apvts.getParameter("lp")), syncedTimeSlider(p.apvts.getParameter("choice"), ""), timeSlider(p.apvts.getParameter("delayTime"), " ms"), mixSlider(p.apvts.getParameter("mix"), " %"), feedbackSlider(p.apvts.getParameter("feedback"), " %"), driveSlider(p.apvts.getParameter("drive"), ""),pitchIntSlider(p.apvts.getParameter("psInterval"),""),syncToggle(p.apvts.getParameter("sync"), "Sync", syncedTimeSlider.slider, timeSlider.slider)
{
    //BG
    setResizable (true, true);
    
    const float ratio = 4.0/ 3.0;
    setSize (p.getEditorWidth(), p.getEditorHeight());
    setResizeLimits (370,  juce::roundToInt (370.0 / ratio),
                         650, juce::roundToInt (650.0 / ratio));
    
    getConstrainer()->setFixedAspectRatio (ratio);
    
    
    //apvts
    for(auto* slider : {&syncedTimeSlider, &timeSlider, &mixSlider, &feedbackSlider, &driveSlider,&pitchIntSlider })
    {
        addAndMakeVisible(slider->slider);
    }
    timeSlider.slider.setVisible(false);
    
    addAndMakeVisible(syncToggle.button);
    addAndMakeVisible(twoValueSlider);
    twoValueSlider.setLookAndFeel(&twoValLaf);
    
    //Labels
    for(auto* label : {&hiCutLabel, &loCutLabel, &feedbackLabel, &driveLabel, &mixLabel, &title, &pitchLabel})
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
    
    pitchLabel.setText("Pitch", juce::dontSendNotification);
    pitchLabel.attachToComponent(&pitchIntSlider.slider, false);
    pitchLabel.setJustificationType(juce::Justification::centred);
    
    title.setText ("F.W Echo v1.1", juce::dontSendNotification);
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
    
    auto r = getLocalBounds();
    title.setBounds(r.removeFromTop(20).reduced(2));
    auto topSection = r.removeFromTop(r.getHeight() * 0.15);
    auto midSection = r.removeFromTop(r.getHeight() * 0.5);
    
    auto topLeftSection = topSection.removeFromLeft(topSection.getWidth()*0.5);
    syncToggle.button.setBounds(topLeftSection.reduced(70,5));
    
    auto timeSlidersRec = midSection.removeFromLeft(midSection.getWidth()*0.5);
    syncedTimeSlider.slider.setBounds(timeSlidersRec.reduced(5));
    timeSlider.slider.setBounds(timeSlidersRec.reduced(5));
    pitchIntSlider.slider.setBounds(midSection.removeFromLeft(midSection.getWidth()*0.33).reduced(5,20));
    driveSlider.slider.setBounds(midSection.removeFromLeft(midSection.getWidth() * 0.5).reduced(5,20));
    feedbackSlider.slider.setBounds(midSection.reduced(5,20));
    
    auto lowLeftSection = r.removeFromLeft(r.getWidth()*0.5);
    twoValueSlider.setBounds(lowLeftSection.reduced(5, lowLeftSection.getHeight()*0.35));
    mixSlider.slider.setBounds(r.reduced(5, lowLeftSection.getHeight() * 0.2));
    

   
}
