/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModularVSTAudioProcessor::ModularVSTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ModularVSTAudioProcessor::~ModularVSTAudioProcessor()
{
}

//==============================================================================
const juce::String ModularVSTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ModularVSTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ModularVSTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ModularVSTAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ModularVSTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ModularVSTAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ModularVSTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ModularVSTAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ModularVSTAudioProcessor::getProgramName (int index)
{
    return {};
}

void ModularVSTAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ModularVSTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;
    
    initActions = {
        addInstrumentAction,
        addResonatorModuleAction,
        addResonatorModuleAction
    };
    
    
    initModuleTypes = {
        stiffString,
        membrane
    };
    
    int numModules = 0;
    for (int i = 0; i < initActions.size(); ++i)
        if (initActions[i] == addResonatorModuleAction)
            ++numModules;
    
    if (numModules != initModuleTypes.size())
        std::cout << "WRONG NUMBER OF MODULE TYPES ASSIGNED. CHANGE initModuleTypes VECTOR TO MATCH THE AMOUNT OF 'addResonatorModuleAction's IN initModuleTypes." << std::endl;
    
    
    if (instruments.size() == 0)
    {
        instruments.reserve (8);
    }
    else
    {
        for (auto inst : instruments)
            if (inst->getFs() != fs)
                inst->initialise (fs);
            
    }
    
    
    int  j = 0;
    NamedValueSet parameters;
    for (int i = 0; i < initActions.size(); ++i)
    {
        switch (initActions[i]) {
            case addInstrumentAction:
                addInstrument();
                break;
            case addResonatorModuleAction:
                switch (initModuleTypes[j]) {
                    case stiffString:
                        parameters = Global::defaultStringParameters;
                        break;
                    case bar:
                        parameters = Global::defaultBarParameters;
                        break;
                    case membrane:
                        parameters = Global::defaultMembraneParameters;
                        break;
                    case thinPlate:
                        parameters = Global::defaultThinPlateParameters;
                        break;
                    case stiffMembrane:
                        parameters = Global::defaultStiffMembraneParameters;
                        break;
                    default:
                        break;
                }
                addResonatorModule (initModuleTypes[j], parameters);
                ++j;
                break;
            default:
                break;
        }
    }
    
}

void ModularVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ModularVSTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
//  #if JucePlugin_IsMidiEffect
//    juce::ignoreUnused (layouts);
//    return true;
//  #else
//    // This is the place where you check if the layout is supported.
//    // In this template code we only support mono or stereo.
//    // Some plugin hosts, such as certain GarageBand versions, will only
//    // load plugins that support stereo bus layouts.
//    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
//     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
//        return false;
//
//    // This checks if the input layout matches the output layout
//   #if ! JucePlugin_IsSynth
//    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
//        return false;
//   #endif
//
//    return true;
//  #endif
}
#endif

void ModularVSTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
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

    int numChannels = buffer.getNumChannels();
    
    // Get pointers to output locations
    float* const channelData1 = buffer.getWritePointer (0, 0);
    float* const channelData2 = numChannels > 1 ? buffer.getWritePointer (1, 0) : nullptr;

    std::vector<float> totOutput (buffer.getNumSamples(), 0.0f);

    std::vector<float* const*> curChannel {&channelData1, &channelData2};
    
    if (setToZero)
        for (auto inst : instruments)
            inst->setStatesToZero();
    
    for (auto inst : instruments)
    {
        // check whether the instrument is ready
        if (!inst->areModulesReady())
            return;
        
        inst->checkIfShouldExcite();

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            inst->calculate();
            inst->solveInteractions();
#ifdef CALC_ENERGY
            inst->calcTotalEnergy();
//#ifdef CALC_ENERGY
            std::cout << instruments[0]->getTotalEnergy() << std::endl;
//#endif

#endif
            inst->update();

            totOutput[i] += inst->getOutput();
        }
        
    }
    
    // limit output
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        for (int channel = 0; channel < numChannels; ++channel)
            curChannel[channel][0][i] = outputLimit (totOutput[i]);
//    std::cout << totOutput[15] << std::endl;
    
}

//==============================================================================
bool ModularVSTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ModularVSTAudioProcessor::createEditor()
{
    return new ModularVSTAudioProcessorEditor (*this);
}

//==============================================================================
void ModularVSTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ModularVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModularVSTAudioProcessor();
}

void ModularVSTAudioProcessor::addInstrument()
{
    std::shared_ptr<Instrument> newInstrument = std::make_shared<Instrument> (*this, fs);
    instruments.push_back (newInstrument);
    currentlyActiveInstrument = static_cast<int> (instruments.size()-1);
    
    refreshEditor = true;
}

void ModularVSTAudioProcessor::addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters)
{
    jassert(currentlyActiveInstrument != -1);
    jassert(currentlyActiveInstrument < instruments.size());

    instruments[currentlyActiveInstrument]->addResonatorModule (rmt, parameters);
    refreshEditor = true;
}

void ModularVSTAudioProcessor::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    int i = 0;
    for (auto inst : instruments)
    {
        if (changeBroadcaster == inst.get())
        {
            if (inst->getApplicationState() != normalState)
            {
                return;
            }
            currentlyActiveInstrument = i;
            refreshEditor = true;
            return;
        }
        ++i;
    }
}

void ModularVSTAudioProcessor::setApplicationState (ApplicationState a)
{
    applicationState = a;
    switch (a)
    {
        case normalState:
            for (auto inst : instruments)
                inst->setApplicationState (normalState);

            setStatesToZero (false);
            break;
        case addConnectionState:
        {
            setStatesToZero (true);
            for (auto inst : instruments)
            {
                if (inst == instruments[currentlyActiveInstrument])
                    inst->setAddingConnection (true);
                else
                    inst->setAddingConnection (false);
                inst->setApplicationState (addConnectionState);
            }
            break;
        }
    }
    for (auto inst : instruments)
        inst->setApplicationState (a);
    
}
