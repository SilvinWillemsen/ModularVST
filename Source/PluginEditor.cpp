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
    
    // What is the size of the editor
    setSize (1200, 600);
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
                    PresetResult res = audioProcessor.savePreset();
                    
                    switch (res)
                    {
                        case success:
                            DBG ("Preset successfully saved");
                            break;
                        default:
                            DBG ("Something went wrong while saving the preset");
                            break;
                    }
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
//        addAndMakeVisible (inst.get());
        inst->resized();
        inst->addChangeListener (this);
//        inst->addChangeListener (this);
    }
    controlPanel->refresh (currentlyActiveInstrument);
    resized();

}

void ModularVSTAudioProcessorEditor::openAddModuleWindow()
{
    addAndMakeVisible (addModuleWindow.get());
    addModuleWindow->triggerComboBox(); // to prevent advanced parameters from appearing when non-advanced parameters should be shown
    dlgWindow->showDialog ("Add Resonator Module", addModuleWindow.get(), this, getLookAndFeel().findColour (ResizableWindow::backgroundColourId), true);
}

void ModularVSTAudioProcessorEditor::setApplicationState (ApplicationState a)
{
    if (applicationState == removeResonatorModuleState)
        currentlyActiveInstrument->resetResonatorToRemove();
    applicationState = a;
    controlPanel->setApplicationState (a);
    controlPanel->refresh (currentlyActiveInstrument);
    audioProcessor.setApplicationState (a);
}
