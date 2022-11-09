/*
  ==============================================================================

    Fx.h
    Created: 8 Nov 2022 10:47:05am
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//#include "PluginProcessor.h"
class Fx
{
public:
    void prepare(float sampleRate, int numchans, float samplesPerBlock);
    
    void processFX(int& channel, float& inSample);
    void processPitchShift(int& channel, float& inSample);

    enum class ParameterId
    {
        sDrive,
        lp,
        hp,
        psInterval
    };
    
    void setParameters(ParameterId paramId, float paramValue);
    void pitchShiftLFO();
    
    juce::SmoothedValue<float> saturationDrive;
    juce::SmoothedValue<float> lpSmoothed;
    juce::SmoothedValue<float> hpSmoothed;
    
private:
    //Pitch shift with delay lines
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>dlPitchShift;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>dlPitchShift2;
    float dFloat{0.0f};
    float dFloat2{0.0f};
    bool delayXfadeBuffer{true};
    
    juce::LinearSmoothedValue<float> dFloatSmoothed{0.0f};
    juce::LinearSmoothedValue<float> dFloatSmoothed2{0.0f};
    
    float tempsmoothedDelaytime;
    float tempsmoothedDelaytime2;
 
    juce::LinearSmoothedValue<float> xFade{1};
    juce::LinearSmoothedValue<float> xFade2{1};
    
    const float piDiv = 2.0f/ juce::MathConstants<float>::pi;
    bool saturationOn{false};
    
    //Filters
    juce::dsp::StateVariableTPTFilter<float> lpFilter;
    juce::dsp::StateVariableTPTFilter<float> hpFilter;
    bool lpOn{false};
    bool hpOn{true};
    
    float semitones;
    float tr;
    float dRate;
    float maxDelay;
    float mySampleRate;
    
    juce::LinearSmoothedValue<float> mute{1};
    bool valuechanged{false};
  //  float maxDelay = mySampleRate * 0.05f;
    
};
