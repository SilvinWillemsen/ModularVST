/*
  ==============================================================================
    LoadPresetWindow.cpp
    Created: 10 Feb 2022 2:58:42pm
    Author:  tlasi
  ==============================================================================
*/

#include "LoadPresetWindow.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <sys/stat.h>


//==============================================================================
LoadPresetWindow::LoadPresetWindow(ChangeListener* audioProcessorEditor, ModularVSTAudioProcessor* modularVSTAudioProcessor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    loadPresetButton = std::make_unique<TextButton> ("Load preset");
    loadPresetButton->addListener (this);
    addAndMakeVisible (loadPresetButton.get());

    filenameEditor = std::make_unique<TextEditor> ();
    addAndMakeVisible (filenameEditor.get());
   // addChangeListener (audioProcessorEditor);
    int n = modularVSTAudioProcessor->getNumPresets();
    for (int i = 0; i < n; i++) {
        presetList.addItem(BinaryData::namedResourceList[i], i+1);
    }
    presetList.onChange = [this] { presetListChanged(); };
    presetList.setSelectedId(1);
    addAndMakeVisible(presetList);


    // initialise to string

    setSize (400, 300);
}

LoadPresetWindow::~LoadPresetWindow()
{
}

void LoadPresetWindow::presetListChanged()
{
    switch (presetList.getSelectedId())
    {
        /*case 1: textFont.setStyleFlags(juce::Font::plain);  break;                               make it call editor to load selected preset from the binary
        case 2: textFont.setStyleFlags(juce::Font::bold);   break;
        case 3: textFont.setStyleFlags(juce::Font::italic); break;*/
    default: break;
    }
}

void LoadPresetWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.
       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void LoadPresetWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totalArea = getLocalBounds();
    Rectangle<int> buttonArea = totalArea.removeFromBottom(Global::buttonHeight + Global::margin);
    buttonArea.reduce(Global::margin, Global::margin);

    loadPresetButton->setBounds(buttonArea.removeFromRight (100));

    totalArea.reduce(Global::margin, Global::margin);

    Rectangle<int> labelArea = totalArea.removeFromTop (Global::buttonHeight);
    totalArea.removeFromLeft (Global::margin);
    filenameEditor->setBounds (totalArea.removeFromTop (Global::buttonHeight));
}


void LoadPresetWindow::buttonClicked (Button* button)
{

}


void LoadPresetWindow::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{

}
