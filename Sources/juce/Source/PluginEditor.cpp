/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
HoaplugAudioProcessorEditor::HoaplugAudioProcessorEditor (HoaplugAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
    // This is where our plugin's editor size is set.
    setSize (600, 450);
}

HoaplugAudioProcessorEditor::~HoaplugAudioProcessorEditor()
{
}

//==============================================================================
void HoaplugAudioProcessorEditor::paint (Graphics& g)
{
    g.drawImageAt(ImageCache::getFromFile(File("map.png")), 150, 100);
    g.drawRoundedRectangle(150, 100, 300, 300, 5, 5);
}
