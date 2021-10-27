/*
  ==============================================================================

    ControlPanel.cpp
    Created: 4 Sep 2021 1:22:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ControlPanel.h"

//==============================================================================
ControlPanel::ControlPanel (ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    allButtons.reserve (8);
    allComboBoxes.reserve (8);

    addInstrumentButton = std::make_shared<TextButton> ("Add Instrument");
    allButtons.push_back (addInstrumentButton);

    addResonatorModuleButton = std::make_shared<TextButton> ("Add Resonator Module");
    allButtons.push_back (addResonatorModuleButton);

    removeResonatorModuleButton = std::make_shared<TextButton> ("Remove Resonator Module");
    allButtons.push_back (removeResonatorModuleButton);
    
    editInOutputsButton = std::make_shared<TextButton> ("Edit In- Outputs");
    allButtons.push_back (editInOutputsButton);
    
    editConnectionButton = std::make_shared<TextButton> ("Edit Connections");
    allButtons.push_back (editConnectionButton);

    savePresetButton = std::make_unique<TextButton> ("Save Preset");
    allButtons.push_back (savePresetButton);

    for (int i = 0; i < allButtons.size(); ++i)
    {
        allButtons[i]->addListener (this);
        addAndMakeVisible (allButtons[i].get());
    }
    
    connectionTypeBox = std::make_shared<ComboBox> ();
    connectionTypeBox->addItem ("Rigid", rigid);
    connectionTypeBox->addItem ("Linear Spring", linearSpring);
    connectionTypeBox->addItem ("Nonlinear spring", nonlinearSpring);

    allComboBoxes.push_back (connectionTypeBox);
    
    for (int i = 0; i < allComboBoxes.size(); ++i)
    {
        allComboBoxes[i]->addListener (this);
        addAndMakeVisible(allComboBoxes[i].get());
        allComboBoxes[i]->setSelectedId (1);
    }
    
    addChangeListener (audioProcessorEditor);
        
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (14.0f);
//    g.drawText ("ControlPanel", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void ControlPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> area = getLocalBounds();
    for (int i = 0; i < allButtons.size(); ++i)
    {
        if (allButtons[i]->isVisible())
        {
            if (applicationState == normalState)
            {
                if (i == 5)
                {
                    allButtons[i]->setBounds (area.removeFromRight (100));
                }
                else
                {
                    allButtons[i]->setBounds (area.removeFromLeft (100));
                }
                area.removeFromLeft (Global::margin);

            }
            else
            {
                allButtons[i]->setBounds (area.removeFromRight (100));
                if (i == 4 && allComboBoxes[0]->isVisible())
                {
                    area.removeFromRight (Global::margin);
                    allComboBoxes[0]->setBounds(area.removeFromRight (100));
                }
                area.removeFromRight (Global::margin);

            }
            
            
           
            
        }
    }
        

}

void ControlPanel::buttonClicked (Button* button)
{
    if (button == addInstrumentButton.get())
        action = addInstrumentAction;
    else if (button == addResonatorModuleButton.get())
        action = addResonatorModuleAction;
    else if (button == removeResonatorModuleButton.get())
    {
        if (applicationState == normalState)
        {
            action = removeResonatorModuleAction;
            removeResonatorModuleButton->setButtonText ("Remove");
        }
        else if (applicationState == removeResonatorModuleState)
        {
            action = cancelRemoveResonatorModuleAction;
            removeResonatorModuleButton->setButtonText ("Remove Resonator Module");
        }
    }
    else if (button == editInOutputsButton.get())
    {
        if (applicationState == normalState)
        {
            action = editInOutputsAction;
            editInOutputsButton->setButtonText ("Done");
        }
        else if (applicationState == editInOutputsState)
        {
            action = cancelInOutputsAction;
            editInOutputsButton->setButtonText ("Edit In- Outputs");
        }
    }
    else if (button == editConnectionButton.get())
    {
        if (applicationState == normalState)
        {
            action = editConnectionAction;
            editConnectionButton->setButtonText ("Done");
        }
        else if (applicationState == editConnectionState)
        {
            action = cancelConnectionAction;
            editConnectionButton->setButtonText ("Edit Connections");
        }
    }
    else if (button == savePresetButton.get())
    {
        action = savePresetAction;
    }
    sendChangeMessage();
}

void ControlPanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    comboBoxChangeBool = true;
    connectionType = static_cast<ConnectionType> (connectionTypeBox->getSelectedId());
//    if (applicationState == normalState && init)
//        editConnectionButton->triggerClick();
//    else
    sendChangeMessage();

//    init = true;
}

void ControlPanel::refresh (std::vector<std::shared_ptr<Instrument>>* instruments, int currentlyActiveInstrument)
{
    std::vector<bool> activeButtons (allButtons.size(), false);
    std::vector<bool> activeComboBoxes (allComboBoxes.size(), false);
    if (currentlyActiveInstrument == -1)
    {
        activeButtons[0] = true;
    }
    else
    {
        switch (applicationState)
        {
            case normalState:
                activeButtons[0] = true;
                activeButtons[1] = true;
                activeButtons[5] = true;

                if (instruments[0][currentlyActiveInstrument]->getNumResonatorModules() != 0)
                {
                    activeButtons[2] = true;
                    activeButtons[3] = true;
                }
                if (instruments[0][currentlyActiveInstrument]->getNumResonatorModules() > 1)
                {
                    activeButtons[4] = true;
                }
                break;
            case removeResonatorModuleState:
                activeButtons[2] = true;
                break;
            case editInOutputsState:
                activeButtons[3] = true;
                break;

            case editConnectionState:
            case moveConnectionState:
            case firstConnectionState:
                activeButtons[4] = true;
                activeComboBoxes[0] = true;
                break;
        }
    }
    for (int i = 0; i < allButtons.size(); ++i)
    {
        allButtons[i]->setVisible (activeButtons[i]);
    }
    
    for (int i = 0; i < allComboBoxes.size(); ++i)
    {
        allComboBoxes[i]->setVisible (activeComboBoxes[i]);
    }
    resized();
}
