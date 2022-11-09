/*
  ==============================================================================

    Fx.cpp
    Created: 8 Nov 2022 10:47:05am
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#include "Fx.h"


void Fx::prepare(float sampleRate, int numchans, float samplesPerBlock)
{
    mySampleRate = sampleRate;
    
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    saturationDrive.reset(sampleRate, 0.05f);
    lpSmoothed.reset(sampleRate, 0.05f);
    hpSmoothed.reset(sampleRate, 0.05f);

    mute.reset(sampleRate, 0.60f);
   
    //Filter
    lpFilter.reset();
    lpFilter.prepare(spec);
    hpFilter.reset();
    hpFilter.prepare(spec);
    lpFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    hpFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    
    dlPitchShift.reset();
    dlPitchShift.setMaximumDelayInSamples(10*sampleRate);
    dlPitchShift.prepare(spec);
    dlPitchShift2.reset();
    dlPitchShift2.setMaximumDelayInSamples(10*sampleRate);
    dlPitchShift2.prepare(spec);
 
    xFade.reset(sampleRate, 0.025f);
    xFade.setTargetValue(1.0f);
    xFade2.reset(sampleRate, 0.025f);
    xFade2.setTargetValue(0.0f);
    
    maxDelay = mySampleRate * 0.05f; // Max delay = 50 ms
}

void Fx::setParameters(ParameterId paramId, float paramValue)
{
    switch (paramId)
    {
        case Fx::ParameterId::sDrive:
        {
            saturationDrive.setTargetValue(paramValue);
            saturationOn =  paramValue > 0.0f ? true : false;
            break;
        }
        case Fx::ParameterId::lp:
        {
            lpSmoothed.setTargetValue(paramValue);
            lpOn = paramValue < 20000 ? true : false;
            break;
        }
        case Fx::ParameterId::hp:
        {
            hpSmoothed.setTargetValue(paramValue);
            hpOn =  paramValue > 0 ? true : false;
            break;
        }
        case Fx::ParameterId::psInterval:
        {
            valuechanged = true;
            //FADE to avoid discontinuity
            mute.setTargetValue(0);
            delayXfadeBuffer = true;
            semitones = paramValue;
            tr = std::pow(2.0f,semitones/12.0f);
            dRate = 1 - tr;
        
            if(dRate < 0)
            {
                dFloat = 0;
                dFloat2 = 0;
            }
            else
            {
                dFloat = maxDelay;
                dFloat2 = maxDelay;
            }
            break;
        }
    }
}

void Fx::processFX(int& channel, float &inSample)
{
    if(saturationOn)
    {
        inSample = inSample * (1.0F - saturationDrive.getNextValue()/10.0F) + piDiv * std::atanf(inSample * saturationDrive.getNextValue());
    }
    
    if(lpOn)
    {
        lpFilter.setCutoffFrequency(lpSmoothed.getNextValue());
        inSample = lpFilter.processSample(channel, inSample);
    }
    
    if(hpOn)
    {
        hpFilter.setCutoffFrequency(hpSmoothed.getNextValue());
        inSample = hpFilter.processSample(channel, inSample);
    }
}

void Fx::pitchShiftLFO()
{
    if (semitones < 0) // Pitch down
    {
        // RESETTING the time to keep the tempo
        if(dFloat >= maxDelay)
        {
            dFloat = 0.0f;
        }
        if(dFloat2 >= maxDelay)
        {
            dFloat2 = 0.0f;
        }
        // APPLY crossfade
        if(dFloat > maxDelay - (maxDelay/2.0f))
        {
            xFade.setTargetValue(0.0f);
            xFade2.setTargetValue(1.0f);
        }
        else if(dFloat2 > maxDelay - (maxDelay/2.0f))
        {
            xFade.setTargetValue(1.0f);
            xFade2.setTargetValue(0.0f);
        }
        // START 2nd buffer
        if(dFloat > (maxDelay) * 0.5f )
        {
            delayXfadeBuffer = false;
        }
    }
    else if(semitones > 0) // Pitch up
    {
        if(dFloat <= 0)
        {
            dFloat = maxDelay;
        }
        if(dFloat2 <= 0)
        {
            dFloat2 = maxDelay;
        }
        
        if(dFloat < maxDelay/2.0f)
        {
            xFade.setTargetValue(0.0f);
            xFade2.setTargetValue(1.0f);
        }
        else if(dFloat2 < maxDelay/2.0f)
        {
            xFade.setTargetValue(1.0f);
            xFade2.setTargetValue(0.0f);
        }
        
        if(dFloat < (maxDelay) * 0.5f )
        {
            delayXfadeBuffer = false;
        }
    }
 
    dFloat = dFloat + dRate;
   
    if(!delayXfadeBuffer)
    {
        dFloat2 = dFloat2 + dRate;
    }
}

void Fx::processPitchShift(int &channel, float &inSample) {

    if(valuechanged) //MUTE output to avoid zipper when changing semitones
    {
        while(mute.getNextValue() !=0)
        {
            inSample =  0;
        }
        valuechanged = false;
        mute.setTargetValue(1);
    }
    else
    {
        dlPitchShift.pushSample(channel, inSample);
        dlPitchShift2.pushSample(channel, inSample);
        float output = dlPitchShift.popSample(channel, dFloat);
        float output2 = dlPitchShift2.popSample(channel, dFloat2);
        inSample =  mute.getNextValue() * output * xFade.getNextValue() +  mute.getNextValue()*output2 * xFade2.getNextValue();
    }
   
}









