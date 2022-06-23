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
    allSliders.reserve (8);

#ifndef USE_RESET_BUTTON
    addInstrumentButton = std::make_shared<TextButton> ("Add Instrument");
#else
    addInstrumentButton = std::make_shared<TextButton> ("Reset");
#endif
    allButtons.push_back (addInstrumentButton);

    addResonatorModuleButton = std::make_shared<TextButton> ("Add Resonator Module");
    allButtons.push_back (addResonatorModuleButton);

    editResonatorModuleButton = std::make_shared<TextButton> ("Remove Resonator Module");
    allButtons.push_back (editResonatorModuleButton);
    
    removeResonatorModuleButton = std::make_unique<TextButton> ("Remove");
    allButtons.push_back (removeResonatorModuleButton);
    
    editInOutputsButton = std::make_shared<TextButton> ("Edit In- Outputs");
    allButtons.push_back (editInOutputsButton);
    
    editConnectionButton = std::make_shared<TextButton> ("Edit Connections");
    allButtons.push_back (editConnectionButton);
    
    editDensityButton = std::make_shared<TextButton> ("Edit Density");
    allButtons.push_back (editDensityButton);

    savePresetButton = std::make_unique<TextButton> ("Save Preset");
    allButtons.push_back (savePresetButton);
   
    loadPresetButton = std::make_unique<TextButton> ("Load Preset");
    allButtons.push_back (loadPresetButton);

    editResonatorGroupsButton = std::make_unique<TextButton> ("Edit Groups");
    allButtons.push_back (editResonatorGroupsButton);

    addResonatorGroupButton = std::make_unique<TextButton> ("Add Group");
    allButtons.push_back (addResonatorGroupButton);
    
    removeResonatorGroupButton = std::make_unique<TextButton> ("Remove Group");
    allButtons.push_back (removeResonatorGroupButton);

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
    
    resonatorGroupBox = std::make_shared<ComboBox> ();

    allComboBoxes.push_back (resonatorGroupBox);
    
    for (int i = 0; i < allComboBoxes.size(); ++i)
    {
        allComboBoxes[i]->addListener (this);
        addAndMakeVisible(allComboBoxes[i].get());
        if (allComboBoxes[i] == resonatorGroupBox)
            allComboBoxes[i]->setSelectedId (0);
        else
            allComboBoxes[i]->setSelectedId (1);

    }
    
    densitySlider = std::make_shared<Slider>();
    densitySlider->setRange (10, 100000);
    densitySlider->setSkewFactorFromMidPoint (1000.0);
    allSliders.push_back (densitySlider);

    for (int i = 0; i < allSliders.size(); ++i)
    {
        allSliders[i]->addListener (this);
        addAndMakeVisible(allSliders[i].get());
    }
    
    instructionsLabel1 = std::make_shared<Label>("Instructions test");
    instructionsLabel1->setColour(Label::ColourIds::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (instructionsLabel1.get());
    instructionsLabel1->setVisible (true);
    
    instructionsLabel2 = std::make_shared<Label>("Instructions test");
    instructionsLabel2->setColour(Label::ColourIds::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (instructionsLabel2.get());
    instructionsLabel2->setVisible (true);

    connectionLabel = std::make_shared<Label>("Connections");
    connectionLabel->setColour(Label::ColourIds::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (connectionLabel.get());
    connectionLabel->setVisible (false);

    
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
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

}

void ControlPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> area = getLocalBounds();
    area.reduce (Global::margin, Global::margin);
    for (int i = 0; i < allButtons.size(); ++i)
    {
        if (allButtons[i]->isVisible())
        {
            if (applicationState == normalState)
            {
                if (allButtons[i] == savePresetButton)
                {
                    allButtons[i]->setBounds (area.removeFromRight (100));
                    area.removeFromRight (Global::margin);

                }
                else if (allButtons[i] == loadPresetButton)
                {
                    allButtons[i]->setBounds (area.removeFromRight (100));
                    area.removeFromRight (Global::margin);
                }
                else
                {
                    allButtons[i]->setBounds (area.removeFromLeft (100));
                    area.removeFromLeft (Global::margin);
                }

            }
            else
            {
                allButtons[i]->setBounds (area.removeFromRight (100));
                if (allButtons[i] == editConnectionButton && allComboBoxes[0]->isVisible())
                {
                    area.removeFromRight (Global::margin);
                    allComboBoxes[0]->setBounds(area.removeFromRight (100));
//                    area.removeFromRight (Global::margin);
//                    allSliders[0]->setBounds(area.removeFromRight (300));

                }
                else if (allButtons[i] == editDensityButton
                         && applicationState == editDensityState
                         && currentlyActiveResonator != nullptr)
                {
                    area.removeFromRight (Global::margin);
                    allSliders[0]->setBounds (area.removeFromRight (300));
//                    area.removeFromRight (Global::margin);
//                    allSliders[0]->setBounds(area.removeFromRight (300));

                }
                else if (allButtons[i] == removeResonatorGroupButton && allComboBoxes[1]->isVisible())
                {
                    area.removeFromRight (Global::margin);
                    allComboBoxes[1]->setBounds(area.removeFromRight (100));
//                    area.removeFromRight (Global::margin);
//                    allSliders[0]->setBounds(area.removeFromRight (300));

                }
                area.removeFromRight (Global::margin);

            }
        }
    }
    if (connectionLabel->isVisible())
        connectionLabel->setBounds (area.removeFromRight (area.getWidth()*0.3333));
    
    if (instructionsLabel1->isVisible())
    {
        instructionsLabel1->setBounds (area.removeFromLeft (area.getWidth()*0.5));
        instructionsLabel2->setBounds (area);
    }

}

void ControlPanel::buttonClicked (Button* button)
{
    if (button == addInstrumentButton.get())
        action = addInstrumentAction;
    else if (button == addResonatorModuleButton.get())
        action = addResonatorModuleAction;
    else if (button == editResonatorModuleButton.get())
    {
        action = editResonatorModulesAction;
        if (applicationState == normalState)
            editResonatorModuleButton->setButtonText ("Done");
        else if (applicationState == removeResonatorModuleState)
            editResonatorModuleButton->setButtonText ("Remove Resonator Module");
    }
    else if (button == removeResonatorModuleButton.get())
    {
        action = removeResonatorModuleAction;
    }
    else if (button == editInOutputsButton.get())
    {
        action = editInOutputsAction;
        if (applicationState == normalState)
            editInOutputsButton->setButtonText ("Done");
        else if (applicationState == editInOutputsState)
            editInOutputsButton->setButtonText ("Edit In- Outputs");

    }
    else if (button == editConnectionButton.get())
    {
        action = editConnectionAction;
        if (applicationState == normalState)
            editConnectionButton->setButtonText ("Done");
        else if (applicationState == editConnectionState)
            editConnectionButton->setButtonText ("Edit Connections");

    }
    else if (button == editDensityButton.get())
    {
        action = editDensityAction;
        if (applicationState == editConnectionState)
        {
            setCurrentlyActiveResonator (nullptr);
            editDensityButton->setButtonText ("Done");
        }
        else if (applicationState == editDensityState)
        {
            editDensityButton->setButtonText ("Edit Density");
        }

    }
    else if (button == editResonatorGroupsButton.get())
    {
        action = editResonatorGroupsAction;
        if (applicationState == normalState)
            editResonatorGroupsButton->setButtonText ("Done");
        else if (applicationState == editResonatorGroupsState)
            editResonatorGroupsButton->setButtonText ("Edit Groups");

    }
    else if (button == addResonatorGroupButton.get())
    {
        action = addResonatorGroupAction;
    }
    else if (button == removeResonatorGroupButton.get())
    {
        action = removeResonatorGroupAction;
    }
    else if (button == savePresetButton.get())
    {
        action = savePresetAction;
    }
    else if (button == loadPresetButton.get())
    {
        action = loadPresetAction;
    }

    sendChangeMessage();
}

void ControlPanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == connectionTypeBox.get())
    {
        connectionTypeComboBoxChangeBool = true;
    }
    else if (comboBoxThatHasChanged == resonatorGroupBox.get())
    {
        resonatorGroupComboBoxChangeBool = true;
    }
    sendChangeMessage();

//    init = true;
}

void ControlPanel::refresh (std::shared_ptr<Instrument> currentlyActiveInstrument)
{
    for (auto btn : allButtons)
        btn->setVisible (false);
    
    for (auto cbb : allComboBoxes)
        cbb->setVisible (false);

    for (auto sl : allSliders)
        sl->setVisible (false);

    bool connectionInfoActive = false;
    
    instructionsLabel1->setText ("", dontSendNotification);
    instructionsLabel2->setText ("", dontSendNotification);

    if (currentlyActiveInstrument == nullptr)
    {
        addInstrumentButton->setVisible (true);
        savePresetButton->setVisible (true);
        loadPresetButton->setVisible (true);
    }
    else
    {
        switch (applicationState)
        {
            case normalState:
                addInstrumentButton->setVisible (true);
                addResonatorModuleButton->setVisible (true);
                savePresetButton->setVisible (true);
                loadPresetButton->setVisible (true);

                if (currentlyActiveInstrument->getNumResonatorModules() != 0)
                {
                    editResonatorModuleButton->setVisible (true);
                    editResonatorGroupsButton->setVisible (true);
                    editInOutputsButton->setVisible (true);
                }
                if (currentlyActiveInstrument->getNumResonatorModules() > 1)
                {
                    editConnectionButton->setVisible (true);
                }
                break;
                
            case removeResonatorModuleState:
                editResonatorModuleButton->setVisible (true);
                removeResonatorModuleButton->setVisible (true);
                break;
                
            case editInOutputsState:
                editInOutputsButton->setVisible (true);
                setInstructionsText (Global::inOutInstructions);
//                instructionsLabel->setText (Global::inOutInstructions, dontSendNotification);
                break;

            case editConnectionState:
            case moveConnectionState:
                editDensityButton->setVisible (true);
            case firstConnectionState:
                setInstructionsText (Global::connectionInstructions);
                if (currentlyActiveConnection != nullptr)
                    connectionInfoActive = true;
                
                editConnectionButton->setVisible (true);
                connectionTypeBox->setVisible (true);
                break;
                
            case editDensityState:
                editDensityButton->setVisible (true);
                densitySlider->setVisible (currentlyActiveResonator == nullptr ? false : true);
                setInstructionsText (Global::densityInstructions);
                break;
                
            case editResonatorGroupsState:
                editResonatorGroupsButton->setVisible (true);
                addResonatorGroupButton->setVisible (true);
                removeResonatorGroupButton->setVisible (true);
                resonatorGroupBox->setVisible (true);

                setInstructionsText (Global::groupInstructions);
                break;
        }
    }
    
    connectionLabel->setVisible (connectionInfoActive);
    resized();
}

void ControlPanel::sliderValueChanged (Slider* slider)
{
    if (slider == densitySlider.get())
    {
        curSliderValue = densitySlider->getValue();
        
        action = densitySliderAction;
        sendChangeMessage();
    }
}

void ControlPanel::setInstructionsText (StringArray& instructions)
{
    
    for (int i = 0; i < ceil (0.5 * instructions.size()); ++i)
    {
        instructionsLabel1->setText (instructionsLabel1->getText() + instructions[i], dontSendNotification);
        instructionsLabel1->setText (instructionsLabel1->getText() + "\n", dontSendNotification);
    }
    for (int i = ceil (0.5 * instructions.size()); i < instructions.size(); ++i)
    {
        instructionsLabel2->setText (instructionsLabel2->getText() + instructions[i], dontSendNotification);
        instructionsLabel2->setText (instructionsLabel2->getText() + "\n", dontSendNotification);
    }
}

void ControlPanel::setCurrentlyActiveConnection (Instrument::ConnectionInfo* CI)
{
    currentlyActiveConnection = CI;
    if (currentlyActiveConnection == nullptr)
        return;
    
    setComboBoxId (currentlyActiveConnection->connType);
    refreshConnectionLabel();
}

void ControlPanel::refreshConnectionLabel()
{
    if (currentlyActiveConnection == nullptr)
        return;
    
    auto params = currentlyActiveConnection->getParams();
    connectionLabel->setText (String ("K1 = " + String(params[0]) + " K3 = " + String(params[1]) + "\nR = " + String(params[2])), dontSendNotification);
    
    resized();

}

void ControlPanel::refreshResonatorGroupBox()
{
    resonatorGroupBox->clear();
    for (int i = 1; i <= numGroups; ++i)
        resonatorGroupBox->addItem ("Group " + String (i), i);
    resonatorGroupBox->setSelectedId (numGroups);
}
