/*
  ==============================================================================

    DelayLine.h
    Created: 11 Nov 2022 3:16:38pm
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fx.h"

class DelayLine
{
   
    
public:
    
    DelayLine(){std::cout << "NEW DELAYLINE CREATED" << pingpongButton << std::endl;};
    
    //Methods
    void prepare(float sampleRate, int numchans, float samplesPerBlock, juce::dsp::ProcessSpec spec);
    
    void processDelay(float& inputLeft, float& inputRight);
    void processForward(float& inputLeft, float& inputRight);
    void processForwardPitched(float& inputLeft, float& inputRight);
    void processReversed(float& inputLeft, float& inputRight);
    void processReversedPitched(float& inputLeft, float& inputRight);
 
    
    enum class ParameterId
    {
        choice,
        delayTime,
        mix,
        feedback,
        sync,
        pingpong,
        reverse
    };
    
    void setParameters(ParameterId paramId, float paramValue);
    void setDelayTarget();
    float updateDelayTime();
    float setSyncedDelayFromChoice(float choice);
    double bpm{ 120.0f };
    bool pingpongButton{false};
    bool syncButton{true};
//    float wetMix;
//    float dryMix;
    bool reversedToggle{false};
    bool reversedPitchedFX;
    
    Fx fxChain;
    juce::LinearSmoothedValue<float> samplesOfDelay;
    int delayNoSmoothing;
    int oldDelayNoSmoothing;
    juce::AudioBuffer<float> reversedBuffer;
    juce::AudioBuffer<float> reversedBuffer2;
    int numSamplesRev;
    float curBarPosition;
    float prevBarPosition;
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>delayLine;
    juce::SmoothedValue<float> mix;
    juce::SmoothedValue<float> feedback;
    
    float syncedDelayChoice;
    float samplesInSec;
    
    
    float dFloat{0.0f};
    float dFloat2{0.0f};
    double mySampleRate{0.0};
    
    float feedBackSignals[2] = { 0.0f,0.0f };
    float feedBackSignals2[2] = { 0.0f,0.0f };
    
    float dryfeedBackSignals[2] = { 0.0f,0.0f };
    float dryfeedBackSignals2[2] = { 0.0f,0.0f };
    
    int channelLeft = 0;
    int channelRight = 1;

    float revBuffer{0.0};
    float revBuffer2{0.0};
    bool resetRevBuffers;
    bool changingDelay;
    bool changingDelayPlaying;
    int changingDelayTimer{0};
    bool changingDelayButton;
    
    int counter{0};
    int resetCounter{0};
    int quarterNote;
    
    int fadelenSamples = 500;
    float fadegain = 1.0f;
    float fadegain2 = 1.0f;
    bool delaySecondBuffer{true};
    bool playDelayBuffer;
    bool release;
    bool clearedBuffer;
    bool pingpongButtonPressed;
    bool clearedBuffer1;
    bool clearedBuffer2;
    
  
    
    float smoothedFeedback;
    float wetMix;
    float dryMix;
   
};


