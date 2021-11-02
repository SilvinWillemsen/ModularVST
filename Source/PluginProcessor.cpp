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
//#ifdef NO_EDITOR
    addParameter (mouseX = new MyAudioParameterFloat (this, "mouseX", "Mouse X", 0, 0.99, 0.5) );
    addParameter (mouseY = new MyAudioParameterFloat (this, "mouseY", "Mouse Y", 0, 0.99, 0.5) );
    addParameter (excite = new MyAudioParameterFloat (this, "excite", "Excite", 0, 1, 1, 0) );
    addParameter (excitationType = new MyAudioParameterFloat (this, "excitationType", "Excitation Type", 0, 2, 1, 0) );
//#endif
//#ifdef EDITOR_AND_SLIDERS
    allParameters.reserve(8);
    allParameters.push_back (mouseX);
    allParameters.push_back (mouseY);
    allParameters.push_back (excite);
    allParameters.push_back (excitationType);
//#endif
    sliderValues.resize (allParameters.size());
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
        File lastSavedPresetFile (File::getCurrentWorkingDirectory().getChildFile(presetPath + "lastPreset.txt"));
        if (!lastSavedPresetFile.exists())
        {
            DBG("There is no last saved preset!");
            return;
        }
//        FileInputStream lastSavedPresetFileReader (lastSavedPresetFile)
        String fileName = lastSavedPresetFile.loadFileAsString();
        PresetResult res = loadPreset (fileName);
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
        if (applicationState == normalState)
            setToZero = false;
        return;
    }
    
    for (auto inst : instruments)
    {
        // check whether the instrument is ready
        if (!inst->areModulesReady() || applicationState == removeResonatorModuleState)
            return;
        
        inst->checkIfShouldExciteRaisedCos();
        
//        if (inst->checkIfShouldRemoveResonatorModule())
//        {
//            inst->removeResonatorModule();
//            refreshEditor = true;
//        }
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            inst->calculate();
            inst->solveInteractions();
            inst->excite();
#ifdef CALC_ENERGY
            inst->calcTotalEnergy();
//#ifdef CALC_ENERGY
            std::cout << "Energy change: " << inst->getTotalEnergy() << std::endl;
//#endif

#endif
#ifdef SAVE_OUTPUT
            inst->saveOutput();
            if (inst == instruments[0])
                ++counter;
//            if (counter > Global::samplesToRecord + buffer.getNumSamples())
//            {
//                exit(0);
//            }
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
#ifdef NO_EDITOR
    return false; // (change this to false if you choose to not supply an editor)
#else
    return true;
#endif
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
    std::shared_ptr<Instrument> newInstrument = std::make_shared<Instrument> (fs);
    instruments.push_back (newInstrument);
    newInstrument->setExcitationType (curExcitationType);
    currentlyActiveInstrument = newInstrument;
    
    refreshEditor = true;
}

void ModularVSTAudioProcessor::addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, InOutInfo inOutInfo, bool advanced)
{
    jassert(currentlyActiveInstrument != nullptr);

    currentlyActiveInstrument->addResonatorModule (rmt, parameters, inOutInfo, advanced);
    refreshEditor = true;
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
            highlightInstrument (currentlyActiveInstrument);
            break;
        }
    }
    for (auto inst : instruments)
        inst->setApplicationState (a);
    
}

void ModularVSTAudioProcessor::highlightInstrument (std::shared_ptr<Instrument> instrumentToHighlight)
{
    for (auto inst : instruments)
        inst->setHighlightedInstrument (false);
    instrumentToHighlight->setHighlightedInstrument (true);

}

PresetResult ModularVSTAudioProcessor::savePreset (String& fileName)
{
    std::ofstream file;
    const char* pathToUse = String (presetPath + fileName + ".xml").getCharPointer();
    file.open (pathToUse);
    file << "<App" << ">" << "\n";
    for (int i = 0; i < instruments.size(); ++i)
    {
        int numResonators = instruments[i]->getNumResonatorModules();
        file << "\t " << "<Instrument id=\"i" << i << "\">"<< "\n";

        for (int r = 0; r < numResonators; ++r)
        {
            ResonatorModule* curResonator = instruments[i]->getResonatorPtr(r).get();
            // type

            file << "\t " << "\t " << "<Resonator id=\"i" << i <<"_r"<< r << "\" type=\"";
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

            for (int p = 0; p < curResonator->getParameters().size(); ++p)
            {
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_r" << r << "_";
                String paramName = curResonator->getParameters().getName(p).toString();
                double value = *curResonator->getParameters().getVarPointer (paramName);
                file << paramName << "\" value=\"" << value << "\"/>\n";
            }
            for (int o = 0; o < curResonator->getInOutInfo()->getNumOutputs(); ++o)
            {
                file << "\t " << "\t " << "\t " << "<Output id=\"i" << i << "_r" << r << "_o" << o;
                int channel = curResonator->getInOutInfo()->getOutChannelAt (o);
                int loc = curResonator->getInOutInfo()->getOutLocAt (o);
                file << "\" channel=\"" << channel << "\" loc=\"" << loc << "\"/>\n";
            }
            
            // inputs

            file << "\t " << "\t " << "</Resonator>" << "\n";

        }

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        int numConnections = (int)instruments[i]->getConnectionInfo()->size();

        for (int c = 0; c < numConnections; ++c)
        {
            String connectionTypeString;
            // type

            file << "\t " << "\t " << "<Connection id=\"i" << i << "_c" << c << "\" type=\"";
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

                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc1 << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc2 << "\"/>\n";

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
    
    // save path of last saved preset
    std::ofstream lastSavedPreset;
    lastSavedPreset.open (String (presetPath + "lastPreset.txt").getCharPointer());
    lastSavedPreset << String (fileName + ".xml");
    lastSavedPreset.close();
    
    return success;
}

PresetResult ModularVSTAudioProcessor::loadPreset (String& fileName)
{
    // make sure that application is loaded from scratch
    if (instruments.size() != 0)
    {
        for (auto inst : instruments)
        {
            inst->removeAllResonators();
        }
        while (instruments.size() > 0)
            instruments.erase(instruments.begin() + instruments.size() - 1);
        
//            return applicationIsNotEmpty;
    }
    
    pugi::xml_document doc;
    std::string test = String(presetPath + fileName).toStdString();// .getCharPointer()
    const char* pathToUse = test.c_str();
    pugi::xml_parse_result result = doc.load_file (pathToUse);
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
    std::vector< std::vector<std::vector<double>>> outputLocs;
    std::vector< std::vector<std::vector<double>>> outputChannels;

    std::vector<String> resoType;
    std::vector<std::vector<String>> connType;
//    std::vector<int> resoNum;
//    std::vector<int> connNum;
    
    initActions.clear();
    initModuleTypes.clear();
    
    InOutInfo IOinfo (false); // for presets we do not want to do a default initialisation of the in- and outputs
    
    int i = 0;
    
    for (pugi::xml_node inst : node.children("Instrument"))
    {
        initActions.push_back(addInstrumentAction);
        
//        resoNum.push_back(0);
//        connNum.push_back(0);
        params.push_back(std::vector< std::vector<double>>());
        connects.push_back(std::vector< std::vector<double>>());
        outputLocs.push_back(std::vector< std::vector<double>>());
        outputChannels.push_back(std::vector< std::vector<double>>());

        int r = 0;
        int c = 0;
        for (pugi::xml_node reso : inst.children("Resonator"))
        {
            initActions.push_back (addResonatorModuleAction);

            params[i].push_back(std::vector<double>());
            outputLocs[i].push_back(std::vector<double>());
            outputChannels[i].push_back(std::vector<double>());
//            ++resoNum[i];
            juce::Logger::getCurrentLogger()->outputDebugString("Resonator:");
            for (pugi::xml_attribute resoAttr : reso.attributes())
            {
                juce::Logger::getCurrentLogger()->outputDebugString(resoAttr.name());
                juce::Logger::getCurrentLogger()->outputDebugString(resoAttr.value());
                auto attrib = String (resoAttr.name());
                if (attrib == "type") {
                    resoType.push_back(resoAttr.value());
                }
            }
            juce::Logger::getCurrentLogger()->outputDebugString("Parameters:");
            
            for (pugi::xml_node resoChild : reso.children())
            {
                if (String (resoChild.name()) == "PARAM")
                {
                    juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("id").value());
                    juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("value").value());
                    params[i][r].push_back(std::stod(resoChild.attribute("value").value()));
                }
                else if (String (resoChild.name()) == "Output")
                {
                    juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("id").value());
                    juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("loc").value());
                    juce::Logger::getCurrentLogger()->outputDebugString(resoChild.attribute("channel").value());
                    outputLocs[i][r].push_back(std::stod(resoChild.attribute("loc").value()));
                    outputChannels[i][r].push_back(std::stod(resoChild.attribute("channel").value()));
                    
                    initActions.push_back (addOutputAction);
                    
                }
            }
            ++r;
        }
        
        // Do connections after resonators
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        for (pugi::xml_node conn : inst.children("Connection"))
        {
            initActions.push_back (addConnectionAction);
            
            connects[i].push_back(std::vector<double>());
            connType.push_back(std::vector<String>());
//            ++connNum[i];
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
//    if (resoNum.size() != connNum.size())
//    {
//        DBG ("Silvin: If this happens, I apparently didn't understand it correctly :)");
//    }
//    for (int i = 0; i < resoNum.size(); ++i)
//    {
//    }
    
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
    int o;      // output
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
                o = 0;
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
                addResonatorModule (initModuleTypes[m], parameters, IOinfo);
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
                        
                instruments[i]->addFirstConnection (instruments[i]->getResonatorPtr (resFromIdx),
                                                    curConnType,
                                                    resFromLoc);
                instruments[i]->addSecondConnection (instruments[i]->getResonatorPtr (resToIdx),
                                                     resToLoc);
                instruments[i]->setCurrentlyActiveConnection (nullptr);
                instruments[i]->setAction (noAction);
                ++c;
                break;
            }
            case addOutputAction:
            {
                // as the resonator idx is incremented before the outputs are added, subtract 1 from this index
                instruments[i]->getResonatorPtr(r-1)->getInOutInfo()->addOutput (outputLocs[i][r-1][o], outputChannels[i][r-1][o]);
                ++o;
                break;
            }
            default:
                break;
        }
    }
    
    // save path of last loaded preset
    std::ofstream lastLoadedPreset;
    lastLoadedPreset.open (String (presetPath + "lastPreset.txt").getCharPointer());
    lastLoadedPreset << String (fileName);
    lastLoadedPreset.close();
    
    return success;
}


//# ifdef NO_EDITOR

void ModularVSTAudioProcessor::genericAudioParameterFloatValueChanged (String name, float value)
{
    if ((name == "mouseX" || name == "mouseY") && (sliderValues[exciteID] >= 0.5))
        currentlyActiveInstrument->virtualMouseMove (sliderValues[mouseXID], sliderValues[mouseYID]);

    if (name == "excite" || name == "excitationType")
    {
        if (sliderValues[exciteID] >= 0.5)
        {
            switch (static_cast<int> (sliderValues[excitationTypeID]))
            {
                case 0:
                    curExcitationType = pluck;
                    break;
                case 1:
                    curExcitationType = hammer;
                    break;
                case 2:
                    curExcitationType = bow;
                    break;
            }
            currentlyActiveInstrument->setExcitationType (curExcitationType);
            currentlyActiveInstrument->resetPrevMouseMoveResonator();
            currentlyActiveInstrument->virtualMouseMove (sliderValues[mouseXID], sliderValues[mouseYID]);
        } else {
            curExcitationType = noExcitation;
            currentlyActiveInstrument->setExcitationType (curExcitationType);
        }
        currentlyActiveInstrument->virtualMouseMove (sliderValues[mouseXID], sliderValues[mouseYID]);

    }
}
void ModularVSTAudioProcessor::myAudioParameterFloatValueChanged (MyAudioParameterFloat* myAudioParameter)
{
    for (int i = 0; i < allParameters.size(); ++i)
        if (myAudioParameter == allParameters[i])
            sliderValues[i] = *myAudioParameter;

    genericAudioParameterFloatValueChanged (myAudioParameter->paramID, *myAudioParameter);
}

#ifdef EDITOR_AND_SLIDERS
void ModularVSTAudioProcessor::myAudioParameterFloatValueChanged (Slider* mySlider)
{
    for (int i = 0; i < editorSliders->size(); ++i)
        if (mySlider == (*editorSliders)[i].get())
            sliderValues[i] = mySlider->getValue();
    
    genericAudioParameterFloatValueChanged (mySlider->getName(), mySlider->getValue());
}
#endif

ModularVSTAudioProcessor::MyAudioParameterFloat::MyAudioParameterFloat (
                                            ModularVSTAudioProcessor* audioProcessor,
                                            String parameterID,
                                            String parameterName,
                                            float minValue,
                                            float maxValue,
                                            float defaultValue) : AudioParameterFloat (parameterID,
                                                                                       parameterName,
                                                                                       minValue,
                                                                                       maxValue,
                                                                                       defaultValue),
                                                                  audioProcessor (audioProcessor)
{
    
}

ModularVSTAudioProcessor::MyAudioParameterFloat::MyAudioParameterFloat (
                                            ModularVSTAudioProcessor* audioProcessor,
                                            String parameterID,
                                            String parameterName,
                                            float minValue,
                                            float maxValue,
                                            float stepSize,
                                            float defaultValue) : AudioParameterFloat (parameterID,
                                                                                       parameterName,
                                                                                       {minValue, maxValue, stepSize},
                                                                                       defaultValue),
                                                                  audioProcessor (audioProcessor)
{
    
}

//#endif
