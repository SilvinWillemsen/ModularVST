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
    addInstrumentButton = std::make_unique<TextButton> ("Add Instrument");
    addInstrumentButton->addListener (this);
    addAndMakeVisible(addInstrumentButton.get());

    addResonatorModuleButton = std::make_unique<TextButton> ("Add Resonator Module");
    addResonatorModuleButton->addListener (this);
    addAndMakeVisible(addResonatorModuleButton.get());
    
    removeResonatorModuleButton = std::make_unique<TextButton> ("Remove Resonator Module");
    removeResonatorModuleButton->addListener (this);
    addAndMakeVisible(removeResonatorModuleButton.get());
    
    editInOutputsButton = std::make_unique<TextButton> ("Edit In- Outputs");
    editInOutputsButton->addListener (this);
    addAndMakeVisible(editInOutputsButton.get());

    editConnectionButton = std::make_unique<TextButton> ("Edit Connections");
    editConnectionButton->addListener (this);
    addAndMakeVisible(editConnectionButton.get());
    
    connectionTypeBox = std::make_unique<ComboBox> ();
    connectionTypeBox->addListener (this);
    addAndMakeVisible(connectionTypeBox.get());
    connectionTypeBox->addItem ("Rigid", rigid);
    connectionTypeBox->addItem ("Linear Spring", linearSpring);
    connectionTypeBox->addItem ("Nonlinear spring", nonlinearSpring);
    connectionTypeBox->setSelectedId (1);
    
    addChangeListener (audioProcessorEditor);
    
    savePresetButton = std::make_unique<TextButton> ("Save Preset");
    savePresetButton->addListener (this);
    addAndMakeVisible(savePresetButton.get());
    
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
    addInstrumentButton->setBounds (area.removeFromLeft (100));
    area.removeFromLeft (Global::margin);
    addResonatorModuleButton->setBounds (area.removeFromLeft (100));
    area.removeFromLeft (Global::margin);
    editInOutputsButton->setBounds (area.removeFromLeft (100));
    area.removeFromLeft (Global::margin);
    editConnectionButton->setBounds (area.removeFromLeft (100));
    area.removeFromLeft (Global::margin);
    connectionTypeBox->setBounds (area.removeFromLeft (100));
    
    savePresetButton->setBounds (area.removeFromRight (100));

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
