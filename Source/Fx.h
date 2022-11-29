/*
  ==============================================================================

    Fx.h
    Created: 8 Nov 2022 10:47:05am
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Fx
{
public:
    void prepare(float sampleRate, int numchans, float samplesPerBlock, juce::dsp::ProcessSpec spec);
    
    void processFX(int& channel, float& inSample);
    void processPitchShift(int& channel, float& inSample);
    void processFlutter( float& inSample,  float& inSampleRight);
    void updateDelayTime();
    enum class ParameterId
    {
        sDrive,
        lp,
        hp,
        psInterval,
        flutter
    };
    
    void setParameters(ParameterId paramId, float paramValue);
    void pitchShiftLFO();
    
    juce::SmoothedValue<float> saturationDrive;
    juce::SmoothedValue<float> lpSmoothed;
    juce::SmoothedValue<float> hpSmoothed;
    juce::SmoothedValue<float> flutterDial;
    juce::LinearSmoothedValue<float> samplesOfDelayFluttSmooth;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>dlFlutter;
    
    float semitones;
    float tr;
    float dRate;
    float flutterRate;
    float flutterRateConv;
    bool settingFlutter;
    float maxDelay;
    float mySampleRate;
    bool pitchShiftToggle{false};
private:
    //Pitch shift with delay lines, two delaylines to fade between
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
    bool lfoDown;
    float rate;
    //Filters
    juce::dsp::StateVariableTPTFilter<float> lpFilter;
    juce::dsp::StateVariableTPTFilter<float> hpFilter;
    bool lpOn{false};
    bool hpOn{true};
    
    juce::LinearSmoothedValue<float> mute{1};
    bool valuechanged{false};
    float currentDelayTime2;

};
