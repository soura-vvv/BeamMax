/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BeamMaxxAudioProcessorEditor::BeamMaxxAudioProcessorEditor (BeamMaxxAudioProcessor& p,juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p),valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&customSlider);
    addAndMakeVisible(doaSlider);
    doaSlider.setRange(0, 360);
    doaSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    
    //doaSlider.addListener (this);
    
    
    setSize (400, 300);
}

BeamMaxxAudioProcessorEditor::~BeamMaxxAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BeamMaxxAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void BeamMaxxAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //doaSlider.setBounds(100,100,300,200);
}

