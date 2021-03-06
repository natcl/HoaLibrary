
#include "PluginProcessor.h"
#include "PluginEditor.h"

HoaToolsAudioProcessor::HoaToolsAudioProcessor()
{
    m_processor         = new HoaProcessor();
    m_source_manager    = m_processor->getSourceManager();
    m_gui               = gui_mode_map;
}

HoaToolsAudioProcessor::~HoaToolsAudioProcessor()
{
    delete m_processor;
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HoaToolsAudioProcessor();
}

const String HoaToolsAudioProcessor::getName() const
{
    return String("HoaTools");
}

AudioProcessorEditor* HoaToolsAudioProcessor::createEditor()
{
    return new HoaToolsAudioProcessorEditor(this, m_processor);
}

bool HoaToolsAudioProcessor::hasEditor() const
{
    return true;
}

/************************************************************************************/
/***************************** PARAMETERS *******************************************/
/************************************************************************************/

int HoaToolsAudioProcessor::getNumParameters()
{
    return m_processor->getNumberOfSources() * 3;
}

float HoaToolsAudioProcessor::getParameter(int index)
{
    if((index + 1) % 3 == 0)
        return m_source_manager->sourceGetMute(index / 3);
    else if((index + 2) % 3 == 0)
        return m_source_manager->sourceGetOrdinate(index / 3) / 10. - 0.5;
    else
        return m_source_manager->sourceGetAbscissa(index / 3) / 10. - 0.5;
}

void HoaToolsAudioProcessor::setParameter(int index, float newValue)
{
    if((index + 1) % 3 == 0)
    {
        newValue += 0.5;
        m_source_manager->sourceSetMute(index / 3, newValue);
    }
    else if((index + 2) % 3 == 0)
        m_source_manager->sourceSetOrdinate(index / 3, (newValue - 0.5) * 20.);
    else
        m_source_manager->sourceSetAbscissa(index / 3, (newValue - 0.5) * 20.);
    
    AudioProcessorEditor* Editor = NULL;
    Editor = getActiveEditor();
    if(Editor)
        Editor->repaint();
}

float HoaToolsAudioProcessor::getParameterMin(int index)
{
    if((index + 1) % 3 == 0)
        return 0;
    else if((index + 2) % 3 == 0)
        return -10.;
    else
        return -10.;
}

float HoaToolsAudioProcessor::getParameterMax(int index)
{
    if((index + 1) % 3 == 0)
        return 1;
    else if((index + 2) % 3 == 0)
        return 10.;
    else
        return 10.;
}

float HoaToolsAudioProcessor::getParameterDefault(int index)
{
    if((index + 1) % 3 == 0)
        return 0;
    else if((index + 2) % 3 == 0)
        return 0.;
    else
        return 0.;
}

int HoaToolsAudioProcessor::getParameterNumSteps(int index)
{
    if((index + 1) % 3 == 0)
        return 1;
    else if((index + 2) % 3 == 0)
        return 0.001;
    else
        return 0.001;
}

float HoaToolsAudioProcessor::getParameterDefaultValue (int index)
{
    if((index + 1) % 3 == 0)
        return 0;
    else if((index + 2) % 3 == 0)
        return 0.;
    else
        return 0.;
}

const String HoaToolsAudioProcessor::getParameterName (int index)
{
    char text[256];
    if((index + 1) % 3 == 0)
        sprintf(text, "Mute     %i : ", index / 3 + 1);
    else if((index + 2) % 3 == 0)
        sprintf(text, "Ordinate %i : ", index / 3 + 1);
    else
        sprintf(text, "Abscissa %i : ", index / 3 + 1);
       
    return String(text);
}

String HoaToolsAudioProcessor::getParameterLabel(int index) const
{
    char text[256];
    if((index + 1) % 3 == 0)
    {
        if (m_source_manager->sourceGetMute(index / 3))
            sprintf(text, "Muted");
        else
           sprintf(text, "Unmuted"); 
        
    }
    else if((index + 2) % 3 == 0)
        sprintf(text, "%f", m_source_manager->sourceGetOrdinate(index / 3));
    else
        sprintf(text, "%f", m_source_manager->sourceGetAbscissa(index / 3));
    
    return String(text);
}

bool HoaToolsAudioProcessor::isParameterAutomatable (int index) const
{
    return true;
}

bool HoaToolsAudioProcessor::isMetaParameter(int index) const
{
    return false;
}

const String HoaToolsAudioProcessor::getParameterText (int index)
{
    char text[256];
    if((index + 1) % 3 == 0)
    {
        if (m_source_manager->sourceGetMute(index / 3))
            sprintf(text, "Muted");
        else
            sprintf(text, "Unmuted");
        
    }
    else if((index + 2) % 3 == 0)
        sprintf(text, "%f", m_source_manager->sourceGetOrdinate(index / 3));
    else
        sprintf(text, "%f", m_source_manager->sourceGetAbscissa(index / 3));
    
    return String(text);
}

/************************************************************************************/
/***************************** PLAY *************************************************/
/************************************************************************************/


void HoaToolsAudioProcessor::numChannelsChanged()
{
    AudioProcessorEditor* Editor = NULL;
    
    if(getNumInputChannels() < m_processor->getNumberOfSources())
    {
        m_processor->setNumberOfSources(getNumInputChannels());
    }
    if(getNumOutputChannels() < m_processor->getNumberOfLoudspeakers())
    {
        m_processor->setNumberOfLoudspeakers(getNumOutputChannels());
    }
    m_processor->postProcess();
    
    Editor = getActiveEditor();
    if(Editor)
        Editor->repaint();
    bool state = isSuspended();
    if(state)
        suspendProcessing(true);
    updateHostDisplay();
    if(state)
        suspendProcessing(false);
}

void HoaToolsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_processor->prepareToPlay(sampleRate, samplesPerBlock);
}

void HoaToolsAudioProcessor::releaseResources()
{
    
}

void HoaToolsAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    m_processor->process(buffer.getArrayOfChannels());
    setNumberOfInputs(m_processor->getNumberOfSources());
    setNumberOfOutputs(m_processor->getNumberOfLoudspeakers());
    m_processor->postProcess();
}

/************************************************************************************/
/***************************** SAVE METHOD ******************************************/
/************************************************************************************/

void HoaToolsAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    XmlElement xml("HoaToolsSettings");
    
    xml.setAttribute("Order", (int)m_processor->getOrder());    
    xml.setAttribute("NumberOfSources", (int)m_processor->getNumberOfSources());
    xml.setAttribute("NumberOfLoudspeakers", (int)m_processor->getNumberOfLoudspeakers());
    xml.setAttribute("OffsetOfLoudspeakers", (int)m_processor->getOffsetOfLoudspeakers());
    xml.setAttribute("Optimization", (int)m_processor->getOptimization());
    xml.setAttribute("Zoom", (double)m_processor->getSourceManager()->getZoom());
    
    copyXmlToBinary(xml, destData);
}

void HoaToolsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName("HoaToolsSettings"))
        {
            m_processor->setOrder(xmlState->getIntAttribute("Order"));
            m_processor->setNumberOfSources(xmlState->getIntAttribute("NumberOfSources"));
            m_processor->setNumberOfLoudspeakers(xmlState->getIntAttribute("NumberOfLoudspeakers"));
            m_processor->setOffsetOfLoudspeakers(xmlState->getIntAttribute("OffsetOfLoudspeakers"));
            m_processor->setOptimization(xmlState->getIntAttribute("Optimization"));
            m_processor->getSourceManager()->setZoom(xmlState->getDoubleAttribute("Zoom"));
            
            setNumberOfInputs(m_processor->getNumberOfSources());
            setNumberOfOutputs(m_processor->getNumberOfLoudspeakers());
        }
    }
}

/************************************************************************************/
/***************************** OTHER ************************************************/
/************************************************************************************/

const String HoaToolsAudioProcessor::getInputChannelName (int channelIndex) const
{
    char text[256];
    sprintf(text, "Source %i", channelIndex+1);
    return String(text);
}

const String HoaToolsAudioProcessor::getOutputChannelName (int channelIndex) const
{
    char text[256];
    sprintf(text, "Loudspeaker %i", channelIndex+1);
    return String(text);
}

bool HoaToolsAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool HoaToolsAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool HoaToolsAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool HoaToolsAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool HoaToolsAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double HoaToolsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HoaToolsAudioProcessor::getNumPrograms()
{
    return 0;
}

int HoaToolsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HoaToolsAudioProcessor::setCurrentProgram (int index)
{
}

const String HoaToolsAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void HoaToolsAudioProcessor::changeProgramName(int index, const String& newName)
{
    ;
}

