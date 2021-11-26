/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModularVSTAudioProcessorEditor::ModularVSTAudioProcessorEditor (ModularVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    controlPanel = std::make_unique<ControlPanel> (this);
    addAndMakeVisible (controlPanel.get());
    addModuleWindow = std::make_unique<AddModuleWindow> (this);
    savePresetWindow = std::make_unique<SavePresetWindow>(this);
//    addAndMakeVisible (addModuleWindow.get());
    
    addAndMakeVisible (addChannelButton);
    addChannelButton.addListener (this);
    instruments = audioProcessor.getInstrumentsPtr();
    
    
    startTimerHz (15);
    for (auto inst : *instruments)
    {
        if (inst->areModulesReady())
        {
            addAndMakeVisible (inst.get());
        }
        else{
            std::cout << "NOT READY" << std::endl;
        }
    }
    addAndMakeVisible (controlPanel.get());
        
    setSize (800, 400);
}

ModularVSTAudioProcessorEditor::~ModularVSTAudioProcessorEditor()
{
}

//==============================================================================
void ModularVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (instruments->size() == 0)
    {
        g.setColour (Colours::white);
        g.drawText("Click on \"Add Instrument\" to add an instrument to the application!", getLocalBounds(), Justification::centred);
    }
}

void ModularVSTAudioProcessorEditor::resized()
{

    Rectangle<int> totalArea = getLocalBounds();
    controlPanel->setBounds(totalArea.removeFromBottom (Global::buttonHeight));
    
    int height = static_cast<float>(totalArea.getHeight())
                    / static_cast<float>(instruments->size());
    
    for (auto inst : *instruments)
        if (inst->areModulesReady())
            inst->setBounds(totalArea.removeFromTop (height));
}

void ModularVSTAudioProcessorEditor::buttonClicked (Button* button)
{
}

void ModularVSTAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.shouldRefreshEditor())
        refresh();
    repaint();
}

void ModularVSTAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    if (changeBroadcaster == controlPanel.get())
    {
        if (controlPanel->didComboBoxChange())
        {
            for (auto inst : *instruments)
                inst->setConnectionType (controlPanel->getConnectionType());
            controlPanel->setComboBoxChangeBoolFalse();
        }
        else
        {
            switch(controlPanel->getAction())
            {
                case addInstrumentAction:
                {
                    audioProcessor.addInstrument();
                    addAndMakeVisible (instruments[0][instruments->size()-1].get());
                    break;
                }
                case addResonatorModuleAction:
                {
                    if (instruments->size() != 0)
                        openAddModuleWindow();
                    else
                        std::cout << "No instruments, can't add module." << std::endl;
                    break;
                }
                case addConnectionAction:
                {
                    setApplicationState (addConnectionState);
                    break;
                }
                case cancelConnectionAction:
                {
                    setApplicationState (normalState);
                    break;
                }

            }
            controlPanel->setAction (noAction);
        }
        refresh();
    }
    
    for (auto inst : *instruments)
    {
        if (changeBroadcaster == inst.get())
        {
            switch (inst->getApplicationState())
            {
                case normalState:
                    break;
                case addConnectionState:
                    controlPanel->toggleConnectionTypeBox (true);
                    controlPanel->toggleAddConnectionButton (true);
                    break;
                case firstConnectionState:
                    controlPanel->toggleConnectionTypeBox (false);
                    controlPanel->toggleAddConnectionButton (false);
                    break;
            }
        }
    }
    
    if (changeBroadcaster == addModuleWindow.get())
    {
        addModuleWindow->setAction (noAction);
    }
<<<<<<< Updated upstream
=======
    else if (changeBroadcaster == savePresetWindow.get())
    {
        if (savePresetWindow->getAction() == savePresetFromWindowAction)
            if (savePresetWindow->getDlgSave() == 1)
                audioProcessor.savePreset();

        savePresetWindow->setDlgSave(-1);
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
                    case noAction:
                        break;
                    default:
                        DBG ("Action shouldn't come from instrument");
                        break;
                }
            inst->setAction (noAction);
        }
>>>>>>> Stashed changes

    
}

void ModularVSTAudioProcessorEditor::refresh()
{
    resized();
    for (auto inst : *instruments)
    {
        addAndMakeVisible (inst.get());
        inst->resized();
        inst->addChangeListener (this);
    }
    refreshControlPanel();

}

void ModularVSTAudioProcessorEditor::openAddModuleWindow()
{
    DialogWindow::LaunchOptions dlg;
    int dlgModal = -1;
    addAndMakeVisible (addModuleWindow.get());

    dlg.dialogTitle = "Add Resonator Module";
    dlg.content.set (addModuleWindow.get(), false);
    dlgModal = dlg.runModal();
    
    if (dlgModal == 1)
        audioProcessor.addResonatorModule (addModuleWindow->getResonatorModuleType(), addModuleWindow->getParameters());
}

void ModularVSTAudioProcessorEditor::refreshControlPanel()
{
    if (audioProcessor.getCurrentlyActiveInstrument() == -1)
    {
        controlPanel->toggleAddInstrumentButton (true);
        controlPanel->toggleAddResonatorButton (false);
        controlPanel->toggleAddConnectionButton (false);
        controlPanel->toggleConnectionTypeBox (false);
    }
    else
    {
        switch (applicationState)
        {
            case normalState:
                controlPanel->toggleAddInstrumentButton (true);
                controlPanel->toggleAddResonatorButton (true);
                if (instruments[0][audioProcessor.getCurrentlyActiveInstrument()]->getNumResonatorModules() > 1)
                {
                    controlPanel->toggleAddConnectionButton (true);
                    controlPanel->toggleConnectionTypeBox (true);
                }
                else
                {
                    controlPanel->toggleAddConnectionButton (false);
                    controlPanel->toggleConnectionTypeBox (false);
                }
                break;
            case addConnectionState:
            case firstConnectionState:
            {
                controlPanel->toggleAddInstrumentButton (false);
                controlPanel->toggleAddResonatorButton (false);
            }
        }
    }
}

void ModularVSTAudioProcessorEditor::openSavePresetWindow()
{
    addAndMakeVisible(savePresetWindow.get());
    dlgWindow->showDialog("Save Preset ", savePresetWindow.get(), this, getLookAndFeel().findColour(ResizableWindow::backgroundColourId), true);
}

void ModularVSTAudioProcessorEditor::setApplicationState (ApplicationState a)
{
    applicationState = a;
    controlPanel->setApplicationState (a);
    audioProcessor.setApplicationState (a);
}


