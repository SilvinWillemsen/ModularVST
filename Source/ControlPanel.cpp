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

    addConnectionButton = std::make_unique<TextButton> ("Add Connection");
    addConnectionButton->addListener (this);
    addAndMakeVisible(addConnectionButton.get());

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

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("ControlPanel", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
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
    addConnectionButton->setBounds (area.removeFromLeft (100));

}

void ControlPanel::buttonClicked (Button* button)
{
    if (button == addInstrumentButton.get())
        action = addInstrumentAction;
    else if (button == addResonatorModuleButton.get())
        action = addResonatorModuleAction;
    else if (button == addConnectionButton.get())
    {
        if (applicationState == normalState)
        {
            action = addConnectionAction;
            addConnectionButton->setButtonText ("Done");
        }
        else if (applicationState == addConnectionState)
        {
            action = cancelConnectionAction;
            addConnectionButton->setButtonText ("Add Connection");
        }
    }
    sendChangeMessage();
}
