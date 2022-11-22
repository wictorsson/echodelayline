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
    samplesOfDelay.reset(sampleRate, 0.8f);
    mix.reset(sampleRate, 0.05f);
    feedback.reset(sampleRate, 0.05f);
    
    reversedBuffer.clear();
    reversedBuffer.setSize(2, sampleRate * 10);
    reversedBuffer2.clear();
    reversedBuffer2.setSize(2, sampleRate * 10);
   
    prevBarPosition = std::floor(curBarPosition);
}

void DelayLine::setParameters(ParameterId paramId, float paramValue)
{
    switch (paramId)
    {
        case DelayLine::ParameterId::choice:
        {
            syncedDelayChoice = setSyncedDelayFromChoice(paramValue);
            changingDelay = true;
        
            resetRevBuffers = true;
            changingDelayPlaying = true;
        
            break;
        }
        case DelayLine::ParameterId::delayTime:
        {
            samplesInSec = (paramValue/1000.0f);
       
            resetRevBuffers = true;
            changingDelay = true;
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
            syncButton = paramValue;
            delayLine.reset();
            break;
        }
        case DelayLine::ParameterId::pingpong:
        {
            resetRevBuffers = true;
            pingpongButtonPressed = !paramValue;
            break;
        }
            
        case DelayLine::ParameterId::reverse:
        {
            resetRevBuffers = true;
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

    if(resetRevBuffers && resetCounter == 0)
    {
        if(pingpongButtonPressed)
        {
            pingpongButton = true;
            pingpongButtonPressed = false;
        }
        else
        {
            pingpongButton = false;
        }
        
        reversedBuffer.clear();
        reversedBuffer2.clear();
        counter = 0;
        revBuffer = 0;
        revBuffer2 = 0;
        
        delaySecondBuffer = true;
        resetRevBuffers = false;
        changingDelay = false;

        clearedBuffer = false;
        DBG("resetRevBuffers");
    }
    
    
    if(!fxChain.pitchShiftToggle && !reversedToggle &&!resetRevBuffers)
    {
        processForward(inputLeft, inputRight);
        
    }
    else if(fxChain.pitchShiftToggle && !reversedToggle &&!resetRevBuffers )
    {
        processForwardPitched(inputLeft, inputRight);
    }
    else if(!fxChain.pitchShiftToggle && reversedToggle &&!resetRevBuffers  )
    {
        reversedPitchedFX = false;
        processReversed(inputLeft, inputRight);

    }
    else if(fxChain.pitchShiftToggle && reversedToggle &&!resetRevBuffers )
    {
        reversedPitchedFX = true;
        //CAlling  same function as above but using bool instead for pitchshift detection = less code
        processReversed(inputLeft, inputRight);
    }

    resetCounter++;

    if(!changingDelay && (resetCounter > delayNoSmoothing || (inputLeft == 0.0 && inputRight == 0.0)))
    {
        resetCounter = 0;
    }
    quarterNote = std::floor(curBarPosition);
    if(quarterNote > prevBarPosition)
    {
        if(changingDelay)
        {

            resetCounter = 0;
        }
        prevBarPosition = quarterNote;
    }
    
    if(inputLeft == 0 && inputRight == 0)
    {
            resetCounter = 0;
            prevBarPosition = 0;
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

    float outputFXLeft = outputLeft;
    float outputFXRight = outputRight;

    fxChain.processFX(channelLeft, outputFXLeft);
    fxChain.processFX(channelRight, outputFXRight);

    inputLeft = dryMix * inputLeft + wetMix * outputFXLeft;
    inputRight = dryMix * inputRight + wetMix * outputFXRight;
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
    float outputFXLeft = outputLeft;
    float outputFXRight = outputRight;
    
    fxChain.processPitchShift(channelLeft, outputFXLeft);
    fxChain.processPitchShift(channelRight, outputFXRight);
    fxChain.processFX(channelLeft, outputFXLeft);
    fxChain.processFX(channelRight, outputFXRight);
    
    inputLeft = dryMix * inputLeft + wetMix * outputFXLeft;
    inputRight = dryMix * inputRight + wetMix * outputFXRight;
    
}

void DelayLine::processReversed(float& inputLeft, float& inputRight)
{

    float smoothedDelaytime = samplesOfDelay.getNextValue();

    // Reversed will be the new input so saving the untouched input here.
    float dryInputL = inputLeft;
    float dryInputR = inputRight;

    if(reversedPitchedFX)
    {
        fxChain.processPitchShift(channelLeft, inputLeft);
        fxChain.processPitchShift(channelRight, inputRight);
        fxChain.processFX(channelLeft, inputLeft);
        fxChain.processFX(channelRight, inputRight);
    }
    else
    {
        fxChain.processFX(channelLeft, inputLeft);
        fxChain.processFX(channelRight, inputRight);
    }
    //Create two buffers to fade between. Since the signal is reversed (playhead and writehead meets in the middle) two buffers are needed.
    
    float outputLeftREV;
    float outputRightREV;
    float outputLeftREV2;
    float outputRightREV2;
    
    reversedBuffer.setSample(0, smoothedDelaytime*2 - revBuffer, inputLeft);
    reversedBuffer.setSample(1, smoothedDelaytime*2  - revBuffer, inputRight);
    reversedBuffer2.setSample(0, smoothedDelaytime*2 - revBuffer2, inputLeft);
    reversedBuffer2.setSample(1, smoothedDelaytime*2  - revBuffer2, inputRight);

    if(pingpongButton)
    {
        counter ++;
        if(counter < delayNoSmoothing)
        {
            outputLeftREV = reversedBuffer.getSample(0, 0);
            outputRightREV = reversedBuffer.getSample(1, revBuffer);
            outputLeftREV2 = reversedBuffer2.getSample(0, 0);
            outputRightREV2 = reversedBuffer2.getSample(1, revBuffer2);
        }
        else
        {
            outputLeftREV = reversedBuffer.getSample(0, revBuffer);
            outputRightREV = reversedBuffer.getSample(1, 0);
            outputLeftREV2 = reversedBuffer2.getSample(0, revBuffer2);
            outputRightREV2 = reversedBuffer2.getSample(1, 0);
        }
        if(counter > delayNoSmoothing * 2)
        {
            counter = 0;
        }
    }
    else
    {
        outputLeftREV = reversedBuffer.getSample(0, revBuffer);
        outputRightREV = reversedBuffer.getSample(1, revBuffer);
        outputLeftREV2 = reversedBuffer2.getSample(0, revBuffer2);
        outputRightREV2 = reversedBuffer2.getSample(1, revBuffer2);
    }
    //Reset buffers, detect audio input note on, sync to DAW.
    revBuffer ++;
    if(revBuffer > delayNoSmoothing)
    {
        delaySecondBuffer = false;
    }
     //OFFSET second buffer
    if(!delaySecondBuffer)
    {
        revBuffer2 = revBuffer2 + 1;
    }

    if(revBuffer > delayNoSmoothing * 2  || (inputLeft == 0.0 && inputRight == 0.0) )
    {
        revBuffer = 0;
       
    }

    if(revBuffer2 > delayNoSmoothing * 2  || (inputLeft == 0.0 && inputRight == 0.0))
    {
        revBuffer2 = 0;
        if(inputLeft == 0.0 && inputRight == 0.0)
        {
            delaySecondBuffer = true;
        }
        
    }
    if(inputLeft == 0 && inputRight == 0)
    {
        counter = 0;
        
        if(!clearedBuffer)
        {
            reversedBuffer.clear();
            reversedBuffer2.clear();
            clearedBuffer = true;
        }
        

    }

    if(inputLeft != 0 || inputRight != 0)
    {
        clearedBuffer = false;
    }
    
    // Extra long fade when changing delay time in realtime
    int fadeinMultiplyer;
    if(changingDelayPlaying)
    {
        fadeinMultiplyer = 30;
    }
    else
    {
        fadeinMultiplyer = 1;
    }

   // FADING both buffers start and end
    if (revBuffer < delayNoSmoothing + fadelenSamples && revBuffer >delayNoSmoothing)
    {
        
        fadegain = juce::jmap<float>(revBuffer, delayNoSmoothing, delayNoSmoothing+fadelenSamples, 0.0, 1.0f);
    }
   else if (revBuffer > delayNoSmoothing * 2 -fadelenSamples)
    { fadegain = juce::jmap<float>(revBuffer, delayNoSmoothing*2 - fadelenSamples, delayNoSmoothing*2, 1.0, 0.0f);

    }

    if (revBuffer2 < delayNoSmoothing + fadelenSamples*fadeinMultiplyer && revBuffer2 >delayNoSmoothing)
    {
        fadegain2 = juce::jmap<float>(revBuffer2, delayNoSmoothing, delayNoSmoothing+fadelenSamples*fadeinMultiplyer, 0.0, 1.0f);
    }
   else if (revBuffer2 > delayNoSmoothing * 2 -fadelenSamples)
    {
        fadegain2 = juce::jmap<float>(revBuffer2, delayNoSmoothing*2 - fadelenSamples, delayNoSmoothing*2, 1.0, 0.0f);
    }

    //DELAY/FEEDBACK part
    // CREATE new reversed input to feed the feedback delayline
    float inputLeftREV = (outputLeftREV * smoothedFeedback*fadegain + outputLeftREV2 * smoothedFeedback* fadegain2);
    float inputRightREV = (outputRightREV* smoothedFeedback * fadegain + outputRightREV2 * smoothedFeedback * fadegain2);

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
    //Ping pong is actually autopannig the reversed signal, ffedback goes to another delayline
    
    

    if(changingDelay)
    {
        inputLeft = dryMix * dryInputL + (wetMix * ( outputLeftREV*fadegain + outputLeftREV2*fadegain2)+ wetMix * outputLeftF)*0;
        inputRight = dryMix * dryInputR + (wetMix * ( outputRightREV*fadegain + outputRightREV2*fadegain2) + wetMix * outputRightF)*0;
        
    }else
    {
        inputLeft = dryMix * dryInputL + wetMix * ( outputLeftREV*fadegain + outputLeftREV2*fadegain2)+ wetMix * outputLeftF;
        inputRight = dryMix * dryInputR + wetMix * ( outputRightREV*fadegain + outputRightREV2*fadegain2) + wetMix * outputRightF;
    }
}


//CALLED from the audioprocessor every buffer block
float DelayLine::updateDelayTime()
{
    if(syncButton)
    {
        float beatsPerSec = bpm / 60;
        float secPerBeat = 1/beatsPerSec;
        return (syncedDelayChoice * (secPerBeat * mySampleRate));
    }
    return (samplesInSec * mySampleRate);
}


