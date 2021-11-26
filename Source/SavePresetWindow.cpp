/*
  ==============================================================================

    SavePresetWindow.cpp
    Created: 25 Nov 2021 2:09:56pm
    Author:  tlasi

  ==============================================================================
*/

#include "SavePresetWindow.h"
#include <JuceHeader.h>

//==============================================================================
SavePresetWindow::SavePresetWindow(ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    savePresetButton = std::make_unique<TextButton> ("Save to file");
    savePresetButton->addListener (this);
    addAndMakeVisible(savePresetButton.get());
    
   // addChangeListener (audioProcessorEditor);
        
    valueEditor = std::make_unique<TextEditor>();
    valueEditor->addListener (this);
    valueEditor->setInputRestrictions (0, "0123456789.eE-");

    addAndMakeVisible (valueEditor.get());

    // initialise to string
    
    setSize (400, 300);
}

SavePresetWindow::~SavePresetWindow()
{
}

void SavePresetWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void SavePresetWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totalArea = getLocalBounds();
    Rectangle<int> buttonArea = totalArea.removeFromBottom(Global::buttonHeight + Global::margin);
    buttonArea.reduce(Global::margin, Global::margin);

    savePresetButton->setBounds(buttonArea.removeFromRight (100));

    totalArea.reduce(Global::margin, Global::margin);
    
    Rectangle<int> labelArea = totalArea.removeFromTop (Global::buttonHeight);
    totalArea.removeFromLeft (Global::margin);
    valueEditor->setBounds (totalArea.removeFromTop (Global::buttonHeight));
}

void SavePresetWindow::buttonClicked (Button* button)
{
    if (button == savePresetButton.get())
    {
        action = savePresetAction;
        DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>();
        std::cout << "Saving Preset" << std::endl;
        dlgSave = 1;
        dw->exitModalState(1);
        action = savePresetFromWindowAction;
        sendChangeMessage();
    }
}

void SavePresetWindow::textEditorTextChanged (TextEditor& TE)
{
    // save the name as a string for filename
    // coefficientList->repaintAndUpdate();
}

void SavePresetWindow::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    valueEditor->setVisible (true);

    valueEditor->setText ("lastSavedPreset");

}
