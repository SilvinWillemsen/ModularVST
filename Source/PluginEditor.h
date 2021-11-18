/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ControlPanel.h"
#include "AddModuleWindow.h"
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
    
    void buttonClicked (Button* button) override;

    void timerCallback() override;
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;

    void refresh();
    
    void openAddModuleWindow();
    
    void refreshControlPanel();

    void setApplicationState (ApplicationState applicationState);
    
//    void retrieveDlgModal();
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ModularVSTAudioProcessor& audioProcessor;
    TextButton addChannelButton;
    
    std::vector<std::shared_ptr<Instrument>>* instruments;
    std::unique_ptr<ControlPanel> controlPanel;
    
    std::unique_ptr<AddModuleWindow> addModuleWindow;
    
    ApplicationState applicationState;
    DialogWindow* dlgWindow;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessorEditor)
};
