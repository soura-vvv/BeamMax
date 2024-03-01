/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BeamMaxxAudioProcessor::BeamMaxxAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::ambisonic(7), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),parameters(*this,nullptr,juce::Identifier("BeamMaxx"),{
         
         std::make_unique<juce::AudioParameterInt>("numMicrophones", "Microphones", 2, 64, 2),
         std::make_unique<juce::AudioParameterFloat>("interMicrophoneDistance", "Distance", 0.01f, 0.5f, 0.05),
         std::make_unique<juce::AudioParameterInt>("directionOfArrival", "Direction Of Arrival", 0, 360, 90)
     })
#endif
{
    numMicrophones=parameters.getRawParameterValue("numMicrophones");
    interMicrophoneDistance=parameters.getRawParameterValue("interMicrophoneDistance");
    directionOfArrival=parameters.getRawParameterValue("directionOfArrival");
}

BeamMaxxAudioProcessor::~BeamMaxxAudioProcessor()
{
}

//==============================================================================
const juce::String BeamMaxxAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BeamMaxxAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BeamMaxxAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BeamMaxxAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BeamMaxxAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BeamMaxxAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BeamMaxxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BeamMaxxAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BeamMaxxAudioProcessor::getProgramName (int index)
{
    return {};
}

void BeamMaxxAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BeamMaxxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    this->sampleRate=sampleRate;
    resetDelays();
}

void BeamMaxxAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BeamMaxxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    
    
//    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
//     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
//        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BeamMaxxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if(prevNumMicrophones!=*numMicrophones)
    {
        resetDelays();
    }
    
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    
    delSampleMax=round(*numMicrophones*sampleRate*(*interMicrophoneDistance)*cos(*directionOfArrival)/c);
    DBG(delSampleMax);
    for(int channel=0;channel<*numMicrophones-1;channel++)
    {
        delSamples=round((channel+1)*sampleRate*(*interMicrophoneDistance)*cos(*directionOfArrival)/c);
        
        if(delSampleMax>0)
        {
            readIndices[channel+1]=writeIndices[channel+1]-delSamples;
            if(readIndices[channel+1]<0)
                readIndices[channel+1]=delayBufferSize+readIndices[channel+1];
            
            for(int sample=0;sample<buffer.getNumSamples();sample++)
            {
                buffer.getWritePointer(channel+1)[sample]=delayBuffer[channel+1][readIndices[channel+1]];
                delayBuffer[channel+1][writeIndices[channel+1]]=buffer.getReadPointer(channel+1)[sample];
            }
        }
        
        //might need to implement new delaybuffer for negative del samples or consider adding an extra channel
        else
        {
            delSamples=abs(delSampleMax-delSamples);
            if(delSamples>0)
            {
                readIndices[channel]=writeIndices[channel]-delSamples;
                if(readIndices[channel]<0)
                    readIndices[channel]=delayBufferSize+readIndices[channel];
                
                for(int sample=0;sample<buffer.getNumSamples();sample++)
                {
                    buffer.getWritePointer(channel)[sample]=delayBuffer[channel][readIndices[channel]];
                    delayBuffer[channel][writeIndices[channel]]=buffer.getReadPointer(channel)[sample];
                
                }
            }
            
                
            
        }
    }
    
    for (int channel = 1; channel < *numMicrophones; ++channel)
    {
        for (int sample=0;sample<buffer.getNumSamples();sample++)
            buffer.getWritePointer(0)[sample]=buffer.getReadPointer(0)[sample]+buffer.getReadPointer(channel)[sample];
        // ..do something to the data...
    }
    for (int sample=0;sample<buffer.getNumSamples();sample++)
        buffer.getWritePointer(0)[sample]=buffer.getReadPointer(0)[sample]/(*numMicrophones);
    
}

//==============================================================================
bool BeamMaxxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BeamMaxxAudioProcessor::createEditor()
{
    //return new BeamMaxxAudioProcessorEditor (*this,parameters);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BeamMaxxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BeamMaxxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeamMaxxAudioProcessor();
}

void BeamMaxxAudioProcessor::resetDelays()
{
    delayBuffer.resize(*numMicrophones);
    for(int i=0;i<*numMicrophones;i++)
    {
        delayBuffer[i].resize(delayBufferSize,0.0f);
    }
    
    readIndices.resize(*numMicrophones,0);
    writeIndices.resize(*numMicrophones,0);
    prevNumMicrophones=*numMicrophones;
}
