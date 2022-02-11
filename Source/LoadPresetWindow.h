/*
  ==============================================================================
    LoadPresetWindow.h
    Created: 10 Feb 2022 2:58:42pm
    Author:  tlasi
  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "CoefficientList.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>
#include <sys/stat.h>
//==============================================================================
/*
*/
class LoadPresetWindow : public juce::Component,
                         public Button::Listener,
                         public ChangeBroadcaster,
                         public ChangeListener
{
public:
    LoadPresetWindow(ChangeListener* audioProcessorEditor);
    ~LoadPresetWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (Button* button) override;
    void setAction (Action a) { action = a; };
    Action getAction() { return action; };
    void presetListChanged();
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    int getDlgPreset() { return dlgPreset; };
    void setDlgPreset(int d) { dlgPreset = d; };

    String& getFileName() { return filename; };
private: 
    int selectedBinaryPreset;
    String filename;
    std::unique_ptr<TextButton> loadPresetButton;

    juce::ComboBox presetList;
    Action action = noAction;
 
    
    int dlgPreset = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoadPresetWindow)
};
