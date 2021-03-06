/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


// extern "c" function definitions

const char* getPresetAt (int i)
{
    return Global::presetFilesToIncludeInUnity[i].getCharPointer();
}

const char* getXMLOfPresetAt (int i)
{
    int size;
    return BinaryData::getNamedResource(Global::presetFilesToIncludeInUnity[i].getCharPointer(), size);
}

int getNumPresets()
{
    return Global::presetFilesToIncludeInUnity.size();
//    return BinaryData::namedResourceListSize;
}


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
    addParameter (mouseX1 = new AudioParameterFloat (customParameterID ("mouseX1"), "Mouse X1", 0, 0.99, 0.5));
    addParameter (mouseY1 = new AudioParameterFloat (customParameterID ("mouseY1"), "Mouse Y1", 0, 0.99, 0.5) );
    addParameter (mouseX2 = new AudioParameterFloat (customParameterID ("mouseX2"), "Mouse X2", 0, 0.99, 0.5) );
    addParameter (mouseY2 = new AudioParameterFloat (customParameterID ("mouseY2"), "Mouse Y2", 0, 0.99, 0.5) );
    addParameter (smooth = new AudioParameterBool (customParameterID ("smooth"), "Smooth", 1));
    addParameter (smoothness = new AudioParameterFloat (customParameterID ("smoothness"), "Smoothness", 0, 99, 95));
    addParameter (excite = new AudioParameterBool (customParameterID ("excite"), "Excite", 0));
    addParameter (excitationType = new AudioParameterFloat (customParameterID ("excitationType"), "Excitation Type", 0, 0.99, 0.5));
    addParameter (useVelocity = new AudioParameterBool (customParameterID ("useVelocity"), "Use Velocity", 1));
    addParameter (velocity = new AudioParameterFloat (customParameterID ("velocity"), "Velocity", 0, 1, 0.5));
    addParameter (trigger1 = new AudioParameterBool (customParameterID ("trigger1"), "Trigger1", 0));
    addParameter (trigger2 = new AudioParameterBool (customParameterID ("trigger2"), "Trigger2", 0));
    addParameter (activateSecondExciter = new AudioParameterBool (customParameterID ("activateSecondExciter"), "ActivateSecondExciter", 0));
    addParameter (presetSelect = new AudioParameterFloat (customParameterID ("presetSelect"), "Preset Select", 0, 0.99, 0.01));
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
    addParameter (loadPresetToggle = new AudioParameterBool (customParameterID ("loadPresetToggle"), "Load preset", 1));
#endif
    
    //#endif
//#ifdef EDITOR_AND_SLIDERS
    allParameters.reserve (8);
    allParameters.push_back (mouseX1);
    allParameters.push_back (mouseY1);
    allParameters.push_back (mouseX2);
    allParameters.push_back (mouseY2);
    allParameters.push_back (smooth);
    allParameters.push_back (smoothness);
    allParameters.push_back (excite);
    allParameters.push_back (excitationType);
    allParameters.push_back (useVelocity);
    allParameters.push_back (velocity);
    allParameters.push_back (trigger1);
    allParameters.push_back (trigger2);
    allParameters.push_back (activateSecondExciter);
    allParameters.push_back (presetSelect);
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
    allParameters.push_back(loadPresetToggle);
#endif
//#endif
    sliderValues.resize (allParameters.size());
        
    
    mouseSmoothValues1 = { *mouseX1, *mouseY1 };
    mouseSmoothValues2 = { *mouseX2, *mouseY2 };
    velocitySmoothValue = *velocity;

    addChangeListener (this);
    
    numOfBinaryPresets = BinaryData::namedResourceListSize;
    
#if defined(NO_EDITOR) || defined(EDITOR_AND_SLIDERS)
    sliderControl = true;
#else
    for (int i = 0; i < sliderValues.size(); ++i)
    {
        sliderValues[i] = allParameters[i]->getValue();
    }
#endif
    prevSliderValues = sliderValues;

//    std::cout << "Constructor processor" << std::endl;
//    Debug::Log ("Debugger (constructor processor)", Color::Orange);
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

#ifdef LOAD_ALL_UNITY_INSTRUMENTS
    totPreset = "<App>\n \t <Instrument id=\"i0\">\n";
//    std::cout << totPreset << std::endl;
    for (int i = 0; i < Global::presetFilesToIncludeInUnity.size(); ++i)
    {
        int sizeDummy = 0;
        const char* name =  Global::presetFilesToIncludeInUnity[i].getCharPointer();
        std::string st = BinaryData::getNamedResource(name, sizeDummy);
        
        // cut off app and instrument
        size_t pos = st.find("\t \t <Resonator");
        st.erase(0,pos);
        pos = st.find("</App>");
//        size_t stringSize = st.size();
        st.erase(pos, st.size());
        
        totPreset += st;
        if (i == Global::presetFilesToIncludeInUnity.size() - 1)
            totPreset += "</App>";
        else
        {
            totPreset += "\t <Instrument id=\"i" + String(i+1).toStdString() + "\">\n";
        }
    }
    
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string (String(totPreset).getCharPointer());
    loadPresetFromPugiDoc (&doc);
    
    switch (result.status)
    {
        case pugi::status_ok:
            debugLoadPresetResult (success);
            break;
        case pugi::status_file_not_found:
            debugLoadPresetResult (fileNotFound);
            break;
        default:
            debugLoadPresetResult (presetNotLoaded);
            break;
    }
    
#else
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

        PresetResult res = loadPreset (fileName, fileName.contains("_"));
        debugLoadPresetResult (res);
    }
#endif
        
    //---// For unity, temporary solution until we get the presets to work //---//
//    addInstrument();
//    addResonatorModule (stiffString, Global::defaultStringParametersAdvanced, InOutInfo());
    //---//
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
#ifndef EDITOR_AND_SLIDERS
    for (int i = 0; i < sliderValues.size(); ++i)
    {
        sliderValues[i] = allParameters[i]->convertFrom0to1 (allParameters[i]->getValue());
    }
    for (int i = 0; i < sliderValues.size(); ++i)
    {
        if (sliderValues[i] != prevSliderValues[i])
            myRangedAudioParameterChanged (allParameters[i]);
    }

    prevSliderValues = sliderValues;
#endif

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
        if (inst != currentlyActiveInstrument)
            continue;
        
        audioMutex.lock();

        // check whether the instrument is ready
        if (!inst->areModulesReady() || applicationState == removeResonatorModuleState)
        {
            audioMutex.unlock();
            continue;
        }

//        DBG("Lock mutex");

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

            totOutputL[i] += inst->getOutputL();
            totOutputR[i] += inst->getOutputR();
            
            // Update the states
            inst->update();

            // virtual mouse move at audio rate (smoothing)
            if (sliderValues[smoothID] == 1 && sliderControl)
            {

                mouseSmoothValues1[0] = (0.99 + 0.0001 * sliderValues[smoothnessID]) * mouseSmoothValues1[0] + (1.0 - (0.99 + 0.0001 * sliderValues[smoothnessID])) * sliderValues[mouseX1ID];
                mouseSmoothValues2[0] = (0.99 + 0.0001 * sliderValues[smoothnessID]) * mouseSmoothValues2[0] + (1.0 - (0.99 + 0.0001 * sliderValues[smoothnessID])) * sliderValues[mouseX2ID];
//#ifndef LOAD_ALL_UNITY_INSTRUMENTS
//                // If velocity is used, locate the mouse at a ylocation dependent on the velocity
//                double yVal = (sliderValues[useVelocityID] && curExcitationType == hammer) ? (floor(sliderValues[mouseYID] * currentlyActiveInstrument->getNumResonatorModules()) + 0.5 - 0.5 * sliderValues[velocityID]) / currentlyActiveInstrument->getNumResonatorModules() : sliderValues[1];
//#else
                double yVal1 = 0;
                double yVal2 = 0;
                if (currentlyActiveInstrument != nullptr)
                {
                    if (currentlyActiveInstrument->getCurrentlyHoveredResonators()[0] != nullptr &&
                       currentlyActiveInstrument->getCurrentlyHoveredResonators()[0]->isModule1D())
                    {
                        // If velocity is used for a 1D object, locate the mouse at a ylocation dependent on the velocity
                        yVal1 = (sliderValues[useVelocityID] && curExcitationType == hammer) ? (floor(sliderValues[mouseY1ID] * currentlyActiveInstrument->getNumResonatorModules()) + 0.5 - 0.5 * sliderValues[velocityID]) / currentlyActiveInstrument->getNumResonatorModules() : sliderValues[mouseY1ID];
                    } else {
                        yVal1 = sliderValues[mouseY1ID];
                    }
                    if (currentlyActiveInstrument->getCurrentlyHoveredResonators()[1] != nullptr &&
                       currentlyActiveInstrument->getCurrentlyHoveredResonators()[1]->isModule1D())
                    {
                        yVal2 = (sliderValues[useVelocityID] && curExcitationType == hammer) ? (floor(sliderValues[mouseY2ID] * currentlyActiveInstrument->getNumResonatorModules()) + 0.5 - 0.5 * sliderValues[velocityID]) / currentlyActiveInstrument->getNumResonatorModules() : sliderValues[mouseY2ID];

                    } else {
                        yVal2 = sliderValues[mouseY2ID];
                    }
                }

                
//#endif
                mouseSmoothValues1[1] = (0.99 + 0.0001 * sliderValues[smoothnessID]) * mouseSmoothValues1[1] + (1.0 - (0.99 + 0.0001 * sliderValues[smoothnessID])) * yVal1;
                mouseSmoothValues2[1] = (0.99 + 0.0001 * sliderValues[smoothnessID]) * mouseSmoothValues2[1] + (1.0 - (0.99 + 0.0001 * sliderValues[smoothnessID])) * yVal2;
                inst->virtualMouseMove1 (mouseSmoothValues1[0], mouseSmoothValues1[1]);
                if (sliderValues[activateSecondExciterID] >= 0.5f)
                    inst->virtualMouseMove2 (mouseSmoothValues2[0], mouseSmoothValues2[1]);

                velocitySmoothValue = (0.99 + 0.0001 * sliderValues[smoothnessID]) * velocitySmoothValue + (1.0 - (0.99 + 0.0001 * sliderValues[smoothnessID])) * (sliderValues[velocityID] * 0.4 - 0.2);
                if (sliderValues[excitationTypeID] >= 0.67f)
                    inst->setBowParams (velocitySmoothValue);
            } else
            {
                mouseSmoothValues1[0] = sliderValues[mouseX1ID];
                mouseSmoothValues1[1] = sliderValues[mouseY1ID];
                mouseSmoothValues2[0] = sliderValues[mouseX2ID];
                mouseSmoothValues2[1] = sliderValues[mouseY2ID];
            }
        }
    
        audioMutex.unlock();
//        DBG("Unlock mutex" + String(counter));

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
    if (shouldLoadPreset)
    {
        for (auto inst : instruments)
            inst->unReadyAllModules();
        sendChangeMessage();
        shouldLoadPreset = false;
        refreshEditor = true;
    }
//    std::cout << totOutput[15] << std::endl;
//    Debug::Log ("Hellow Orange", Color::Orange); // unity debug

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
    newInstrument->setName ("Instrument " + String(instruments.size()));
    newInstrument->setExcitationType (curExcitationType);
    currentlyActiveInstrument = newInstrument;
    
    instruments.push_back (newInstrument);

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
        case removeResonatorModuleState:
        case editConnectionState:
            if (currentlyActiveInstrument != nullptr)
                currentlyActiveInstrument->setCurrentlySelectedResonatorToNullptr();
        default:
            setStatesToZero (true);
            highlightInstrument (currentlyActiveInstrument);
            break;

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

PresetResult ModularVSTAudioProcessor::savePreset (String& filePath)
{
    std::ofstream file;
    const char* pathToUse = String (filePath).getCharPointer();
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
                // make "if" for string, bar and membrane i suppose, cuz it all depends on h: locRatio = locpoint/N;  N = floor (L/h) 
                if (curResonator->isModule1D())
                {
                    int N = curResonator-> getNumIntervals();
                    double locRatio = double(curResonator->getInOutInfo()->getOutLocAt(o)) / (N + 1);
                    file << "\" channel=\"" << channel << "\" loc=\"" << locRatio << "\"/>\n";
                }
                else
                {
                    int Nx = curResonator->getNumIntervalsX();
                    int Ny = curResonator->getNumIntervalsY();
                    double locRatioX = double(curResonator->getInOutInfo()->getOutLocAt(o) % Nx) / (Nx + 1);
                    double locRatioY = double(curResonator->getInOutInfo()->getOutLocAt(o) / Nx) / (Ny + 1);
                    file << "\" channel=\"" << channel << "\" locX=\"" << locRatioX << "\" locY=\"" << locRatioY << "\"/>\n";
                }
                //double loc = (curResonator->getInOutInfo()->getOutLocAt (o));
                //file << "\" channel=\"" << channel << "\" loc=\"" << loc << "\"/>\n";
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

                /*file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc1 << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\"/>\n";
                file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tL\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].loc2 << "\"/>\n";*/

                //=============================== 1 D res1 ==========================================================================
                if (instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res1->getID())->isModule1D())
                {
                    int N = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res1->getID())->getNumIntervals();
                    double locRatio = double(instruments[i]->getConnectionInfo()[0][c].loc1) / (N + 1);
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\"/>\n";
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fL\" " << "value=\"" << locRatio << "\"/>\n";
                }
                //================================= 2 D res1 =======================================================================================
                else
                {
                    int Nx = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res1->getID())->getNumIntervalsX();
                    int Ny = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res1->getID())->getNumIntervalsY();
                    double locRatioX = double(instruments[i]->getConnectionInfo()[0][c].loc1 % Nx) / (Nx + 1);
                    double locRatioY = double(instruments[i]->getConnectionInfo()[0][c].loc1 / Nx) / (Ny + 1);
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res1->getID() << "\"/>\n";
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fL\" " << "valueX=\"" << locRatioX << " \" valueY=\"" << locRatioY << "\"/>\n";
                }
                //================================= 1 D res2 =======================================================================================
                if (instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res2->getID())->isModule1D())
                {
                    int N = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res2->getID())->getNumIntervals();
                    double locRatio = double(instruments[i]->getConnectionInfo()[0][c].loc2) / (N + 1);
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\"/>\n";
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_fL\" " << "value=\"" << locRatio << "\"/>\n";
                }
                //================================= 2 D res2 =======================================================================================
                else
                {
                    int Nx = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res2->getID())->getNumIntervalsX();
                    int Ny = instruments[i]->getResonatorPtr(instruments[i]->getConnectionInfo()[0][c].res2->getID())->getNumIntervalsY();
                    double locRatioX = double(instruments[i]->getConnectionInfo()[0][c].loc2 % Nx) / (Nx + 1);
                    double locRatioY = double(instruments[i]->getConnectionInfo()[0][c].loc2 / Nx) / (Ny + 1);
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tR\" " << "value=\"" << instruments[i]->getConnectionInfo()[0][c].res2->getID() << "\"/>\n";
                    file << "\t " << "\t " << "\t " << "<PARAM id=\"i" << i << "_c" << c << "_tL\" " << "valueX=\"" << locRatioX << " \" valueY=\"" << locRatioY << "\"/>\n";
                }

            file << "\t " << "\t " << "</Connection>" << "\n";


        // ----------------------------------------------------------------------------------
       

        }
        
        for (int g = 0; g < instruments[i]->getNumResonatorGroups(); ++g)
        {
            file << "\t " << "\t " << "<ResonatorGroup id=\"i" << i << "_g" << g << "\">\n";
            for (auto res : instruments[i]->getResonatorGroups()[g]->getResonatorsInGroup())
                file << "\t " << "\t " << "\t " << "<Res id=\"" << res->getID() << "\"/>\n";
            file << "\t " << "\t " << "</ResonatorGroup>\n";
        }
        
        file << "\t " << "</Instrument>" << "\n";
    }
    file << "</App" << ">" << "\n";
    file.close();
    
    return success;
}

PresetResult ModularVSTAudioProcessor::loadPreset (String& fileName, bool loadFromBinary)
{
    
    pugi::xml_document doc;
    std::string test = fileName.toStdString();// .getCharPointer()
    
    const char* pathToUse = test.c_str();
    int sizeTest = 0;
    const char* fileCharIfNeeded = fileName.getCharPointer();
    pugi::xml_parse_result result = loadFromBinary ? doc.load_string(BinaryData::getNamedResource (fileCharIfNeeded, sizeTest)) : doc.load_file (pathToUse);

    switch (result.status)
    {
        case pugi::status_ok:
            break;
        case pugi::status_file_not_found:
            return fileNotFound;
            break;
        default:
            return presetNotLoaded;
            break;
    }
    
    loadPresetFromPugiDoc (&doc);

    return success;
}

void ModularVSTAudioProcessor::loadPresetFromPugiDoc (pugi::xml_document* doc)
{
    loadPresetMutex.lock();
    // make sure that application is loaded from scratch
    if (instruments.size() != 0)
    {
        for (auto inst : instruments)
        {
            inst->removeAllResonators();
        }
        while (instruments.size() > 0)
            instruments.erase(instruments.begin() + instruments.size() - 1);
    }
    
    pugi::xml_node node = doc->child("App");
    pugi::xml_node instrum = node.child("Instrument");
    
    DBG("hello");
    DBG(instrum.child("Resonator").child("PARAM").attribute("id").value());
    DBG(doc->child("App").child("Instrument").child("Resonator").child("PARAM").attribute("value").value());
    DBG(instrum.child("Connection").attribute("type").value());
    
    std::vector<std::vector<std::vector<double>>> params;
    std::vector<std::vector<std::vector<std::vector<double>>>> connects;
    std::vector<std::vector<std::vector<std::vector<double>>>> outputLocs;
    std::vector<std::vector<std::vector<double>>> outputChannels;
    std::vector<std::vector<std::vector<int>>> resonatorInGroupIds;

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
        params.push_back(std::vector<std::vector<double>>());
        connects.push_back(std::vector<std::vector<std::vector<double>>>());
        outputLocs.push_back(std::vector<std::vector<std::vector<double>>>());
        outputChannels.push_back(std::vector<std::vector<double>>());
        
        resonatorInGroupIds.push_back(std::vector<std::vector<int>>());

        int r = 0;
        int c = 0;
        for (pugi::xml_node reso : inst.children("Resonator"))
        {
            initActions.push_back (addResonatorModuleAction);

            params[i].push_back(std::vector<double>());
            outputLocs[i].push_back(std::vector<std::vector<double>>());
            outputChannels[i].push_back(std::vector<double>());
//            ++resoNum[i];
            DBG("Resonator:");
            for (pugi::xml_attribute resoAttr : reso.attributes())
            {
                DBG(resoAttr.name());
                DBG(resoAttr.value());
                auto attrib = String (resoAttr.name());
                if (attrib == "type") {
                    resoType.push_back(resoAttr.value());
                }
            }
            DBG("Parameters:");
            
            for (pugi::xml_node resoChild : reso.children())
            {
                if (String (resoChild.name()) == "PARAM")
                {
                    DBG(resoChild.attribute("id").value());
                    DBG(resoChild.attribute("value").value());
                    params[i][r].push_back(std::stod(resoChild.attribute("value").value()));
                }
                else if (String (resoChild.name()) == "Output")
                {
                    DBG(resoChild.attribute("id").value());
                    DBG(resoChild.attribute("loc").value());
                    DBG(resoChild.attribute("channel").value());
                    if (resoChild.attribute ("loc"))
                        outputLocs[i][r].push_back (std::vector<double> {std::stod(resoChild.attribute("loc").value())});
                    else
                    {
                        outputLocs[i][r].push_back(std::vector<double>{std::stod(resoChild.attribute("locX").value()),
                            std::stod(resoChild.attribute("locY").value())
                        });
                    }
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
            
            connects[i].push_back(std::vector<std::vector<double>>());
            connType.push_back(std::vector<String>());
//            ++connNum[i];
            DBG("Connection:");
            for (pugi::xml_attribute connAttr : conn.attributes())
            {
                DBG(connAttr.name());
                DBG(connAttr.value());
                auto attrib = String(connAttr.name());
                if (attrib == "type") {
                    connType[i].push_back(connAttr.value());
                }
            }
            DBG("Connection locations:");

            for (pugi::xml_node connChild : conn.children())
            {
                DBG(connChild.attribute("id").value());
                DBG(connChild.attribute("value").value());
                if (connChild.attribute("value"))
                    connects[i][c].push_back(std::vector<double> {std::stod(connChild.attribute("value").value())});
                else
                    connects[i][c].push_back(std::vector<double> {std::stod(connChild.attribute("valueX").value()),
                        std::stod(connChild.attribute("valueY").value())
                    });

            }

            ++c;
        }
        
        int g = 0;
        for (pugi::xml_node group : inst.children("ResonatorGroup"))
        {
            initActions.push_back (addResonatorGroupAction);
            resonatorInGroupIds[i].push_back (std::vector<int>());
            for (pugi::xml_node resInGroup : group.children())
            {
                initActions.push_back (addResonatorToGroupAction);
                resonatorInGroupIds[i][g].push_back (std::stoi(resInGroup.attribute("id").value()));
            }
            ++g;
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
    int g;      // group
    int rIG;    // resonator in group
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
                g = 0;
                break;
            }
            case addResonatorModuleAction:
            {
                o = 0;
                switch (initModuleTypes[m]) {
                    case stiffString:
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
                            {"T", params[i][r][2]},
                            {"rho", params[i][r][3]},
                            {"H", params[i][r][4]},
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
                int resFromIdx = connects[i][c][0][0];
                double resFromLoc1 = connects[i][c][1][0];
                double resFromLoc2 = (connects[i][c][1].size() == 1) ? -1 : connects[i][c][1][1];
                int resToIdx = connects[i][c][2][0];
                double resToLoc1 = connects[i][c][3][0];
                double resToLoc2 = (connects[i][c][3].size() == 1) ? -1 : connects[i][c][3][1];
                ConnectionType curConnType;
                
                if (connType[i][c] == "rigid")
                    curConnType = rigid;
                else if (connType[i][c] == "linear")
                    curConnType = linearSpring;
                else if (connType[i][c] == "nonlinear")
                    curConnType = nonlinearSpring;
                        
                if (resFromLoc2 == -1)
                    instruments[i]->addFirstConnection (instruments[i]->getResonatorPtr (resFromIdx),
                                                        curConnType,
                                                        resFromLoc1);
                else
                    instruments[i]->addFirstConnection (instruments[i]->getResonatorPtr (resFromIdx),
                                                        curConnType,
                                                        resFromLoc1, resFromLoc2);
                if (resToLoc2 == -1)
                    instruments[i]->addSecondConnection (instruments[i]->getResonatorPtr (resToIdx),
                                                         resToLoc1);
                else
                    instruments[i]->addSecondConnection (instruments[i]->getResonatorPtr (resToIdx),
                                                         resToLoc1, resToLoc2);
                instruments[i]->setCurrentlyActiveConnection (nullptr);
                instruments[i]->setAction (noAction);
                ++c;
                break;
            }
            case addOutputAction:
            {
                // as the resonator idx is incremented before the outputs are added, subtract 1 from this index
                if (outputLocs[i][r-1][o].size() == 1)
                    instruments[i]->getResonatorPtr(r-1)->getInOutInfo()->addOutput (outputLocs[i][r-1][o][0], outputChannels[i][r-1][o]);
                else
                    instruments[i]->getResonatorPtr(r-1)->getInOutInfo()->addOutput (outputLocs[i][r-1][o][0], outputLocs[i][r-1][o][1], outputChannels[i][r-1][o]);

                ++o;
                break;
            }
            case addResonatorGroupAction:
                instruments[i]->addResonatorGroup();
                ++g; // addResonatorToGroupAction starts at 1
                rIG = 0;
                break;
            case addResonatorToGroupAction:
                instruments[i]->getCurrentlySelectedResonatorGroup()->addResonator (instruments[i]->getResonatorPtr (resonatorInGroupIds[i][g-1][rIG]), g);
                DBG ("Resonator " + String (resonatorInGroupIds[i][g-1][rIG]) + " is part of group " + String (g));

                ++rIG;
                break;

            default:
                break;
        }
    }
    currentlyActiveInstrument->setCurrentlySelectedResonatorToNullptr();
    loadPresetMutex.unlock();
}

//# ifdef NO_EDITOR

void ModularVSTAudioProcessor::genericAudioParameterValueChanged (String name, float value)
{
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
    if (name == "loadPresetToggle" && sliderValues[loadPresetToggleID] == 1)
    {
        for (int i = 0; i < Global::presetFilesToIncludeInUnity.size(); ++i)
        {
            if (sliderValues[presetSelectID] < static_cast<float>(i+1) / Global::presetFilesToIncludeInUnity.size())
            {
                presetToLoad = Global::presetFilesToIncludeInUnity[i];
                std::cout << presetToLoad << std::endl;
                break;
            }
        }
        setShouldLoadPreset (presetToLoad, true);
    }
#else
    if (name == "presetSelect")
    {
        int idx = floor(sliderValues[presetSelectID] * instruments.size());
        if (instruments[idx] != currentlyActiveInstrument)
            changeActiveInstrument (instruments[idx]);
    }
#endif
    // return if there is no currently active instrument
    if (currentlyActiveInstrument == nullptr)
        return;
    
    if (((name == "mouseX1" || name == "mouseY1"
         || (name == "velocity" && sliderValues[useVelocityID] && curExcitationType == hammer))
        || name == "useVelocity") // refresh
        && sliderValues[exciteID] >= 0.5)
    {
        if (sliderValues[smoothID] != 1)
        {
            double yVal = (sliderValues[useVelocityID] && curExcitationType == hammer) ? (floor(sliderValues[mouseY1ID] * currentlyActiveInstrument->getNumResonatorModules()) + 0.5 - 0.5 * sliderValues[velocityID]) / currentlyActiveInstrument->getNumResonatorModules() : sliderValues[mouseY1ID];
            currentlyActiveInstrument->virtualMouseMove1 (sliderValues[mouseX1ID], yVal);
        }
        // else, the virtualMouseMove goes at audio rate
    }
    
    if (((name == "mouseX2" || name == "mouseY2"
         || (name == "velocity" && sliderValues[useVelocityID] && curExcitationType == hammer))
        || name == "useVelocity") // refresh
        && sliderValues[exciteID] >= 0.5)
    {
        if (sliderValues[smoothID] != 1)
        {
            double yVal = (sliderValues[useVelocityID] && curExcitationType == hammer) ? (floor(sliderValues[mouseY2ID] * currentlyActiveInstrument->getNumResonatorModules()) + 0.5 - 0.5 * sliderValues[velocityID]) / currentlyActiveInstrument->getNumResonatorModules() : sliderValues[mouseY2ID];
            currentlyActiveInstrument->virtualMouseMove2 (sliderValues[mouseX2ID], yVal);
        }
        // else, the virtualMouseMove goes at audio rate
    }
    
    // Set hammer velocity for all 2D objects (as this can not be directly controlled by the mouse x/y positions)
    if (name == "velocity")
        if (curExcitationType == hammer)
        {
            for (auto inst : instruments)
                inst->set2DresHammerVelocity(sliderValues[velocityID] * 0.1 + 0.5);
        }
        else if (curExcitationType == bow && sliderValues[smoothID] != 1)
        {
            for (auto inst : instruments)
                currentlyActiveInstrument->setBowParams (sliderValues[velocityID] * 0.4 - 0.2);
        }
    
    if (name == "trigger1" && sliderValues[trigger1ID] == 1)
    {
        if (sliderValues[exciteID] >= 0.5)
            currentlyActiveInstrument->triggerHammer1();
    }
    if (name == "trigger2" && sliderValues[trigger2ID] == 1)
    {
        if (sliderValues[exciteID] >= 0.5)
            currentlyActiveInstrument->triggerHammer2();
    }

    if (name == "activateSecondExciter")
    {
        if (sliderValues[activateSecondExciterID] < 0.5f)
        {
            currentlyActiveInstrument->exitMouse2();
        }
    }
    
    if (name == "excite" || name == "excitationType")
    {
        if (sliderValues[exciteID] >= 0.5)
        {
            if (sliderValues[excitationTypeID] < 0.33)
                curExcitationType = pluck;
            else if (sliderValues[excitationTypeID] < 0.67)
                curExcitationType = hammer;
            else
                curExcitationType = bow;

            for (auto inst : instruments)
            {
                inst->setExcitationType (curExcitationType);
                inst->resetPrevMouseMoveResonators();
                
                // MIGHT NOT BE NECESSARY ->
//                inst->virtualMouseMove1 (sliderValues[mouseX1ID], sliderValues[mouseY1ID]);
//                inst->virtualMouseMove2 (sliderValues[mouseX2ID], sliderValues[mouseY2ID]);
            }
        } else {
            curExcitationType = noExcitation;
            for (auto inst : instruments)
            {
                inst->setExcitationType (curExcitationType);
            }
        }
        // only move the mouse for the currently active instrument
        currentlyActiveInstrument->virtualMouseMove1 (sliderValues[mouseX1ID], sliderValues[mouseY1ID]);
        if (sliderValues[activateSecondExciterID] >= 0.5f)
            currentlyActiveInstrument->virtualMouseMove2 (sliderValues[mouseX2ID], sliderValues[mouseY2ID]);

    }
}

void ModularVSTAudioProcessor::debugLoadPresetResult (PresetResult res)
{
    switch (res) {
        case applicationIsNotEmpty:
            DBG ("Application is not empty.");
            break;
        case fileNotFound:
            DBG ("Presetfile not found");
            break;
        case presetNotLoaded:
            DBG ("For whatever reason, the preset was not loaded.");
            break;
        case loadingCancelled:
            DBG ("Loading was cancelled.");
            break;
        case success:
            DBG ("Preset loaded successfully.");
            break;

        default:
            break;
    }
}

void ModularVSTAudioProcessor::myRangedAudioParameterChanged (RangedAudioParameter* myAudioParameter)
{
    for (int i = 0; i < allParameters.size(); ++i)
        if (myAudioParameter == allParameters[i])
            sliderValues[i] = myAudioParameter->convertFrom0to1(myAudioParameter->getValue());

    genericAudioParameterValueChanged (myAudioParameter->paramID, myAudioParameter->convertFrom0to1(myAudioParameter->getValue()));
}

#ifdef EDITOR_AND_SLIDERS
void ModularVSTAudioProcessor::myRangedAudioParameterChanged (Slider* mySlider)
{
    for (int i = 0; i < editorSliders->size(); ++i)
        if (mySlider == (*editorSliders)[i].get())
            sliderValues[i] = mySlider->getValue();
    
    genericAudioParameterValueChanged (mySlider->getName(), mySlider->getValue());
}
#endif
void ModularVSTAudioProcessor::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    if (changeBroadcaster == this)
    {
        if (shouldLoadFromBinary)
        {
#ifdef LOAD_ALL_UNITY_INSTRUMENTS
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string (String(totPreset).getCharPointer());
            loadPresetFromPugiDoc (&doc);
            PresetResult res;
            switch (result.status)
            {
                case pugi::status_ok:
                    res = success;
                    break;
                case pugi::status_file_not_found:
                    res = fileNotFound;
                    break;
                default:
                    res = presetNotLoaded;
                    break;
            }
#else
            PresetResult res = loadPreset (presetToLoad, shouldLoadFromBinary);
#endif
            debugLoadPresetResult (res);
            if (res != success)
                for (auto inst : instruments)
                    inst->reReadyAllModules();
            else
            {
                currentlyActiveInstrument = instruments[instruments.size()-1];
                refreshEditor = true;
            }
        } else {
            loadPresetWindowCallback (presetToLoad);
        }
        std::string debugString = String("Preset loaded is: " + presetToLoad).toStdString();
        Debug::Log (debugString);
#ifndef EDITOR_AND_SLIDERS
        refreshSliderValues();
#else
        refreshSlidersFromEditor = true;
#endif

    }
#if defined(NO_EDITOR) || defined(EDITOR_AND_SLIDERS)
    for (int i = 0; i < presetSelectID; ++i)
        prevSliderValues[i] = 0;
#endif

}

void ModularVSTAudioProcessor::setShouldLoadPreset (String filename, bool loadFromBinary, std::function<void(String)> callback)
{
    shouldLoadPreset = true;
    presetToLoad = filename;
    shouldLoadFromBinary = loadFromBinary;
    if (callback != NULL)
        loadPresetWindowCallback = callback;
    
}

void ModularVSTAudioProcessor::LoadIncludedPreset (int i)
{
    setShouldLoadPreset (Global::presetFilesToIncludeInUnity[i], true);
}

void ModularVSTAudioProcessor::changeActiveInstrument (std::shared_ptr<Instrument> instToChangeTo)
{
#ifdef LOAD_ALL_UNITY_INSTRUMENTS
//    std::cout << "Setting states of " << getCurrentlyActiveInstrument()->getName() << " to zero." << std::endl;
    
    for (int i = 0; i < currentlyActiveInstrument->getNumResonatorModules(); ++i)
        currentlyActiveInstrument->getResonatorPtr(i)->myMouseExit(-1, -1, false);
    setToZero = true;
    highlightInstrument (instToChangeTo);
#endif
    currentlyActiveInstrument = instToChangeTo;

//    std::cout << currentlyActiveInstrument->getName() << " is active now." << std::endl;

}


void ModularVSTAudioProcessor::refreshSliderValues()
{
    // refresh parameters
    for (int i = 0; i < allParameters.size(); ++i)
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
        if (i != loadPresetToggleID)
#endif
            myRangedAudioParameterChanged (allParameters[i]);

}
