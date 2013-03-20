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
    : AudioProcessorEditor (ownerFilter),
    nbSources_Slider ("nbSources"),
    nbSpeakers_Slider ("nbSpeakers"),
    speakerOffset_Slider ("speakerOffset"),
    speakerDistance_Slider ("speakerDistance")
{
    bg = ImageCache::getFromMemory (BinaryData::background_jpg, BinaryData::background_jpgSize);
    
    //add the draggable-number-box sliders :
    addAndMakeVisible (&nbSources_Slider);
    nbSources_Slider.addChangeListener(this);
    nbSources_Slider.setRange(1, 64, 1);
    
    addAndMakeVisible (&nbSpeakers_Slider);
    nbSources_Slider.addChangeListener(this);
    nbSpeakers_Slider.setRange(3, 64, 1);
    
    addAndMakeVisible (&speakerOffset_Slider);
    speakerOffset_Slider.addChangeListener(this);
    speakerOffset_Slider.setRange(-180, 180, 1);
    
    addAndMakeVisible (&speakerDistance_Slider);
    speakerDistance_Slider.addChangeListener(this);
    speakerDistance_Slider.setRange(0., 1., 0.01);
    
    // add the Map
    addAndMakeVisible (&theMap);
    theMap.addChangeListener(this);
    
    updateMouseCursor ();
    
    // This is where our plugin's editor size is set.
    setSize (580, 420);
}

HoaplugAudioProcessorEditor::~HoaplugAudioProcessorEditor()
{
    theMap.removeAllChangeListeners();
}

//==============================================================================
void HoaplugAudioProcessorEditor::paint (Graphics& g)
{
    g.drawImage(bg, 0, 0, bg.getWidth(), bg.getHeight(), 0, 0, bg.getWidth(), bg.getHeight());
        
    nbSources_Slider.setBounds (getWidth()-159, 216, 58, 23);
    nbSpeakers_Slider.setBounds (getWidth()-80, 216, 58, 23);
    speakerOffset_Slider.setBounds (getWidth()-80, 278, 62, 23);
    speakerDistance_Slider.setBounds (getWidth()-80, 347, 60, 23);

    //test.setBounds (getWidth()-80, 278, 62, 23);
    
    theMap.setBounds (10, 10, 400, 400);
}

void HoaplugAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &theMap) {
        for (int i = 0; i < nbSources_Slider.getValue(); i++) {
            getProcessor()->setParameterNotifyingHost ( i*2+2, (float) theMap.getSourceAbscissa(i) );
            getProcessor()->setParameterNotifyingHost ( i*2+3, (float) theMap.getSourceOrdinate(i) );
        }
    }
    else if (source == &nbSources_Slider) {
        theMap.setNbSources(nbSources_Slider.getValue());
    }
    else if (source == &nbSpeakers_Slider) {
        theMap.setNbSpeakers(nbSpeakers_Slider.getValue());
    }
    else if (source == &speakerOffset_Slider) {
        theMap.setSpeakerOffset(speakerOffset_Slider.getValue());
    }
    else if (source == &speakerDistance_Slider) {
        theMap.setSpeakerDistance(speakerDistance_Slider.getValue());
        getProcessor()->setParameterNotifyingHost (HoaplugAudioProcessor::m_distance_of_loudspeakers_parameter, (float) speakerDistance_Slider.getValue());
    }
}

