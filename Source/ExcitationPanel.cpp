/*
  ==============================================================================

    ExcitationPanel.cpp
    Created: 22 Nov 2021 11:29:36am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ExcitationPanel.h"

//==============================================================================
ExcitationPanel::ExcitationPanel (ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    excitationTypeBox = std::make_shared<ComboBox> ();
    excitationTypeBox->addItem ("Pluck", pluck);
    excitationTypeBox->addItem ("Hammer", hammer);
    excitationTypeBox->addItem ("Bow", bow);


    excitationTypeBox->addListener (this);
    addAndMakeVisible(excitationTypeBox.get());
    if (Global::bowAtStartup)
        excitationTypeBox->setSelectedId (3);
    else
        excitationTypeBox->setSelectedId (1, dontSendNotification);
    
    toggleExcitationButton = std::make_shared<TextButton> ("Excite!");
    toggleExcitationButton->setColour (TextButton::ColourIds::buttonColourId, exciteMode ? Colours::green : Colours::red);
    toggleExcitationButton->addListener (this);
    addAndMakeVisible (toggleExcitationButton.get());
    
    toggleGraphicsButton = std::make_shared<TextButton> ("Graphics");
    toggleGraphicsButton->setColour (TextButton::ColourIds::buttonColourId, graphicsToggle ? Colours::green : Colours::red);
    toggleGraphicsButton->addListener (this);
    if (Global::showGraphicsToggle)
        addAndMakeVisible (toggleGraphicsButton.get());

    if (Global::bowAtStartup)
        buttonClicked (toggleExcitationButton.get());
    else if (Global::pluckAtStartup)
        buttonClicked (toggleExcitationButton.get());
    
    addChangeListener (audioProcessorEditor);

}

ExcitationPanel::~ExcitationPanel()
{
}

void ExcitationPanel::paint (juce::Graphics& g)
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

void ExcitationPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> area = getLocalBounds();
    area.reduce (Global::margin, Global::margin);
    excitationTypeBox->setBounds (area.removeFromTop (Global::buttonHeight));
    if (Global::showGraphicsToggle)
    {
        toggleGraphicsButton->setBounds (area.removeFromBottom (Global::buttonHeight));
        area.removeFromBottom (Global::margin);
    }
    toggleExcitationButton->setBounds (area.removeFromBottom (Global::buttonHeight));
}

void ExcitationPanel::buttonClicked (Button* button)
{
    if (button == toggleExcitationButton.get())
    {
        exciteMode = !exciteMode;
        toggleExcitationButton->setColour(TextButton::ColourIds::buttonColourId,
                                          exciteMode ? Colours::green : Colours::red);
        action = exciteAction;
        sendChangeMessage();
        
    }
    if (button == toggleGraphicsButton.get())
    
    {
        graphicsToggle = !graphicsToggle;
        toggleGraphicsButton->setColour (TextButton::ColourIds::buttonColourId,
                                          graphicsToggle ? Colours::green : Colours::red);
        action = graphicsToggleAction;
        sendChangeMessage();

    }
}

void ExcitationPanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    action = changeExcitationAction;
    sendChangeMessage();
}
