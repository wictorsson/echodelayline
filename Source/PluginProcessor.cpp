/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
EchoDlineAudioProcessor::EchoDlineAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "PARAMETERS", initializeGUI())
#endif
{
    parameters.addParameterListener("mix", this);
    parameters.addParameterListener("sync", this);
    parameters.addParameterListener("choice", this);
    parameters.addParameterListener("delayTime", this);
    parameters.addParameterListener("feedback", this);
    parameters.addParameterListener("lp", this);
    parameters.addParameterListener("hp", this);
    parameters.addParameterListener("drive", this);
}

EchoDlineAudioProcessor::~EchoDlineAudioProcessor()
{
    parameters.removeParameterListener("mix", this);
    parameters.removeParameterListener("sync", this);
    parameters.removeParameterListener("choice", this);
    parameters.removeParameterListener("delayTime", this);
    parameters.removeParameterListener("feedback", this);
    parameters.removeParameterListener("lp", this);
    parameters.removeParameterListener("hp", this);
    parameters.removeParameterListener("drive", this);
}
juce::AudioProcessorValueTreeState::ParameterLayout EchoDlineAudioProcessor::initializeGUI()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"mix",1},"Dry/Wet",juce::NormalisableRange<float>(0.0f,100.0f,1.0f),50.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"sync",1}, "Sync", true));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"choice",1}, "Delay Time", juce::StringArray{"1/16 triplet", "1/32 dotted","1/16", "1/8 triplet", "1/16 dotted", "1/8", "1/4 triplet", "1/8 dotted","1/4", "1/2 triplet", "1/4 dotted", "1/2","1/1 triplet", "1/2 dotted"} ,7));
   
    // Set to ms 1-4000 ms
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"delayTime", 1},"Delay Time MS",juce::NormalisableRange<float>(1.0f,4000.0f,1.f, 0.5f),375.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"feedback",1},"Feedback",juce::NormalisableRange<float>(0.0f,100.0f,1.0f), 20.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lp",1}, "High Cut", juce::NormalisableRange<float>(0.0f, 20000.0f, 1.0f, 0.5f), 20000.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"hp",1}, "Low Cut", juce::NormalisableRange<float>(0.0f, 20000.0f, 1.0f, 0.5f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive",1}, "Drive", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 0.0f));
   
   
    return {params.begin(), params.end()};

}

void EchoDlineAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Byt ut till newValue
    if (parameterID == "mix")
    {
        mix.setTargetValue(newValue/100);
    }
    if (parameterID == "sync")
    {
        syncButton = !syncButton;
        delayLine.reset();
    }
    if (parameterID == "choice")
    {
        syncedDelayChoice = setSyncedDelayFromChoice(newValue);
    }
    if (parameterID == "delayTime")
    {
        samplesInSec = (newValue/1000.0f);
    }
    if (parameterID == "feedback")
    {
        feedback.setTargetValue(newValue/100.0f);
    }
    if (parameterID == "lp")
    {
        lpFilter.setCutoffFrequency(newValue);
        if(newValue < 20000)
        {
            lpOn = true;
        }
        else
        {
            lpOn = false;
        }
    }
    if (parameterID == "hp")
    {
        hpFilter.setCutoffFrequency(newValue);
    }
        if(newValue > 0)
        {
            hpOn = true;
        }
        else
        {
            hpOn = false;
        }
    
    if (parameterID == "drive")
    {
        saturationDrive = newValue;
        if(newValue > 0.0f)
        {
            saturationOn = true;
        }
        else
        {
            saturationOn = false;
        }
    }
}

float EchoDlineAudioProcessor::setSyncedDelayFromChoice(float choice)
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
//==============================================================================
const juce::String EchoDlineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EchoDlineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EchoDlineAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EchoDlineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EchoDlineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EchoDlineAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EchoDlineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EchoDlineAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EchoDlineAudioProcessor::getProgramName (int index)
{
    return {};
}

void EchoDlineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EchoDlineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySampleRate = sampleRate;
    
    //DSP
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    delayLine.reset();
    delayLine.setMaximumDelayInSamples(10*sampleRate);
    delayLine.prepare(spec);

    //Params
    samplesOfDelay.reset(sampleRate, 0.8f);
    syncedDelayChoice = setSyncedDelayFromChoice(*parameters.getRawParameterValue("choice"));
    samplesOfDelay.setTargetValue(*parameters.getRawParameterValue("delayTime"));
    mix.reset(sampleRate, 0.05f);
    mix.setTargetValue(*parameters.getRawParameterValue("mix")/100);
    feedback.reset(sampleRate, 0.05f);
    feedback.setTargetValue(*parameters.getRawParameterValue("feedback")/100.0f);
    saturationDrive.reset(sampleRate, 0.05f);
    saturationDrive.setTargetValue(*parameters.getRawParameterValue("drive"));
    samplesInSec = *parameters.getRawParameterValue("delayTime")/1000.0f;
    
    //Filter
    lpFilter.reset();
    lpFilter.setCutoffFrequency(*parameters.getRawParameterValue("lp"));
    lpFilter.prepare(spec);
    hpFilter.reset();
    hpFilter.setCutoffFrequency(*parameters.getRawParameterValue("hp"));
    hpFilter.prepare(spec);
    lpFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    hpFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
}

void EchoDlineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EchoDlineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EchoDlineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
   
    samplesOfDelay.setTargetValue(updateDelayTime());
    
    auto writePtrs = buffer.getArrayOfWritePointers();
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float smoothedDelaytime = samplesOfDelay.getNextValue();
        float smoothedFeedback = feedback.getNextValue();
        float smoothedMix = mix.getNextValue();
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float input = writePtrs[channel][sample];
            float processedFXSample = input;
            processFX(channel, processedFXSample);
            delayLine.pushSample(channel, processedFXSample + feedBackSignals[channel] * smoothedFeedback);
            float output = delayLine.popSample(channel, smoothedDelaytime);
            feedBackSignals[channel] = output;
            output = output * smoothedMix + input * (1.0f - smoothedMix);
            writePtrs[channel][sample] = output;
        }
    }
}

float EchoDlineAudioProcessor::updateDelayTime()
{
    if(syncButton)
    {
        // Check if bpm info is available, if not BPM = 120
        if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm())
        {
            bpm = bpmFromHost;
        }
        float beatsPerSec = bpm / 60;
        float secPerBeat = 1/beatsPerSec;
        return (syncedDelayChoice * (secPerBeat * mySampleRate));
    }
    return (samplesInSec * mySampleRate);
}

void EchoDlineAudioProcessor::processFX(int channel, float& inSample)
{
    if(saturationOn)
    {
        inSample = inSample * (1.0F - saturationDrive.getNextValue()/10.0F) + piDiv * std::atanf(inSample * saturationDrive.getNextValue());
    }
  
    if(lpOn)
    {
        inSample = lpFilter.processSample(channel, inSample);
    }
    if(hpOn)
    {
        inSample = hpFilter.processSample(channel, inSample);
    }
    
}
//==============================================================================
bool EchoDlineAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EchoDlineAudioProcessor::createEditor()
{
    return new EchoDlineAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EchoDlineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    parameters.state.writeToStream(stream);
}

void EchoDlineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    //call the save state
    auto tree = juce::ValueTree::readFromData(data, size_t (sizeInBytes));
    
    if(tree.isValid())
    {
        parameters.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EchoDlineAudioProcessor();
}
