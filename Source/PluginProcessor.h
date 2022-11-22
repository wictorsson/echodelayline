/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"

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
        return size.getProperty ("width", 450);
    }
    int getEditorHeight()
    {
        const float ratio = 4.0/ 3.0;
        auto size = apvts.state.getOrCreateChildWithName ("lastSize", nullptr);
        return size.getProperty ("height", 450.0 / ratio);
    }

    void setEditorSize (int width, int height)
    {
        auto size = apvts.state.getOrCreateChildWithName ("lastSize", nullptr);
        size.setProperty ("width", width, nullptr);
        size.setProperty ("height", height, nullptr);
    }
    
    void setStutter(bool b);

    
private:
    
  //  juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>delayLine;

    juce::SmoothedValue<float> mix;
    juce::SmoothedValue<float> feedback;


    double mySampleRate{0.0};

    void parameterChanged(const juce::String& parameterID, float newValue) override;

     DelayLine delayLine;
   
    double stutterLength = 0.0;
    bool stutterActive = false;
    int captureBufferIndex = 0;
    int playBufferIndex = 0;
    juce::CriticalSection cs;
    double curSampleRate = 0.0;
    
    float wetMix = mix.getNextValue();
    float dryMix = 1.0 - wetMix;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EchoDlineAudioProcessor)
};
