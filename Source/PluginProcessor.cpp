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
    
    if (Global::loadPresetAtStartUp)
    {
        LoadPresetResult res = loadPreset();
        switch (res) {
            case applicationIsNotEmpty:
                DBG ("Application is not empty.");
                break;
            case fileNotFound:
                DBG ("Presetfile not found");
                break;
            case presetNotLoaded:
                DBG ("For whatever reason, the preset was not loaded.");
            case success:
                DBG ("Preset loaded successfully.");
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

void ModularVSTAudioProcessor::addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced)
{
    jassert(currentlyActiveInstrument != -1);
    jassert(currentlyActiveInstrument < instruments.size());

    instruments[currentlyActiveInstrument]->addResonatorModule (rmt, parameters, advanced);
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

    
    file.open (presetPath);
    file << "<App" << ">" << "\n";
    for (int i = 0; i < instruments.size(); ++i)
    {
        int numResonators = instruments[i]->getNumResonatorModules();
        file << "\t " << "<Instrument id=\"" << i << "\">"<< "\n";

        for (int r = 0; r < numResonators; ++r)
        {
            ResonatorModule* curResonator =instruments[i]->getResonatorPtr(r).get();
            // type

            file << "\t " << "\t " << "<Resonator id=\"" << i <<"_"<< r << "_r\" type=\"";
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
                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << r << "_r_";
                String paramName = curResonator->getParamters().getName(p).toString();
                double value = *curResonator->getParamters().getVarPointer (paramName);
                file << paramName << "\" value=\"" << value << "\"/>\n";
            }

            file << "\t " << "\t " << "</Resonator>" << "\n";

        }

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        int numConnections = (int)instruments[i]->getConnectionInfo()->size();

        for (int c = 0; c < numConnections; ++c)
        {
            String connectionTypeString;
            // type

            file << "\t " << "\t " << "<Connection id=\"" << i << "_" << c << "_c\" type=\"";
            switch (instruments[i]->getConnectionInfo()[0][c].connType) {
            case rigid:
                file << "rigid\">";
                break;
            case linearSpring:
                file << "linear\">";
                break;
            case nonlinearSpring:
                file << "nonlinear\">";
                break;
            default:
                break;
            }
            file << "\n";

                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << c << "_c_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << c << "_c_fL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc1 << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << c << "_c_tR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"" << i << "_" << c << "_c_tL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc2 << "\"/>\n";

            file << "\t " << "\t " << "</Connection>" << "\n";


        // ----------------------------------------------------------------------------------
       
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
        
        //for (int c = 0; c < numConnections; ++c)
        //{
        //    file << "\t " << "\t " << "<Connection id=\"" << i << "_" << c << "_c\" type=\"";
        //    String connectionTypeString;
        //    switch (instruments[i]->getConnectionInfo()[0][c].connType) {
        //        case rigid:
        //            connectionTypeString = "rigid";
        //            break;
        //        case linearSpring:
        //            connectionTypeString = "linear";
        //            break;
        //        case nonlinearSpring:
        //            connectionTypeString = "nonlinear";
        //            break;

        //        default:
        //            break;
        //    }
        //    
        //    file << connectionTypeString <<"\" fromResonator=\""
        //    << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\" fromLocation=\""
        //    << instruments[i]->getConnectionInfo()[0][c].loc1
        //    << "\" toResonator=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\" toLocation=\""
        //    << instruments[i]->getConnectionInfo()[0][c].loc2 << "\"/>" << "\n";
        }
        file << "\t " << "</Instrument>" << "\n";
    }
    file << "</App" << ">" << "\n";
    file.close();
}

LoadPresetResult ModularVSTAudioProcessor::loadPreset()
{
    // make sure that application is loaded from scratch
    if (instruments.size() != 0)
        return applicationIsNotEmpty;
    
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file (presetPath);
    if (result.status != pugi::status_ok)
    {
        return fileNotFound;
    }
    pugi::xml_node node = doc.child("App");
    pugi::xml_node instrum = node.child("Instrument");
    
    juce::Logger::getCurrentLogger()->outputDebugString("hello");
    juce::Logger::getCurrentLogger()->outputDebugString(instrum.child("Resonator").child("PARAM").attribute("id").value());
    juce::Logger::getCurrentLogger()->outputDebugString(doc.child("App").child("Instrument").child("Resonator").child("PARAM").attribute("value").value());
    juce::Logger::getCurrentLogger()->outputDebugString(instrum.child("Connection").attribute("type").value());
    
    std::vector< std::vector<std::vector<double>>> params;
    std::vector< std::vector<std::vector<double>>> connects;
    std::vector<String> resoType;
    std::vector<std::vector<String>> connType;
    std::vector<int> resoNum;
    std::vector<int> connNum;
    
    int i = 0;
    for (pugi::xml_node inst : node.children("Instrument"))
    {
        resoNum.push_back(0);
        connNum.push_back(0);
        params.push_back(std::vector< std::vector<double>>());
        connects.push_back(std::vector< std::vector<double>>());
        int j = 0;
        int c = 0;
        for (pugi::xml_node reso : inst.children("Resonator"))
        {
            params[i].push_back(std::vector<double>());
            ++resoNum[i];
            juce::Logger::getCurrentLogger()->outputDebugString("Resonator:");
            for (pugi::xml_attribute resoAttr : reso.attributes())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(resoAttr.name());
                juce::Logger::getCurrentLogger()->outputDebugString(resoAttr.value());
                auto attrib = *(resoAttr.name());
                if (attrib == 't') {
                    resoType.push_back(resoAttr.value());
                }
            }
            juce::Logger::getCurrentLogger()->outputDebugString("Parameters:");
            
            for (pugi::xml_node resoChild : reso.children())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("id").value());
                juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("value").value());
                params[i][j].push_back(std::stod(resoChild.attribute("value").value()));
                
            }
            
            ++j;
        }
        
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        for (pugi::xml_node conn : inst.children("Connection"))
        {
            connects[i].push_back(std::vector<double>());
            connType.push_back(std::vector<String>());
            ++connNum[i];
            juce::Logger::getCurrentLogger()->outputDebugString("Connection:");
            for (pugi::xml_attribute connAttr : conn.attributes())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(connAttr.name());
                juce::Logger::getCurrentLogger()->outputDebugString(connAttr.value());
                auto attrib = String(connAttr.name());
                if (attrib == "type") {
                    connType[i].push_back(connAttr.value());
                }
            }
            juce::Logger::getCurrentLogger()->outputDebugString("Connection locations:");

            for (pugi::xml_node connChild : conn.children())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(connChild.attribute("id").value());
                juce::Logger::getCurrentLogger()->outputDebugString(connChild.attribute("value").value());
                connects[i][c].push_back(std::stod(connChild.attribute("value").value()));

            }

            ++c;
        }
        ++i;
        // --------------------------------------------------------------------------------------------------

        
    }
    if (resoNum.size() != connNum.size())
    {
        DBG ("Silvin: If this happens, I apparently didn't understand it correctly :)");
    }
    for (int i = 0; i < resoNum.size(); ++i)
    {
        initActions.push_back(addInstrumentAction);
        for (int j = 0; j < resoNum[i]; ++j)
        {
            initActions.push_back (addResonatorModuleAction);
        }
        for (int j = 0; j < connNum[i]; ++j)
        {
            initActions.push_back (addConnectionAction);
        }
    }
    
    for (int i = 0; i < resoType.size(); ++i) {
        if (resoType[i] == "Stiff_String") { initModuleTypes.push_back(stiffString); }
        else if (resoType[i] == "Bar") { initModuleTypes.push_back(bar); }
        else if (resoType[i] == "Thin_Plate") { initModuleTypes.push_back(thinPlate); }
        else if (resoType[i] == "Membrane") { initModuleTypes.push_back(membrane); }
        else if (resoType[i] == "Stiff_Membrane") { initModuleTypes.push_back(stiffMembrane); }
    }
    
    int numModules = 0;
    for (int i = 0; i < initActions.size(); ++i)
    {
        if (initActions[i] == addResonatorModuleAction)
            ++numModules;
        
    }
    
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
    
    // indices  for loop below
    int m = 0;  // module type
    int r;      // resonator
    int c;      // connection
    
    i = -1; // reinitialise instrument index

    NamedValueSet parameters;
    for (int a = 0; a < initActions.size(); ++a)
    {
        switch (initActions[a]) {
            case addInstrumentAction:
            {
                addInstrument();
                ++i;
                r = 0;
                c = 0;
                break;
            }
            case addResonatorModuleAction:
            {
                switch (initModuleTypes[m]) {
                    case stiffString:
                        juce::Logger::getCurrentLogger()->outputDebugString(std::to_string(params[i][r][0]));
                        parameters = {
                            {"L", params[i][r][0]},
                            {"T", params[i][r][1]},
                            {"rho", params[i][r][2]},
                            {"A", params[i][r][3]},
                            {"E", params[i][r][4]},
                            {"I", params[i][r][5]},
                            {"sig0", params[i][r][6]},
                            {"sig1", params[i][r][7]} };
                        break;
                    case bar:
                        parameters = {
                            {"L", params[i][r][0]},
                            {"rho", params[i][r][1]},
                            {"A", params[i][r][2]},
                            {"E", params[i][r][3]},
                            {"I", params[i][r][4]},
                            {"sig0", params[i][r][5]},
                            {"sig1", params[i][r][6]} };
                        break;
                    case membrane:
                        parameters = {
                            {"Lx", params[i][r][0]},
                            {"Ly", params[i][r][1]},
                            {"rho", params[i][r][2]},
                            {"H", params[i][r][3]},
                            {"T", params[i][r][4]},
                            {"sig0", params[i][r][5]},
                            {"sig1", params[i][r][6]},
                            {"maxPoints", params[i][r][7]} };
                        break;
                    case thinPlate:
                        parameters = {
                            {"Lx", params[i][r][0]},
                            {"Ly", params[i][r][1]},
                            {"rho", params[i][r][2]},
                            {"H", params[i][r][3]},
                            {"E", params[i][r][4]},
                            {"nu", params[i][r][5]},
                            {"sig0", params[i][r][6]},
                            {"sig1", params[i][r][7]},
                            {"maxPoints", params[i][r][8]} };
                        break;
                    case stiffMembrane:
                        
                        parameters = {
                            {"Lx", params[i][r][0]},
                            {"Ly", params[i][r][1]},
                            {"rho", params[i][r][2]},
                            {"H", params[i][r][3]},
                            {"T", params[i][r][4]},
                            {"E", params[i][r][5]},
                            {"nu", params[i][r][6]},
                            {"sig0", params[i][r][7]},
                            {"sig1", params[i][r][8]},
                            {"maxPoints", params[i][r][9]} };
                        break;
                    default:
                        break;
                }
                addResonatorModule (initModuleTypes[m], parameters);
                ++m;
                ++r;
                break;
            }
            case addConnectionAction:
            {
                int resFromIdx = connects[i][c][0];
                int resFromLoc = connects[i][c][1];
                int resToIdx = connects[i][c][2];
                int resToLoc = connects[i][c][3];
                ConnectionType curConnType;
                
                if (connType[i][c] == "rigid")
                    curConnType = rigid;
                else if (connType[i][c] == "linear")
                    curConnType = linearSpring;
                else if (connType[i][c] == "nonlinear")
                    curConnType = nonlinearSpring;
                        
                instruments[i]->addFirstConnection (instruments[i]->getResonatorPtr(resFromIdx),
                                                    curConnType,
                                                    resFromLoc);
                instruments[i]->addSecondConnection (instruments[i]->getResonatorPtr(resToIdx),
                                                     resToLoc);
                instruments[i]->setCurrentlyActiveConnection (nullptr);
                ++c;
                break;
            }
            default:
                break;
        }
    }
    return success;
}
