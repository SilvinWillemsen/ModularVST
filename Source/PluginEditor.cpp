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
    savePresetWindow = std::make_unique<SavePresetWindow>(this);
//    addAndMakeVisible (addModuleWindow.get());
    
    // Instruments
    for (auto inst : instruments)
        if (inst->areModulesReady())
        {
            addAndMakeVisible (inst.get());
            currentlyActiveInstrument = inst;
            audioProcessor.setCurrentlyActiveInstrument (inst);
        }
        else
            std::cout << "NOT READY" << std::endl;
        
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
        newSlider->setValue(s->get());
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
    setSize (1500, 600);
#else
    // What is the size of the editor
    setSize (1200, 600);
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
        g.drawText ("Click on \"Add Instrument\" to add an instrument to the application!", getLocalBounds(), Justification::centred);
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
        audioProcessor.dontRefreshEditor();
    }
    repaint();
}

void ModularVSTAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    // If the controlpanel is the broadcaster
    if (changeBroadcaster == controlPanel.get())
    {
        // If the connection type changed (right now this is the only combobox in controlpanel)
        if (controlPanel->didComboBoxChange())
        {
            for (auto inst : instruments)
                inst->setConnectionType (controlPanel->getConnectionType());
            controlPanel->setComboBoxChangeBoolFalse();
            controlPanel->refreshConnectionLabel();
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
                    // Add an instrument and make it visible
                    audioProcessor.addInstrument();
                    std::shared_ptr<Instrument> newInstrument = instruments[instruments.size()-1];
                    currentlyActiveInstrument = newInstrument;
                    addAndMakeVisible (newInstrument.get());
                    newInstrument->addChangeListener (this);
                    newInstrument->resized();
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
                    setApplicationState (removeResonatorModuleState);
                    break;
                }
                case removeResonatorModuleAction:
                {
                    currentlyActiveInstrument->setToRemoveResonatorModule();
                    break;
                }
                case cancelRemoveResonatorModuleAction:
                {
                    setApplicationState (normalState);
                    break;
                }
                case editInOutputsAction:
                {
                    setApplicationState (editInOutputsState);
                    break;
                }
                case cancelInOutputsAction:
                {
                    setApplicationState (normalState);
                    break;
                }

                case editConnectionAction:
                {
                    setApplicationState (editConnectionState);
                    break;
                }
                    
                case changeMassRatioAction:
                {
                    currentlyActiveInstrument->setCustomMassRatio (controlPanel->getCurSliderValue());
                    break;
                }
                case cancelConnectionAction:
                {
                    setApplicationState (normalState);
                    break;
                }
                case savePresetAction:
                {
                    openSavePresetWindow();
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
                }
                else
                {
                    
                // Otherwise disable excitation using exciter modules
                    for (auto inst : instruments)
                        inst->setExcitationType (noExcitation);
                }
                break;
            case noAction:
                break;
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
        if (addModuleWindow->getAction() == addResonatorModuleFromWindowAction)
            if (addModuleWindow->getDlgModal() == 1)
                audioProcessor.addResonatorModule (addModuleWindow->getResonatorModuleType(), addModuleWindow->getParameters(), InOutInfo(), addModuleWindow->isAdvanced());

        addModuleWindow->setDlgModal (-1);
        addModuleWindow->setAction (noAction);
    }
    else if (changeBroadcaster == savePresetWindow.get())
    {
        PresetResult result;
        if (savePresetWindow->getDlgPreset() == 1)
        {
            result = audioProcessor.savePreset (savePresetWindow->getFileName());
        }
        switch (result) {
            case success:
            {
                String message = "Preset \"" + savePresetWindow->getFileName() + "\" has been saved";
                //AlertWindow("File with this name exists", message, "QuestionIcon");
                NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Saved", message, nullptr);
                break;
            }
            default:
            {
                String message = "Preset \"" + savePresetWindow->getFileName() + "\" could not be saved";
                //AlertWindow("File with this name exists", message, "QuestionIcon");
                NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Error", message, nullptr);
                break;
            }
        }


        savePresetWindow->setDlgPreset(-1);
        savePresetWindow->setAction(noAction);
    }
    // If none of the above, the broadcaster has to be an an instrument
    else
    {
       
        for (auto inst : instruments)
        {
            if (applicationState == normalState)
            {
                currentlyActiveInstrument = inst;
                audioProcessor.setCurrentlyActiveInstrument (inst);
            }
            if (changeBroadcaster == inst.get())
                switch (inst->getAction())
                {
                    case changeActiveConnectionAction:
                        controlPanel->setCurrentlyActiveConnection (inst->getCurrentlyActiveConnection()); // nullptr is handled inside funtion
                        controlPanel->refresh (currentlyActiveInstrument);
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

void ModularVSTAudioProcessorEditor::refresh()
{
    for (auto inst : instruments)
    {
        addAndMakeVisible (inst.get());
        inst->resized();
        inst->addChangeListener (this);
    }
    controlPanel->refresh (currentlyActiveInstrument);
    resized();

}

void ModularVSTAudioProcessorEditor::openAddModuleWindow()
{
    addAndMakeVisible (addModuleWindow.get());
    addModuleWindow->triggerComboBox(); // to prevent advanced parameters from appearing when non-advanced parameters should be shown
    if (currentlyActiveInstrument->getCurrentlySelectedResonator() != nullptr)
    {
        if (addModuleWindow->isAdvanced())
        {
            addModuleWindow->getCoefficientList()->setParameters (currentlyActiveInstrument->getCurrentlySelectedResonator()->getParameters());
        } else {
            const NamedValueSet nonAdvancedParameters = currentlyActiveInstrument->getCurrentlySelectedResonator()->getNonAdvancedParameters();
            addModuleWindow->getCoefficientList()->setParameters (nonAdvancedParameters);
        }
    }
    dlgWindow->showDialog ("Add Resonator Module", addModuleWindow.get(), this, getLookAndFeel().findColour (ResizableWindow::backgroundColourId), true);
}

void ModularVSTAudioProcessorEditor::openSavePresetWindow()
{
    addAndMakeVisible(savePresetWindow.get());
    dlgWindow->showDialog("Save Preset", savePresetWindow.get(), this, getLookAndFeel().findColour(ResizableWindow::backgroundColourId), true);
}

void ModularVSTAudioProcessorEditor::openLoadPresetWindow()
{
    stopTimer();
    loadPresetWindow = std::make_unique<FileChooser> ("Load Preset", audioProcessor.getPresetPath(), "*.xml", true, true);
    String resultingFile;
    loadPresetWindow->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
       [this] (const FileChooser& fileChooser) {
        if (fileChooser.getResult().exists())
        {
            String fileName = fileChooser.getResult().getFileName();
            PresetResult res = audioProcessor.loadPreset (fileName);
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
                case success:
                    DBG ("Preset loaded successfully.");
                    break;

                default:
                    break;
            }
            
        }
        startTimerHz (15);
        refresh();
        
    });
    
}
//std::function<void (const FileChooser&)> ModularVSTAudioProcessorEditor::loadPresetCallBack (FileChooser& fileChooser)
//{
////   String resultingFile = fileChooser.getResult().getFullPathName();
////   std::cout << resultingFile << std::endl;
//}

void ModularVSTAudioProcessorEditor::setApplicationState (ApplicationState a)
{
    if (applicationState == removeResonatorModuleState)
        currentlyActiveInstrument->resetResonatorToRemove();
    applicationState = a;
    controlPanel->setApplicationState (a);
    controlPanel->refresh (currentlyActiveInstrument);
    audioProcessor.setApplicationState (a);
}

#ifdef EDITOR_AND_SLIDERS
void ModularVSTAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    for (int i = 0; i < parameters.size(); ++i)
    {
        if (parameters[i].get() == slider)
            audioProcessor.myAudioParameterFloatValueChanged (slider);
    }
}
#endif
