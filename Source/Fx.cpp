/*
  ==============================================================================

    Fx.cpp
    Created: 8 Nov 2022 10:47:05am
    Author:  Fredrik Wictorsson

  ==============================================================================
*/

#include "Fx.h"
#include "DelayLine.h"

void Fx::prepare(float sampleRate, int numchans, float samplesPerBlock, juce::dsp::ProcessSpec spec)
{
    mySampleRate = sampleRate;
    
    saturationDrive.reset(sampleRate, 0.05f);
    lpSmoothed.reset(sampleRate, 0.05f);
    hpSmoothed.reset(sampleRate, 0.05f);
    flutterDial.reset(sampleRate, 0.5f);

    mute.reset(sampleRate, 0.60f);
   
    //Filter
    lpFilter.reset();
    lpFilter.prepare(spec);
    hpFilter.reset();
    hpFilter.prepare(spec);
    lpFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    hpFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    
    dlPitchShift.reset();
    dlPitchShift.setMaximumDelayInSamples(mySampleRate * 10);
    dlPitchShift.prepare(spec);
    dlPitchShift2.reset();
    dlPitchShift2.setMaximumDelayInSamples(mySampleRate * 10);
    dlPitchShift2.prepare(spec);
    
    dlFlutter.reset();
    dlFlutter.setMaximumDelayInSamples(mySampleRate * 10);
    dlFlutter.prepare(spec);
 
    xFade.reset(sampleRate, 0.0625f);
    xFade.setTargetValue(1.0f);
    xFade2.reset(sampleRate, 0.0625f);
    xFade2.setTargetValue(0.0f);
    
    samplesOfDelayFluttSmooth.reset(sampleRate, 0.8f);
  
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
            
            if(semitones != 0)
            {
                pitchShiftToggle = true;
            }
            else
            {
                pitchShiftToggle = false;
            }
            break;
        }
        case Fx::ParameterId::flutter:
        {
            settingFlutter = true;
            flutterDial.setTargetValue(paramValue*0.03);
        
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

void Fx::processFlutter( float& inSample, float& inSampleRight)
{
    //LFO
    auto rateRandomInt = juce::Random::getSystemRandom().nextInt (65) + 35;
    float tempRate = rateRandomInt;
    if(flutterRateConv > 0)
    {
        settingFlutter = true;
    }
    else
    {
        settingFlutter = false;
    }
    if(flutterRate <= mySampleRate*0.1 && !lfoDown)
    {
        rate = tempRate;
    }
    else if(flutterRate >= mySampleRate*0.1)
    {
        lfoDown = true;
        rate = -tempRate;
    }
    flutterRate = flutterRate + rate;
    flutterRateConv = flutterRate/mySampleRate*flutterDial.getNextValue();

    if(flutterRate <= 0)
    {
        lfoDown = false;
    }
 
    float samplesOfDelay = samplesOfDelayFluttSmooth.getNextValue();
  //  DBG(dlFlutter.getMaximumDelayInSamples());
    if(samplesOfDelay < 1 || samplesOfDelay >= mySampleRate * 8)
    {
        samplesOfDelay = 1;
    }
    dlFlutter.pushSample(0, inSample );
    dlFlutter.pushSample(1, inSampleRight );
        
    float outputLeftFlutter = dlFlutter.popSample(0, samplesOfDelay);
    float outputRightFlutter = dlFlutter.popSample(1, samplesOfDelay);
        
    inSample = outputLeftFlutter;
    inSampleRight = outputRightFlutter;
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
        // APPLY crossfade
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
    //Offset buffer 2
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
        inSample =  mute.getNextValue() * output * xFade.getNextValue() + mute.getNextValue()*output2 * xFade2.getNextValue();
    }
}

void Fx::updateDelayTime()
{
    currentDelayTime2 = flutterRateConv*mySampleRate;
    samplesOfDelayFluttSmooth.setTargetValue(currentDelayTime2);
}






