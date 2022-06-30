/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModularVSTAudioProcessorEditor::ModularVSTAudioProcessorEditor (ModularVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), instruments (audioProcessor.getInstrumentsRef())
{
    
    // Allow the editor to be resized
    setResizable (true, true);
    
    // Panels
    controlPanel = std::make_unique<ControlPanel> (this);
    addAndMakeVisible (controlPanel.get());
    
    excitationPanel = std::make_unique<ExcitationPanel> (this);
    addAndMakeVisible (excitationPanel.get());
    
    // Window for adding modules
    addModuleWindow = std::make_unique<AddModuleWindow> (this);
    loadPresetWindow = std::make_unique<LoadPresetWindow>(this);
//    addAndMakeVisible (addModuleWindow.get());
    
    // Instruments
    for (auto inst : instruments)
        if (inst->areModulesReady())
        {
            addAndMakeVisible (inst.get());
            audioProcessor.setCurrentlyActiveInstrument (inst);
            
        }
        else
            std::cout << "NOT READY" << std::endl;
    
    if (getCurrentlyActiveInstrument() != nullptr)
        controlPanel->setNumGroups (getCurrentlyActiveInstrument()->getNumResonatorGroups());
    
    if (excitationPanel->getExciteMode())
    {
        for (auto inst : instruments)
            inst->setExcitationType (excitationPanel->getExcitationType());
        audioProcessor.setExcitationType (excitationPanel->getExcitationType());
    }
    else
    {
        
    // Otherwise disable excitation using exciter modules
        for (auto inst : instruments)
            inst->setExcitationType (noExcitation);
        audioProcessor.setExcitationType (noExcitation);
    }

    // At what rate to refresh the states of the system
    startTimerHz (15);
    
    
#ifdef EDITOR_AND_SLIDERS
    parameters.reserve (8);
    parameterLabels.reserve (8);
    for (auto s : audioProcessor.getMyParameters())
    {
        std::shared_ptr<Slider> newSlider = std::make_shared<Slider> (Slider::SliderStyle::LinearHorizontal, Slider::TextBoxRight);
        newSlider->setName (s->paramID);
        newSlider->setRange (s->getNormalisableRange().start, s->getNormalisableRange().end, s->getNormalisableRange().interval);
        newSlider->setValue (s->getValue());
        newSlider->addListener (this);
        
        std::shared_ptr<Label> newLabel = std::make_shared<Label> (s->name, s->name);
        newLabel->setColour(Label::backgroundColourId, Colours::transparentBlack);
        newLabel->setColour(Label::textColourId, Colours::white);
        
        addAndMakeVisible (newSlider.get());
        addAndMakeVisible (newLabel.get());

        parameters.push_back (newSlider);
        parameterLabels.push_back (newLabel);
        
        audioProcessor.setEditorSliders (&parameters);
    }
    refreshSliderValues();

    setSize (1500, 600);
#else
    // What is the size of the editor
    setSize (1200, 600); // , 300) for manual figures
#endif
    
}

ModularVSTAudioProcessorEditor::~ModularVSTAudioProcessorEditor()
{
}

//==============================================================================
void ModularVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Just paint the editor if there are no instruments.
    // Otherwise all that you see is from the child components.
    
    if (instruments.size() == 0)
    {
        g.setColour (Colours::white);
#ifdef USE_RESET_BUTTON
        g.drawText ("Click on \"Reset\" to start building your instrument!", getLocalBounds(), Justification::centred);
#else
        g.drawText ("Click on \"Add Instrument\" to add an instrument to the application!", getLocalBounds(), Justification::centred);
#endif
        Rectangle<int> area = getLocalBounds()
            .withHeight (getHeight() - (2.0 * Global::margin + Global::buttonHeight))
            .withWidth (getWidth() - (2.0 * Global::margin + Global::buttonWidth))
            .reduced (Global::margin, Global::margin);
        g.drawRect (area);
    }
}

void ModularVSTAudioProcessorEditor::resized()
{

    Rectangle<int> totalArea = getLocalBounds();
    
#ifdef EDITOR_AND_SLIDERS
    Rectangle<int> sliderArea = totalArea.removeFromRight (300);
    for (int i = 0; i < parameters.size(); ++i )
    {
        Rectangle<int> curSliderArea = sliderArea.removeFromTop (Global::buttonHeight);
        parameterLabels[i]->setBounds (curSliderArea.removeFromLeft(curSliderArea.getWidth() * 0.33));
        parameters[i]->setBounds (curSliderArea);
    }
#endif
    // Panels
    controlPanel->setBounds (totalArea.removeFromBottom (Global::buttonHeight + 2.0 * Global::margin));
    excitationPanel->setBounds (totalArea.removeFromRight (Global::buttonWidth + 2.0 * Global::margin));

    // Divide the height of the left-over area over the total number of instruments
    int height = static_cast<float>(totalArea.getHeight())
                    / static_cast<float>(instruments.size());
    
    for (auto inst : instruments)
        if (inst->areModulesReady())
            inst->setBounds(totalArea.removeFromTop (height));
}

void ModularVSTAudioProcessorEditor::buttonClicked (Button* button)
{
}

void ModularVSTAudioProcessorEditor::timerCallback()
{
    // LOOK AT THE CONTENTS OF THIS FUNCTION
    for (auto inst : instruments)
        if (inst->checkIfShouldRemoveResonatorModule())
        {
            inst->removeResonatorModule();
            audioProcessor.setStatesToZero (false);
            refresh();
        }
        
    if (audioProcessor.shouldRefreshEditor())
    {
        refresh();
#if (BUILD_CONFIG != 2)
        if (excitationPanel->getExciteMode())
            for (auto inst : instruments)
                inst->setExcitationType (excitationPanel->getExcitationType());
#endif
        audioProcessor.dontRefreshEditor();
    }
#ifdef EDITOR_AND_SLIDERS
    if (audioProcessor.shouldRefreshSlidersFromEditor())
        refreshSliderValues();
#endif
    
    repaint();
}

void ModularVSTAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    // If the controlpanel is the broadcaster
    if (changeBroadcaster == controlPanel.get())
    {
        // If the connection type changed (right now this is the only combobox in controlpanel)
        if (controlPanel->didConnectionTypeComboBoxChange() || controlPanel->didResonatorGroupComboBoxChange() )
        {
            if (controlPanel->didConnectionTypeComboBoxChange())
            {
                if (getCurrentlyActiveInstrument() != nullptr)
                    getCurrentlyActiveInstrument()->setConnectionType (controlPanel->getConnectionType());
                controlPanel->setConnectionTypeComboBoxChangeBoolFalse();
                controlPanel->refreshConnectionLabel();
            }
            if (controlPanel->didResonatorGroupComboBoxChange())
            {
                getCurrentlyActiveInstrument()->setCurrentlySelectedResonatorGroup (controlPanel->getCurrentResonatorGroup());
                
                controlPanel->setResonatorGroupComboBoxChangeBoolFalse();
            }
        }
        else
        {
            switch(controlPanel->getAction())
            {
                case noAction:
                    DBG ("Control panel does not have an action");
                    break;
                case addInstrumentAction:
                {
#ifndef USE_RESET_BUTTON
                    // Add an instrument and make it visible
                    audioProcessor.addInstrument();
                    std::shared_ptr<Instrument> newInstrument = instruments[instruments.size()-1];
                    audioProcessor.setCurrentlyActiveInstrument (newInstrument);
                    controlPanel->setNumGroups (0);
                    addAndMakeVisible (newInstrument.get());
                    newInstrument->addChangeListener (this);
                    newInstrument->resized();
#else
                    stopTimer();
                    for (auto inst : instruments)
                    {
                        inst->unReadyAllModules();
                        audioProcessor.setStatesToZero (true);
                    }
                    String emptyInst = "EmptyInstrument_xml";
                    PresetResult res = audioProcessor.loadPreset (emptyInst, true);
                    if (res != success)
                        for (auto inst : instruments)
                            inst->reReadyAllModules();
                    else
                        audioProcessor.setCurrentlyActiveInstrument(instruments[instruments.size()-1]);
                    refresh();

#endif
                    break;

                }
                case addResonatorModuleAction:
                {
                    if (instruments.size() != 0)
                        openAddModuleWindow();
                    else
                        std::cout << "Shouldn't happen. Button is inactive" << std::endl;
                    break;
                }
                case editResonatorModulesAction:
                {
                    if (applicationState == normalState)
                        setApplicationState (removeResonatorModuleState);
                    else
                        setApplicationState (normalState);
                    break;
                }
                case removeResonatorModuleAction:
                {
                    getCurrentlyActiveInstrument()->setToRemoveResonatorModule();
                    break;
                }

                case editInOutputsAction:
                {
                    if (applicationState == normalState)
                        setApplicationState (editInOutputsState);
                    else
                        setApplicationState (normalState);

                    break;
                }

                case editConnectionAction:
                {
                    if (applicationState == normalState)
                        setApplicationState (editConnectionState);
                    else
                        setApplicationState (normalState);

                    break;
                }
                    
                case editDensityAction:
                {
                    if (applicationState == editConnectionState)
                        setApplicationState (editDensityState);
                    else
                        setApplicationState (editConnectionState);

                    break;
                }
                case densitySliderAction:
                {
                    getCurrentlyActiveInstrument()->getCurrentlySelectedResonator()->changeDensity (controlPanel->getCurSliderValue());
                    break;
                }
                case editResonatorGroupsAction:
                {
                    if (applicationState == normalState)
                        setApplicationState (editResonatorGroupsState);
                    else
                        setApplicationState (normalState);
                    break;
                }
                case addResonatorGroupAction:
                {
                    addResonatorGroup();
                    break;
                }
                case removeResonatorGroupAction:
                {
                    removeResonatorGroup();
                    break;
                }
                case savePresetAction:
                {
                    savePresetFromWindow();
                    break;
                }
                case loadPresetAction:
                {
                    openLoadPresetWindow();
                    break;
                }
                default:
                    DBG ("Action shouldn't come from controlpanel");
                    break;
            }
            
            // Reset the action of the controlpanel
            controlPanel->setAction (noAction);
        }
    }
    // If the excitation panel is the broadcaster
    else if (changeBroadcaster == excitationPanel.get())
    {
        switch (excitationPanel->getAction())
        {
            // If the excite button is pressed or the excitation type has changed
            case exciteAction:
            case changeExcitationAction:
                
                // If the application should be excited using the exciter modules
                if (excitationPanel->getExciteMode())
                {
                    for (auto inst : instruments)
                        inst->setExcitationType (excitationPanel->getExcitationType());
                    audioProcessor.setExcitationType (excitationPanel->getExcitationType());
                }
                else
                {
                    
                // Otherwise disable excitation using exciter modules
                    for (auto inst : instruments)
                        inst->setExcitationType (noExcitation);
                    audioProcessor.setExcitationType (noExcitation);
                }
                break;
            case noAction:
                break;
            case graphicsToggleAction:
            {
                toggleGraphics();
                break;
            }
            default:
                DBG ("Action shouldn't come from excitation panel");
                break;
        }
        // Reset the action of the excitation panel
        excitationPanel->setAction (noAction);
    }
    // If the the broadcaster is the add module window
    else if (changeBroadcaster == addModuleWindow.get())
    {
        for (int i = 0; i < addModuleWindow->getParameters().size(); ++i)
        {
            std::cout << addModuleWindow->getParameters().getName (i).toString() << ": " << (double)*(addModuleWindow->getParameters().getVarPointerAt(i)) << std::endl;
        }
        
        if (addModuleWindow->getAction() == addResonatorModuleFromWindowAction)
            if (addModuleWindow->getDlgModal() == 1)
                audioProcessor.addResonatorModule (addModuleWindow->getResonatorModuleType(), addModuleWindow->getParameters(), InOutInfo(), addModuleWindow->isAdvanced());

        addModuleWindow->setDlgModal (-1);
        addModuleWindow->setAction (noAction);
    }
    else if (changeBroadcaster == loadPresetWindow.get())
    {
        action = loadPresetWindow->getAction();
        String fileName = BinaryData::namedResourceList[loadPresetWindow->getSelectedBinaryPreset()];

        if (action == loadBinaryPresetAction)
        {
            audioProcessor.setShouldLoadPreset (fileName, true);
            loadPresetWindow->setAction (noAction);

        }
        else if (action == loadPresetFromWindowAction)
        {
            loadPresetFromWindow();
//            audioProcessor.setShouldLoadPreset (fileName, false, loadPresetFromWindow());
            loadPresetWindow->setAction(noAction);
        }


    }
    // If none of the above, the broadcaster has to be an an instrument
    else
    {
       
        for (auto inst : instruments)
        {
            if (changeBroadcaster == inst.get())
            {
//                std::cout << inst->getName() << " is the changeBroadcaster." << std::endl;
                if (inst != getCurrentlyActiveInstrument())
                {
                    audioProcessor.changeActiveInstrument (inst);
                }
                
                if (applicationState == normalState)
                {
//                    audioProcessor.setCurrentlyActiveInstrument (inst);
                    controlPanel->setNumGroups (getCurrentlyActiveInstrument()->getNumResonatorGroups());

                }
                switch (inst->getAction())
                {
                    case changeActiveConnectionAction:
                        controlPanel->setCurrentlyActiveConnection (inst->getCurrentlyActiveConnection()); // nullptr is handled inside funtion
                        controlPanel->refresh (getCurrentlyActiveInstrument());
                        break;
                    case updateDensityAction:
                        controlPanel->setDensitySliderValueFromResonator (getCurrentlyActiveInstrument()->getCurrentlySelectedResonator());
                        controlPanel->refresh (getCurrentlyActiveInstrument());
                        break;
                    // when a module has just been added
                    case refreshEditorAction:
                        refresh();
                        break;
                    case setStatesToZeroAction:
                        audioProcessor.setStatesToZero (true);
                        break;
                    case noAction:
                        break;
                    default:
                        DBG ("Action shouldn't come from instrument");
                        break;
                }
                inst->setAction (noAction);
            }
        }
    }
}

void ModularVSTAudioProcessorEditor::refresh()
{
    for (auto inst : instruments)
    {
        addAndMakeVisible (inst.get());
        inst->resized();
        inst->addChangeListener (this);
    }
    startTimerHz (15);
    controlPanel->refresh (getCurrentlyActiveInstrument());
    resized();

}

void ModularVSTAudioProcessorEditor::openAddModuleWindow()
{
    addAndMakeVisible (addModuleWindow.get());
    ResonatorModule* curSelectedResonator = getCurrentlyActiveInstrument()->getCurrentlySelectedResonator().get();

    if (curSelectedResonator != nullptr)
    {
        addModuleWindow->triggerComboBox (curSelectedResonator->getResonatorModuleType());
        if (addModuleWindow->isAdvanced())
        {
            addModuleWindow->getCoefficientList()->setParameters (getCurrentlyActiveInstrument()->getCurrentlySelectedResonator()->getParameters());
        } else {
            const NamedValueSet nonAdvancedParameters = getCurrentlyActiveInstrument()->getCurrentlySelectedResonator()->getNonAdvancedParameters();
            addModuleWindow->getCoefficientList()->setParameters (nonAdvancedParameters);
        }
    } else {
        addModuleWindow->triggerComboBox(); // to prevent advanced parameters from appearing when non-advanced parameters should be shown
    }
    dlgWindow->showDialog ("Add Resonator Module", addModuleWindow.get(), this, getLookAndFeel().findColour (ResizableWindow::backgroundColourId), true);
}

void ModularVSTAudioProcessorEditor::openLoadPresetWindow()
{

    addAndMakeVisible(loadPresetWindow.get());
    dlgWindow->showDialog("Load Preset", loadPresetWindow.get(), this, getLookAndFeel().findColour(ResizableWindow::backgroundColourId), true);
}
void ModularVSTAudioProcessorEditor::loadPresetFromWindow()
{
    stopTimer();
    for (auto inst : instruments)
    {
        inst->unReadyAllModules();
        audioProcessor.setStatesToZero (true);
    }

    openloadPresetFromWindow = std::make_unique<FileChooser> ("Load Preset", File::getCurrentWorkingDirectory().getChildFile(audioProcessor.getPresetPath()), "*.xml", true, true);
    openloadPresetFromWindow->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this] (const FileChooser& fileChooser) {
        if (fileChooser.getResult().exists())
        {
            String filePath = fileChooser.getResult().getFullPathName();
            audioProcessor.setShouldLoadPreset (filePath, false, [this](String filePath) {
                PresetResult res = audioProcessor.loadPreset (filePath, false);
                audioProcessor.debugLoadPresetResult (res);
                if (res != success)
                    for (auto inst : instruments)
                        inst->reReadyAllModules();
                else
                {
                    audioProcessor.setCurrentlyActiveInstrument (instruments[instruments.size()-1]);
                }
//#ifdef EDITOR_AND_SLIDERS
                audioProcessor.refreshSliderValues();
//#endif

            });

        } else {
            Logger::getCurrentLogger()->outputDebugString ("Loading was cancelled.");
            for (auto inst : instruments)
                inst->reReadyAllModules();
        }
        refresh();
    });
}

void ModularVSTAudioProcessorEditor::savePresetFromWindow()
{
    stopTimer();
    for (auto inst : instruments)
    {
        inst->unReadyAllModules();
        audioProcessor.setStatesToZero (true);
    }

    savePresetFileChooser = std::make_unique<FileChooser> ("Save Preset", File::getCurrentWorkingDirectory().getChildFile("myPreset.xml"), "*.xml", true, true);
    savePresetFileChooser->launchAsync (FileBrowserComponent::saveMode, [this] (const FileChooser& fileChooser) {
            String filePath = fileChooser.getResult().getFullPathName();
            String fileName = fileChooser.getResult().getFileName();
            PresetResult res = fileName != "" ? audioProcessor.savePreset (filePath) : PresetResult::loadingCancelled;
            
            switch (res) {
                case success:
                {
                    String message = "Preset \"" + fileName + "\" has been saved";
                    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Saved", message, nullptr);
                    break;
                }
                case loadingCancelled:
                {
                    String message = "Saving was cancelled";
                    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Cancelled", message, nullptr);
                    break;
                }
                default:
                {
                    String message = "Preset could not be saved";
                    //AlertWindow("File with this name exists", message, "QuestionIcon");
                    NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Error", message, nullptr);
                    break;
                }
            }
            refresh();
        });
    for (auto inst : instruments)
        inst->reReadyAllModules();

}

void ModularVSTAudioProcessorEditor::setApplicationState (ApplicationState a)
{
    if (applicationState == removeResonatorModuleState)
        getCurrentlyActiveInstrument()->resetResonatorToRemove();
    applicationState = a;
    excitationPanel->setEnabled (applicationState == normalState);
    controlPanel->setApplicationState (a);
    controlPanel->refresh (getCurrentlyActiveInstrument());
    audioProcessor.setApplicationState (a);
}

void ModularVSTAudioProcessorEditor::addResonatorGroup()
{
    controlPanel->addResonatorGroup();
    getCurrentlyActiveInstrument()->addResonatorGroup();
}

void ModularVSTAudioProcessorEditor::removeResonatorGroup()
{
    if (controlPanel->getNumGroups() == 0)
        return;
    getCurrentlyActiveInstrument()->removeResonatorGroup (controlPanel->getCurrentResonatorGroup() - 1);
    controlPanel->removeResonatorGroup();

}

#ifdef EDITOR_AND_SLIDERS
void ModularVSTAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    for (int i = 0; i < parameters.size(); ++i)
    {
        if (parameters[i].get() == slider)
            audioProcessor.myRangedAudioParameterChanged (slider);
    }
}

void ModularVSTAudioProcessorEditor::refreshSliderValues()
{
    // refresh parameters
    for (int i = 0; i < parameters.size(); ++i)
        if (parameters[i]->getName() != "loadPresetToggle")
            audioProcessor.myRangedAudioParameterChanged (parameters[i].get());
    
    audioProcessor.setRefreshSlidersFromEditor (false);

}

#endif

void ModularVSTAudioProcessorEditor::toggleGraphics()
{
    if (excitationPanel->getGraphicsToggle())
        startTimerHz (15);
    else
        stopTimer();
}
