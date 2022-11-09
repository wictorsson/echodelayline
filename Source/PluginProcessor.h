/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fx.h"
//==============================================================================
/**
*/
class EchoDlineAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    EchoDlineAudioProcessor();
    ~EchoDlineAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout initializeGUI();

    
    float workOutMS(float sampleRate, float ms)
    {
        return((sampleRate / 1000) * ms);
    }
    
    
    // Save and set GUI resize
    int getEditorWidth()
    {
        auto size = apvts.state.getOrCreateChildWithName ("lastSize", nullptr);
        return size.getProperty ("width", 370);
    }
    int getEditorHeight()
    {
        const float ratio = 4.0/ 3.0;
        auto size = apvts.state.getOrCreateChildWithName ("lastSize", nullptr);
        return size.getProperty ("height", 370.0 / ratio);
    }

    void setEditorSize (int width, int height)
    {
        auto size = apvts.state.getOrCreateChildWithName ("lastSize", nullptr);
        size.setProperty ("width", width, nullptr);
        size.setProperty ("height", height, nullptr);
    }
    
private:
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>delayLine;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>delayLine2;

    juce::SmoothedValue<float> mix;
    juce::SmoothedValue<float> feedback;

    float syncedDelayChoice;
    float samplesInSec;
    juce::LinearSmoothedValue<float> samplesOfDelay;
    double bpm{ 120.0f };
    float dFloat{0.0f};
    float dFloat2{0.0f};
    double mySampleRate{0.0};
    bool syncButton{true};
    float feedBackSignals[2] = { 0.0f,0.0f };
 
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    float updateDelayTime();
    float setSyncedDelayFromChoice(float choice);
    
    Fx fxChain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EchoDlineAudioProcessor)
};
