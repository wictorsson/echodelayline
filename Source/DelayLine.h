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
    void prepare(float sampleRate, int numchans, float samplesPerBlock, juce::dsp::ProcessSpec spec);
    void processDelay(float& inputLeft, float& inputRight);
    //
    void processForward(float& inputLeft, float& inputRight);
    void processForwardPitched(float& inputLeft, float& inputRight);
    void processReversed(float& inputLeft, float& inputRight);
 
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
    float curBarPosition;
    float prevBarPosition;
    double bpm{ 120.0f };
    bool pingpongButton{false};
    bool syncButton{true};
    bool reversedToggle{false};
    bool reversedPitchedFX;
    int delayNoSmoothing;
    Fx fxChain;
    juce::LinearSmoothedValue<float> samplesOfDelay;
    
    juce::AudioBuffer<float> reversedBuffer;
    juce::AudioBuffer<float> reversedBuffer2;
    
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>delayLine;
    juce::SmoothedValue<float> mix;
    juce::SmoothedValue<float> feedback;
    float smoothedFeedback;
    float wetMix;
    float dryMix;
    float syncedDelayChoice;
    float samplesInSec;
    float feedBackSignals[2] = { 0.0f,0.0f };
    float feedBackSignals2[2] = { 0.0f,0.0f };
    float revBuffer{0.0};
    float revBuffer2{0.0};
    
    double mySampleRate{0.0};
    
    int counter{0};
    int resetCounter{0};
    int channelLeft = 0;
    int channelRight = 1;
    
    bool resetRevBuffers;
    bool changingDelay;
    bool changingDelayPlaying;
    bool delaySecondBuffer{true};
    bool playDelayBuffer;
    bool clearedBuffer;
    bool pingpongButtonPressed;
    bool clearedBuffer1;
    bool clearedBuffer2;

};


