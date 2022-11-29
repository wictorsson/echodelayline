/*
  ==============================================================================

    DelayLine.cpp
    Created: 11 Nov 2022 3:16:38pm
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#include "DelayLine.h"


void DelayLine::prepare(float sampleRate, int numchans, float samplesPerBlock, juce::dsp::ProcessSpec spec) {
    
    mySampleRate = sampleRate;
    delayLine.reset();
    delayLine.setMaximumDelayInSamples(10*sampleRate);
    delayLine.prepare(spec);
    samplesOfDelay.reset(sampleRate, 0.08f);
 
    mix.reset(sampleRate, 0.05f);
    feedback.reset(sampleRate, 0.05f);
    reversedBuffer.clear();
    reversedBuffer.setSize(2, sampleRate * 10);
    startBPM = bpm;
    prevBarPos = -100;
   
    fadeFrontBuffGain.reset(sampleRate, 0.2f);
    fadeRevBuffGain.reset(sampleRate, 0.2f);
    fadeRevBuffGain.setTargetValue(0);
    fadeFrontBuffGain.setTargetValue(1);
    
  
}

void DelayLine::setParameters(ParameterId paramId, float paramValue)
{
    switch (paramId)
    {
        case DelayLine::ParameterId::choice:
        {
           
            syncedDelayChoice = setSyncedDelayFromChoice(paramValue);
            if(reversedToggle)
            {
                changingDelay = true;
            }
            break;
        }
        case DelayLine::ParameterId::delayTime:
        {
            samplesInSec = (paramValue/1000.0f);
            if(reversedToggle)
            {
                changingDelay = true;
            }
            break;
        }
        case DelayLine::ParameterId::feedback:
        {
            feedback.setTargetValue(paramValue/100.0f);
            break;
        }
        case DelayLine::ParameterId::mix:
        {
            mix.setTargetValue(paramValue/100);
            break;
        }
        case DelayLine::ParameterId::sync:
        {
            changingDelay = true;
            syncButton = paramValue;
            delayLine.reset();
            break;
        }
        case DelayLine::ParameterId::pingpong:
        {
            changingDelay = true;
            pingpongButton = !paramValue;
            break;
        }
            
        case DelayLine::ParameterId::reverse:
        {
           
            fadeFrontBuffGain.setTargetValue(0);
            fadeRevBuffGain.setTargetValue(0);
            changingDelay = true;
            reverseTransition = true;
            delaySwitch = true;
            reversedToggle = !paramValue;
         
            break;
        }
        default : std::cout << "Invalid" << std::endl;
    }
}

float DelayLine::setSyncedDelayFromChoice(float choice)
{
    switch((int)choice)
    {
        case 0:
            return 0.16675f;
            break;
        case 1:
            return 0.1875f;
            break;
        case 2:
            return 0.25f;
            break;
        case 3:
            return 0.3335f;
            break;
        case 4:
            return 0.375f;
            break;
        case 5:
            return 0.5f;
            break;
        case 6:
            return 0.667f;
            break;
        case 7:
            return 0.75f;
            break;
        case 8:
            return 1.0f;
            break;
        case 9:
            return 1.334f;
            break;
        case 10:
            return 1.5f;
            break;
        case 11:
            return 2.0f;
            break;
        case 12:
            return 2.668f;
            break;
        case 13:
            return 3.0f;
            break;
    }
    return 0.0f;
}
void DelayLine::setDelayTarget()
{
    fxChain.updateDelayTime();
    float currentDelayTime = updateDelayTime();
    samplesOfDelay.setTargetValue(currentDelayTime);
    delayNoSmoothing = currentDelayTime;
}

void DelayLine::processDelay(float& inputLeft, float& inputRight)
{
    
    //---------
    fxChain.pitchShiftLFO();
    smoothedFeedback = feedback.getNextValue();
    wetMix = mix.getNextValue();
    dryMix = 1.0 - wetMix;
    //-----------

    resetCounter ++;
    if(startBPM != bpm)
    {
        changingDelay = true;
        startBPM = bpm;
    }

    
    //Reset when silent
    if(inputLeft == 0 && inputRight == 0 )
    {
        silentTimer++;
        if(std::floor(curBarPosition) ==  std::floor(prevBarPos) && silentTimer > 2000)
            {
                // !FRED fix - do this only once
                reversedBuffer.clear();
                silentTimer = 0;
                fadeFrontBuffGain.setTargetValue(1);
                fadeRevBuffGain.setTargetValue(1);
                changingDelay = false;
            }
        resetCounter = 0;
    }
    int quarterNote = std::floor(curBarPosition);
    prevBarPos = curBarPosition;
    // SYNC to daw, in reverse mode, reset the buffers on the downbeat
    if(inputLeft != 0 && inputRight != 0)
    {
        if(reversedToggle && curBarPosition - quarterNote > 0.95f && resetCounter > delayNoSmoothing && changingDelay)
        {
            resetCounter = 0;
            reversedBuffer.clear();
            counter = 0;
            revBuffer = 1;
            changingDelay = false;
            reverseTransition = false;
        }
        if(reversedToggle && !reverseTransition)
        {
            resetCounter2++;
            if(resetCounter2 > delayNoSmoothing && curBarPosition - quarterNote > 0.95f)
            {
           
                if(fadeRevBuffGain.getNextValue()==0)
                {
                    fadeRevBuffGain.setTargetValue(1);
                    fadeFrontBuffGain.setTargetValue(0);
                }
                resetCounter2 = 0;
            }
        }
        if(!reversedToggle && curBarPosition - quarterNote > 0.95f && resetCounter > delayNoSmoothing)
        {
            reverseTransition = false;
            changingDelay = false;
        }
        if(!reversedToggle && !reverseTransition)
        {
            if(fadeFrontBuffGain.getNextValue()==0)
            {
                fadeFrontBuffGain.setTargetValue(1);
                fadeRevBuffGain.setTargetValue(0);
            }
        }
    }

    // PROCESS different delay variations
    if(!reversedToggle &&!changingDelay)
    {
        if(!fxChain.pitchShiftToggle)
        {
            processForward(inputLeft, inputRight);
        }
        else
        {
            processForwardPitched(inputLeft, inputRight);
        }
    }
    //REVERSED
    else if(reversedToggle &&!changingDelay)
    {
       // DBG("REVERS BUFFER");
        processReversed(inputLeft, inputRight);
        reversedPitchedFX = fxChain.pitchShiftToggle ? true : false;
    }
    else
    {
        inputLeft = dryMix * inputLeft;
        inputRight = dryMix * inputRight;
    }
 
}

void DelayLine::processForward(float& inputLeft, float& inputRight)
{
  
    if(pingpongButton)
    {
        delayLine.pushSample(channelLeft, (inputLeft + inputRight) * 0.5 + feedBackSignals[channelRight]);
        delayLine.pushSample(channelRight, feedBackSignals[channelLeft]);
    }
    else
    {
        delayLine.pushSample(channelLeft, inputLeft + feedBackSignals[channelLeft] * smoothedFeedback);
        delayLine.pushSample(channelRight, inputRight + feedBackSignals[channelRight] * smoothedFeedback);
    }
    
    float smoothedDelaytime = samplesOfDelay.getNextValue();;
    float outputLeft = delayLine.popSample(channelLeft, smoothedDelaytime);
    float outputRight = delayLine.popSample(channelRight, smoothedDelaytime);

    feedBackSignals[channelLeft] = outputLeft;
    feedBackSignals[channelRight] = pingpongButton ? outputRight * smoothedFeedback : outputRight;

    fxChain.processFlutter(outputLeft, outputRight);
    float outputFXLeft = outputLeft;
    float outputFXRight = outputRight;
    fxChain.processFX(channelLeft, outputFXLeft);
    fxChain.processFX(channelRight, outputFXRight);
    
    inputLeft = dryMix * inputLeft + wetMix * outputFXLeft * fadeFrontBuffGain.getNextValue();
    inputRight = dryMix * inputRight + wetMix * outputFXRight * fadeFrontBuffGain.getNextValue();
}

void DelayLine::processForwardPitched(float& inputLeft, float& inputRight)
{
    if(pingpongButton)
    {
        delayLine.pushSample(channelLeft, (inputLeft + inputRight) * 0.5 + feedBackSignals[channelRight]);
        delayLine.pushSample(channelRight, feedBackSignals[channelLeft]);
    }
    else
    {
        delayLine.pushSample(channelLeft, inputLeft + feedBackSignals[channelLeft] * smoothedFeedback);
        delayLine.pushSample(channelRight, inputRight + feedBackSignals[channelRight] * smoothedFeedback);
    }
    
    //multiply only if pitch shift (two delay lines - multiplication needed.....)
    float smoothedDelaytime = samplesOfDelay.getNextValue() * 2;
    float outputLeft;
    float outputRight;
    
    if(pingpongButton)
    {
        outputLeft = delayLine.popSample(channelLeft, smoothedDelaytime - (mySampleRate * 0.05)*2);
        outputRight = delayLine.popSample(channelRight, smoothedDelaytime -  (mySampleRate * 0.05)*2);
    }
    else
    {
        // ADJUSTING delay drift (- mySampleRate * 0.05) To compensate for pitch shift delay (50 ms
        outputLeft = delayLine.popSample(channelLeft, smoothedDelaytime - (mySampleRate * 0.05));
        outputRight = delayLine.popSample(channelRight, smoothedDelaytime - (mySampleRate * 0.05));
    }
    // ADD back sample for feedback,
    if(pingpongButton)
    {
        delayLine.pushSample(channelLeft, (inputLeft + inputRight) * 0.5 + feedBackSignals[channelRight]);
        delayLine.pushSample(channelRight, feedBackSignals[channelLeft]);
    }
    else
    {
        delayLine.pushSample(channelLeft, inputLeft + feedBackSignals[channelLeft] * smoothedFeedback);
        delayLine.pushSample(channelRight, inputRight + feedBackSignals[channelRight] * smoothedFeedback);
    }
    float outputLeftF = delayLine.popSample(channelLeft, smoothedDelaytime);
    float outputRightF = delayLine.popSample(channelRight, smoothedDelaytime);
    
    feedBackSignals[channelLeft] = outputLeftF;
    feedBackSignals[channelRight] = pingpongButton ? outputRightF * smoothedFeedback : outputRightF;

    //APPLY fx
    fxChain.processFlutter(outputLeft, outputRight);
    float outputFXLeft = outputLeft;
    float outputFXRight = outputRight;
    
    fxChain.processPitchShift(channelLeft, outputFXLeft);
    fxChain.processPitchShift(channelRight, outputFXRight);
    fxChain.processFX(channelLeft, outputFXLeft);
  
    inputLeft = dryMix * inputLeft + wetMix * outputFXLeft*fadeFrontBuffGain.getNextValue();
    inputRight = dryMix * inputRight + wetMix * outputFXRight*fadeFrontBuffGain.getNextValue();
}

void DelayLine::processReversed(float& inputLeft, float& inputRight)
{
 
    float smoothedDelaytime = samplesOfDelay.getNextValue();
   
    // Reversed will be the new input to feed the delay, so saving the untouched input here.
    float dryInputL = inputLeft;
    float dryInputR = inputRight;

    //FX
    if(reversedPitchedFX)
    {
        fxChain.processPitchShift(channelLeft, inputLeft);
        fxChain.processPitchShift(channelRight, inputRight);
    }
    
    fxChain.processFX(channelLeft, inputLeft);
    fxChain.processFX(channelRight, inputRight);
    fxChain.processFlutter(inputLeft, inputRight);
    
    //SET fade time at start, end and middle of the double sized buffer.
    int fadeLengthSamples = 500;
    float fadeGain = 1.0f;
  
    // FRED fix - fade gain here when toggling buttons too
    if (revBuffer < fadeLengthSamples )
    {
        fadeGain = juce::jmap<float>(revBuffer, 0, fadeLengthSamples, 0.0, 1.0f);
    }
    if (revBuffer >= delayNoSmoothing -fadeLengthSamples && revBuffer < delayNoSmoothing )
    {
        fadeGain = juce::jmap<float>(revBuffer, delayNoSmoothing - fadeLengthSamples, delayNoSmoothing, 1.0, 0.0f);
    }
    
    if (revBuffer >= delayNoSmoothing  && revBuffer < delayNoSmoothing +fadeLengthSamples )
    {
        fadeGain = juce::jmap<float>(revBuffer, delayNoSmoothing, delayNoSmoothing+fadeLengthSamples, 0.0, 1.0f);
    }
    
   if (revBuffer >= (delayNoSmoothing * 2 -fadeLengthSamples) && revBuffer < delayNoSmoothing * 2)
    {
        fadeGain = juce::jmap<float>(revBuffer, delayNoSmoothing*2 - fadeLengthSamples, delayNoSmoothing*2 , 1.0, 0.0f);
    }
        reversedBuffer.setSample(channelLeft, smoothedDelaytime*2 - revBuffer, inputLeft * fadeGain);
        reversedBuffer.setSample(channelRight, smoothedDelaytime*2  - revBuffer, inputRight * fadeGain);
    
    //Creating new outputs for reversed delay and reversed ping pong
    float outputLeftREV;
    float outputRightREV;
    
    if(pingpongButton)
    {
        counter ++;
        if(counter < delayNoSmoothing)
        {
            outputLeftREV = reversedBuffer.getSample(channelLeft, 0);
            outputRightREV = reversedBuffer.getSample(channelRight, revBuffer);
        }
        else
        {
            outputLeftREV = reversedBuffer.getSample(channelLeft, revBuffer);
            outputRightREV = reversedBuffer.getSample(channelRight, 0);
        }
        if(counter > delayNoSmoothing * 2)
        {
            counter = 0;
        }
    }
    else
    {
        outputLeftREV = reversedBuffer.getSample(channelLeft, revBuffer) ;
        outputRightREV = reversedBuffer.getSample(channelRight, revBuffer);
    }
    
    //DETECT audio input note on, sync to DAW.
    revBuffer ++;

    if(revBuffer >= delayNoSmoothing * 2 || (inputLeft == 0.0 && inputRight == 0.0) )
    {
        revBuffer = 1;
    }
 
    if(inputLeft == 0 && inputRight == 0)
    {
        //Ping pong counter
        counter = 0;
    }

    //---------------------DELAY/FEEDBACK part-----------------
    // CREATE new reversed input to feed the feedback delayline
    float inputLeftREV = (outputLeftREV * smoothedFeedback*fadeGain );
    float inputRightREV = (outputRightREV* smoothedFeedback * fadeGain);

    if(pingpongButton)
    {
        delayLine.pushSample(channelLeft, feedBackSignals[channelRight] * smoothedFeedback);
        delayLine.pushSample(channelRight, (inputRightREV + inputLeftREV)*0.5 + feedBackSignals[channelLeft]);
    }
    else
    {
        delayLine.pushSample(channelLeft, inputLeftREV + feedBackSignals[channelLeft] * smoothedFeedback);
        delayLine.pushSample(channelRight, inputRightREV +feedBackSignals[channelRight] * smoothedFeedback);
    }
    float outputLeftF = delayLine.popSample(channelLeft, smoothedDelaytime);
    float outputRightF = delayLine.popSample(channelRight, smoothedDelaytime);
    feedBackSignals[channelLeft] = pingpongButton ? outputLeftF * smoothedFeedback : outputLeftF;
    feedBackSignals[channelRight] = outputRightF;
  //  Ping pong is autopannig the reversed signal, feedback goes to another delayline
    // AVOID glitch while changing delay time = mute delayed output

    inputLeft = dryMix * dryInputL + (wetMix * (outputLeftREV * fadeGain)+ wetMix * outputLeftF)*fadeRevBuffGain.getNextValue();
    inputRight = dryMix * dryInputR + (wetMix * ( outputRightREV * fadeGain ) + wetMix * outputRightF)*fadeRevBuffGain.getNextValue();
    
}

//CALLED from the audioprocessor every buffer block
float DelayLine::updateDelayTime()
{
    if(syncButton)
    {
        float beatsPerSec = bpm / 60;
        float secPerBeat = 1/beatsPerSec;
        samplesInSec = secPerBeat * syncedDelayChoice;
        return (samplesInSec * mySampleRate);
    }
        return (samplesInSec * mySampleRate);
}

