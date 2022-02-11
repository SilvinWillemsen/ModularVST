/*
  ==============================================================================
    LoadPresetWindow.cpp
    Created: 10 Feb 2022 2:58:42pm
    Author:  tlasi
  ==============================================================================
*/

#include "LoadPresetWindow.h"
#include <JuceHeader.h>


//==============================================================================
LoadPresetWindow::LoadPresetWindow (ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    loadPresetButton = std::make_unique<TextButton> ("Load preset");
    loadPresetButton->addListener (this);
    addAndMakeVisible (loadPresetButton.get());

    loadFromFileButton = std::make_unique<TextButton>("Load preset from file");
    loadFromFileButton->addListener(this);
    addAndMakeVisible(loadFromFileButton.get());

    presetList = std::make_unique<ComboBox> ();
    addAndMakeVisible (presetList.get());

    addChangeListener (audioProcessorEditor);
    int n = BinaryData::namedResourceListSize;
    


    for (int i = 0; i < n; i++) {
        std::string st = BinaryData::namedResourceList[i];
        Logger::getCurrentLogger()->outputDebugString(st.substr(0, st.size() - 4));
        std::string stShort = st.substr(0, st.size() - 4);

        const char* c = stShort.c_str();
        

        presetList->addItem(c, i + 1);
    }
    presetList->onChange = [this] { presetListChanged(); };
    presetList->setSelectedId (1);


    // initialise to string

    setSize (400, 300);
}

LoadPresetWindow::~LoadPresetWindow()
{
}

void LoadPresetWindow::presetListChanged()
{
    selectedBinaryPreset = presetList->getSelectedId() - 1;
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
    loadFromFileButton->setBounds(buttonArea.removeFromRight(100));

    totalArea.reduce(Global::margin, Global::margin);

    Rectangle<int> labelArea = totalArea.removeFromTop (Global::buttonHeight);
    totalArea.removeFromLeft (Global::margin);
    presetList->setBounds (totalArea.removeFromTop (Global::buttonHeight));
}


void LoadPresetWindow::buttonClicked (Button* button)
{
//    ModularVSTAudioProcessorEditor* modularVSTAudioProcessorEditor;
//    modularVSTAudioProcessorEditor->loadPresetFromWindow(selectedBinaryPreset);
    if (button == loadPresetButton.get()) 
    {
        action = loadBinaryPresetAction;
        sendChangeMessage();
    }
    else if (button == loadFromFileButton.get())
    {
        // load from file
        Logger::getCurrentLogger()->outputDebugString("load preset from file");
        action = loadPresetFromWindowAction;
        sendChangeMessage();
    }

    
}

