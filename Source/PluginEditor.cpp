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
    {
        refresh();
        audioProcessor.dontRefreshEditor();
    }
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
                case removeResonatorModuleAction:
                {
                    setApplicationState (removeResonatorModuleState);
                    break;
                }
                case cancelRemoveResonatorModuleAction:
                {
                    instruments[0][audioProcessor.getCurrentlyActiveInstrument()]->setToRemoveResonatorModule();
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
                case cancelConnectionAction:
                {
                    setApplicationState (normalState);
                    break;
                }
                case savePresetAction:
                {
                    audioProcessor.savePreset();
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
                case editConnectionState:
                    controlPanel->toggleConnectionTypeBox (true);
                    controlPanel->toggleEditConnectionButton (true);
                    break;
                case firstConnectionState:
                    controlPanel->toggleConnectionTypeBox (false);
                    controlPanel->toggleEditConnectionButton (false);
                    break;
            }
        }
    }
    
    if (changeBroadcaster == addModuleWindow.get())
    {
        addModuleWindow->setAction (noAction);
    }

    
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
    controlPanel->refresh (instruments, audioProcessor.getCurrentlyActiveInstrument());
}

void ModularVSTAudioProcessorEditor::setApplicationState (ApplicationState a)
{
    applicationState = a;
    controlPanel->setApplicationState (a);
    audioProcessor.setApplicationState (a);
}
