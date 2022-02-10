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
#include <JuceHeader.h>
#include <sys/stat.h>
//==============================================================================
/*
*/
class LoadPresetWindow : public juce::Component,
                         public Button::Listener,
                         public ChangeBroadcaster,
                         public ChangeListener,
                         public Timer
{
public:
    LoadPresetWindow(ChangeListener* audioProcessorEditor, ModularVSTAudioProcessor* modularVSTAudioProcessor);
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
    
    String filename;
    std::unique_ptr<TextButton> loadPresetButton;

    juce::ComboBox presetList;
    Action action = noAction;
    ModularVSTAudioProcessor *modularVSTAudioProcessor;
    std::unique_ptr<TextEditor> filenameEditor;

    int dlgPreset = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoadPresetWindow)
};
