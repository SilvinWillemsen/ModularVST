/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ControlPanel.h"
#include "ExcitationPanel.h"
#include "AddModuleWindow.h"
#include "SavePresetWindow.h"
//==============================================================================
/**
*/
class ModularVSTAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public Button::Listener,
                                        public Timer,
                                        public ChangeListener
{
public:
    ModularVSTAudioProcessorEditor (ModularVSTAudioProcessor&);
    ~ModularVSTAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Callbacks
    void buttonClicked (Button* button) override;
    void timerCallback() override;
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;

    // Refresh
    void refresh();
    
    // Windows
    void openAddModuleWindow();
    void openSavePresetWindow();
    void openLoadPresetWindow();

    // Set application state and "rain it down" to all other components
    void setApplicationState (ApplicationState applicationState);
        
//    std::function<void (const FileChooser&)> loadPresetCallBack (FileChooser& fileChooser);
private:
    // Reference to the audio processor
    ModularVSTAudioProcessor& audioProcessor;

    // Reference to the instruments
    std::vector<std::shared_ptr<Instrument>>& instruments;
    std::shared_ptr<Instrument> currentlyActiveInstrument = nullptr;
    
    // Various GUI panels
    std::unique_ptr<ControlPanel> controlPanel;
    std::unique_ptr<ExcitationPanel> excitationPanel;

    // Window for adding modules
    std::unique_ptr<AddModuleWindow> addModuleWindow;
    std::unique_ptr<SavePresetWindow> savePresetWindow;
    DialogWindow* dlgWindow;
    std::unique_ptr<FileChooser> loadPresetWindow;

    // State of the application
    ApplicationState applicationState;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessorEditor)
};
