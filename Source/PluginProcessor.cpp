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
                       ), apvts(*this, nullptr, "apvts", initializeGUI())
#endif
{
    apvts.addParameterListener("mix", this);
    apvts.addParameterListener("sync", this);
    apvts.addParameterListener("choice", this);
    apvts.addParameterListener("delayTime", this);
    apvts.addParameterListener("feedback", this);
    apvts.addParameterListener("lp", this);
    apvts.addParameterListener("hp", this);
    apvts.addParameterListener("drive", this);
    apvts.addParameterListener("psInterval", this);
    apvts.addParameterListener("pingpong", this);
    apvts.addParameterListener("reverse", this);
}

EchoDlineAudioProcessor::~EchoDlineAudioProcessor()
{
    apvts.removeParameterListener("mix", this);
    apvts.removeParameterListener("sync", this);
    apvts.removeParameterListener("choice", this);
    apvts.removeParameterListener("delayTime", this);
    apvts.removeParameterListener("feedback", this);
    apvts.removeParameterListener("lp", this);
    apvts.removeParameterListener("hp", this);
    apvts.removeParameterListener("drive", this);
    apvts.removeParameterListener("psInterval", this);
    apvts.removeParameterListener("pingpong", this);
    apvts.removeParameterListener("reverse", this);
}
juce::AudioProcessorValueTreeState::ParameterLayout EchoDlineAudioProcessor::initializeGUI()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"mix",1},"Dry/Wet",juce::NormalisableRange<float>(0.0f,100.0f,1.0f),50.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"sync",1}, "Sync", true));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"choice",1}, "Delay Time", juce::StringArray{"1/16 triplet", "1/32 dotted","1/16", "1/8 triplet", "1/16 dotted", "1/8", "1/4 triplet", "1/8 dotted","1/4", "1/2 triplet", "1/4 dotted", "1/2","1/1 triplet", "1/2 dotted"} ,7));
   
    // Set to ms 1-4000 ms
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"delayTime", 1},"Delay Time MS",juce::NormalisableRange<float>(50.0f,4000.0f,1.f, 0.5f),375.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"feedback",1},"Feedback",juce::NormalisableRange<float>(0.0f,100.0f,1.0f), 20.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lp",1}, "High Cut", juce::NormalisableRange<float>(0.0f, 20000.0f, 1.0f, 0.5f), 20000.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"hp",1}, "Low Cut", juce::NormalisableRange<float>(0.0f, 20000.0f, 1.0f, 0.5f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive",1}, "Drive", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"psInterval",1}, "Pitch Interval", juce::NormalisableRange<float>(-12.0f, 12.0f, 1.0f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"pingpong",1}, "Ping Pong", true));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"reverse",1}, "Reverse", true));
   
   
    return {params.begin(), params.end()};

}

void EchoDlineAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    
    //---------------Delayline------------------
    if (parameterID == "mix")
    {
        delayLine.setParameters(DelayLine::ParameterId::mix,newValue);
    }
    if (parameterID == "sync")
    {
        delayLine.setParameters(DelayLine::ParameterId::sync,newValue);
    }
    
    if (parameterID == "pingpong")
    {
        delayLine.setParameters(DelayLine::ParameterId::pingpong,newValue);
    }
    
    if (parameterID == "choice")
    {
        delayLine.setParameters(DelayLine::ParameterId::choice,newValue);
    }
    if (parameterID == "delayTime")
    {
        delayLine.setParameters(DelayLine::ParameterId::delayTime,newValue);
    }
    if (parameterID == "feedback")
    {
        delayLine.setParameters(DelayLine::ParameterId::feedback,newValue);
    }
    
    if (parameterID == "reverse")
    {
        delayLine.setParameters(DelayLine::ParameterId::reverse,newValue);
    }
    
    //---------------FX------------------
    if (parameterID == "lp")
    {
        delayLine.fxChain.setParameters(Fx::ParameterId::lp,newValue);
    }
    if (parameterID == "hp")
    {
        delayLine.fxChain.setParameters(Fx::ParameterId::hp,newValue);
    }
    
    if (parameterID == "drive")
    {
        delayLine.fxChain.setParameters(Fx::ParameterId::sDrive,newValue);
    }
    
    if (parameterID == "psInterval")
    {
        delayLine.fxChain.setParameters(Fx::ParameterId::psInterval,newValue);
    }
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
 
    //CREATE and SEND spec
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    
    //FX
    delayLine.fxChain.prepare(sampleRate, getTotalNumInputChannels(), samplesPerBlock, spec);
    delayLine.fxChain.setParameters(Fx::ParameterId::sDrive,*apvts.getRawParameterValue("drive"));
    delayLine.fxChain.setParameters(Fx::ParameterId::lp,*apvts.getRawParameterValue("lp"));
    delayLine.fxChain.setParameters(Fx::ParameterId::hp,*apvts.getRawParameterValue("hp"));
    delayLine.fxChain.setParameters(Fx::ParameterId::psInterval,*apvts.getRawParameterValue("psInterval"));
    //DELAYLINE
    delayLine.prepare(sampleRate, getTotalNumInputChannels(), samplesPerBlock, spec);
    delayLine.setParameters(DelayLine::ParameterId::choice,*apvts.getRawParameterValue("choice"));
    delayLine.setParameters(DelayLine::ParameterId::delayTime,*apvts.getRawParameterValue("delayTime"));
    delayLine.setParameters(DelayLine::ParameterId::mix,*apvts.getRawParameterValue("mix"));
    delayLine.setParameters(DelayLine::ParameterId::feedback,*apvts.getRawParameterValue("feedback"));
    delayLine.setParameters(DelayLine::ParameterId::sync,*apvts.getRawParameterValue("sync"));
    delayLine.setParameters(DelayLine::ParameterId::pingpong,*apvts.getRawParameterValue("pingpong"));
//    
//    playBuffer.setSize(2, sampleRate * 5);
//    captureBuffer.setSize(2, sampleRate * 10);
//    playBufferIndex = 0;
//    captureBufferIndex = 0;
//    curSampleRate = sampleRate;
//    reversedBuffer.setSize(2, sampleRate * 5);
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
    
    
    //UPDATE the delay time here (every buffer)
    delayLine.setDelayTarget();

    // CHECK if bpm info is available, if not BPM = 120
    
    delayLine.curBarPosition = *getPlayHead()->getPosition()->getPpqPosition();
    
    if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm())
    {
        delayLine.bpm = bpmFromHost;
    }
    
    auto writePtrs = buffer.getArrayOfWritePointers();
  //  auto revPtrs = reversedBuffer.getArrayOfWritePointers();

    int channelLeft = 0;
    int channelRight = 1;
    
    //TEMP
  
    //
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
      
        
        //SPLIT into channels left and right
        float& inputLeft = writePtrs[channelLeft][sample];
        float& inputRight = writePtrs[channelRight][sample];

        delayLine.processDelay(inputLeft, inputRight);
        
        writePtrs[channelLeft][sample] = inputLeft;
        writePtrs[channelRight][sample] = inputRight;
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
    apvts.state.writeToStream(stream);
}

void EchoDlineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto* editor = getActiveEditor())
    {
        editor->setSize (getEditorWidth(), getEditorHeight());
    }
    
    //call the save state
    auto tree = juce::ValueTree::readFromData(data, size_t (sizeInBytes));
    
    if(tree.isValid())
    {
        apvts.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EchoDlineAudioProcessor();
}
