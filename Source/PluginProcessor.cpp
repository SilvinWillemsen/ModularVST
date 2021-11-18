/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "pugixml.hpp"

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
    if (instruments.size() != 0)
    {
        instruments.clear();
        instruments.reserve (8);
    }
    fs = sampleRate;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("savedPreset.xml");
    pugi::xml_node node = doc.child("App");
    pugi::xml_node instrum = node.child("Instrument");
    
<<<<<<< Updated upstream
    initActions = {
        addInstrumentAction,
        addResonatorModuleAction,
        addResonatorModuleAction,
//        addInstrumentAction,
//        addResonatorModuleAction,
//        addInstrumentAction,
//        addResonatorModuleAction,
//        addResonatorModuleAction
    };
=======
>>>>>>> Stashed changes
    
    juce::Logger::getCurrentLogger()->outputDebugString("hello");
    juce::Logger::getCurrentLogger()->outputDebugString(instrum.child("Resonator").child("PARAM").attribute("id").value());
    juce::Logger::getCurrentLogger()->outputDebugString(doc.child("App").child("Instrument").child("Resonator").child("PARAM").attribute("value").value());
    juce::Logger::getCurrentLogger()->outputDebugString(instrum.child("Connection").attribute("type").value());
    
<<<<<<< Updated upstream
    initModuleTypes = {
        stiffString,
        thinPlate,
    };
=======
    std::vector<std::string> resType{};
    std::vector<int> resonNum;
    int i = 0;
    for (pugi::xml_node inst : node.children("Instrument")) {
        
        resonNum.push_back(i);
        for (pugi::xml_node reso : inst.children("Resonator"))
        {
            resonNum[i]++;
            juce::Logger::getCurrentLogger()->outputDebugString("Resonator:");
            for (pugi::xml_attribute attr : reso.attributes())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(attr.name());
                juce::Logger::getCurrentLogger()->outputDebugString(attr.value());
                auto attrib = *(attr.name());
                if (attrib == 't') {
                    resType.push_back(attr.value());
                }
            }
            juce::Logger::getCurrentLogger()->outputDebugString("Parameters:");  // not yet done, now just printing out
            for (pugi::xml_node child : reso.children())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(child.attribute("id").value());
                juce::Logger::getCurrentLogger()->outputDebugString(child.attribute("value").value());
            }
        }
        for (pugi::xml_node reso : instrum.children("Connection"))  // not yet done, now just printing out
        {
            juce::Logger::getCurrentLogger()->outputDebugString("Connections:");

            for (pugi::xml_attribute attr : reso.attributes())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(attr.name());
                juce::Logger::getCurrentLogger()->outputDebugString(attr.value());
            }
        }
        i++;
    }
    for (int i = 0; i < resonNum.size(); i++)
    {
        initActions.push_back(addInstrumentAction);
        for (int j = 0; j < resonNum[i]; j++)
        {
            initActions.push_back(addResonatorModuleAction);
        }
    }

    //initActions = {
      //  addInstrumentAction,
        /*addInstrumentAction,
        addResonatorModuleAction,
        addInstrumentAction,
        addResonatorModuleAction,
        addResonatorModuleAction*/
   //  addResonatorModuleAction, addResonatorModuleAction, addResonatorModuleAction };
    
    for (int i = 0; i < resType.size(); i++) {
        if (resType[i] == "Stiff_String") { initModuleTypes.push_back(stiffString); }
        else if (resType[i] == "Bar") { initModuleTypes.push_back(bar); }
        else if (resType[i] == "Thin_Plate") { initModuleTypes.push_back(thinPlate); }
        else if (resType[i] == "Membrane") { initModuleTypes.push_back(membrane); } 
        else if (resType[i] == "Stiff_Membrane") { initModuleTypes.push_back(stiffMembrane); }
    }

    /*initModuleTypes = {
        stiffString,
        stiffString,
        thinPlate,
        bar,
        bar,
        membrane
    };*/
>>>>>>> Stashed changes
    
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

    std::vector<float> totOutputL (buffer.getNumSamples(), 0.0f);
    std::vector<float> totOutputR (buffer.getNumSamples(), 0.0f);

    std::vector<float* const*> curChannel {&channelData1, &channelData2};
    
    for (auto inst : instruments)
        if (inst->shouldRemoveInOrOutput())
            inst->removeInOrOutput();
    
    if (setToZero)
    {
        for (auto inst : instruments)
            inst->setStatesToZero();
        return;
    }
    
    for (auto inst : instruments)
    {
        // check whether the instrument is ready
        if (!inst->areModulesReady())
            return;
        
        inst->checkIfShouldExcite();
        
//        if (inst->checkIfShouldRemoveResonatorModule())
//        {
//            inst->removeResonatorModule();
//            refreshEditor = true;
//        }
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            inst->calculate();
            inst->solveInteractions();
#ifdef CALC_ENERGY
            inst->calcTotalEnergy();
//#ifdef CALC_ENERGY
            std::cout << inst->getTotalEnergy() << std::endl;
//#endif

#endif
            inst->update();

            totOutputL[i] += inst->getOutputL();
            totOutputR[i] += inst->getOutputR();
        }
        
    }
    
    // limit output
    for (int channel = 0; channel < numChannels; ++channel)
    {
        if (channel == 0)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                curChannel[channel][0][i] = outputLimit (totOutputL[i]);
        }
        else if (channel == 1)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                curChannel[channel][0][i] = outputLimit (totOutputR[i]);
        }
    }

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
            setStatesToZero (false);
            break;
        case editInOutputsState:
        {
            setStatesToZero (true);
            highlightInstrument (currentlyActiveInstrument);
            break;
        }
        case editConnectionState:
        {
            setStatesToZero (true);
            highlightInstrument (currentlyActiveInstrument);
            break;
        }
        case removeResonatorModuleState:
        {
            setStatesToZero (true);
            break;
        }
    }
    for (auto inst : instruments)
        inst->setApplicationState (a);
    
}

void ModularVSTAudioProcessor::highlightInstrument (int instrumentToHighlight)
{
    for (auto inst : instruments)
        inst->setHighlightedInstrument (false);
    instruments[instrumentToHighlight]->setHighlightedInstrument (true);

}

void ModularVSTAudioProcessor::savePreset()
{
    std::ofstream file;

    file.open("savedPreset.xml");
    file << "<App" << ">" << "\n";
    for (int i = 0; i < instruments.size(); ++i)
    {
        int numResonators = instruments[i]->getNumResonatorModules();
        file << "\t " << "<Instrument id=\"" << i << "\">"<< "\n";

        for (int r = 0; r < numResonators; ++r)
        {
            ResonatorModule* curResonator =instruments[i]->getResonatorPtr(r);
            // type

            file << "\t " << "\t " << "<Resonator id=\"" << i <<"_"<< r << "r\" type=\"";
            switch (curResonator->getResonatorModuleType()) {
                case stiffString:
                    file << "Stiff_String\">";
                    break;
                case bar:
                    file << "Bar\">";
                    break;
                case acousticTube:
                    file << "Acoustic_Tube\">";
                    break;
                case membrane:
                    file << "Membrane\">";
                    break;
                case thinPlate:
                    file << "Thin_Plate\">";
                    break;
                case stiffMembrane:
                    file << "Stiff_Membrane\">";
                    break;

                default:
                    break;
            }
            file << "\n";

            for (int p = 0; p < curResonator->getParamters().size(); ++p)
            {
                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << r << "r_";
                String paramName = curResonator->getParamters().getName(p).toString();
                double value = *curResonator->getParamters().getVarPointer (paramName);
                file << paramName << "\" value=\"" << value << "\"/>\n";
            }

            file << "\t " << "\t " << "</Resonator>" << "\n";

        }
        int numConnections = (int)instruments[i]->getConnectionInfo()->size();
        //switch (numConnections) {
        //    case 0:
        //        file << "... and " << numConnections << " connections."<< "\n";
        //        break;
        //    case 1:
        //        file << "... and " << numConnections << " connection:"<< "\n";
        //        break;
        //    default:
        //        file << "... and " << numConnections << " connections:"<< "\n";
        //        break;
        //}
        
        for (int c = 0; c < numConnections; ++c)
        {
            file << "\t " << "\t " << "<Connection id=\"" << i << "_" << c << "c\" type=\"";
            String connectionTypeString;
            switch (instruments[i]->getConnectionInfo()[0][c].connType) {
                case rigid:
                    connectionTypeString = "rigid";
                    break;
                case linearSpring:
                    connectionTypeString = "linear";
                    break;
                case nonlinearSpring:
                    connectionTypeString = "nonlinear";
                    break;

                default:
                    break;
            }
            
            file << connectionTypeString <<"\" fromResonator=\""
            << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\" fromLocation=\""
            << instruments[i]->getConnectionInfo()[0][c].loc1
            << "\" toResonator=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\" toLocation=\""
            << instruments[i]->getConnectionInfo()[0][c].loc2 << "\"/>" << "\n";
        }
        file << "\t " << "</Instrument>" << "\n";
    }
    file << "</App" << ">" << "\n";
    file.close();
}
