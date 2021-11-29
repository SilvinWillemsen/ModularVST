/*
  ==============================================================================

    ControlPanel.h
    Created: 4 Sep 2021 1:22:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "Instrument.h"

//==============================================================================
/*
*/

class ControlPanel  :   public juce::Component,
                        public ChangeBroadcaster,
                        public Button::Listener,
                        public ComboBox::Listener,
                        public Slider::Listener
{
public:
    ControlPanel (ChangeListener* audioProcessorEditor);
    ~ControlPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void addInstrument();

    void buttonClicked (Button* button) override;
    
    Action getAction() { return action; };
    void setAction (Action a) { action = a; };
    
    // Determine what buttons are visible (also depending on the currently active instrument)
    void refresh (std::shared_ptr<Instrument> currentlyActiveInstrument);
    
    void setApplicationState (ApplicationState a) { applicationState = a; };
    void setComboBoxId (ConnectionType c) { connectionTypeBox->setSelectedId (c); };
    
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (Slider* slider) override;

    bool didComboBoxChange() { return comboBoxChangeBool; };
    void setComboBoxChangeBoolFalse() { comboBoxChangeBool = false; };
    ConnectionType getConnectionType() { return connectionType; };
    
    double getCurSliderValue() { return curSliderValue; };
    void setMassRatioSliderValue (double val) { massRatioSlider->setValue (val, dontSendNotification); };

    void setInstructionsText (StringArray& instructions);
    void setCurrentlyActiveConnection (Instrument::ConnectionInfo* CI);
    void refreshConnectionLabel(); 
    
private:
    
    // Collection of all buttons
    // 0 - Add Instrument
    // 1 - Add Resonator
    // 2 - Edit Resonator
    // 3 - Remove Resonator
    // 4 - Edit In- Outputs
    // 5 - Edit Connections
    // 6 - Save Preset
    // 6 - Load Preset

    std::vector<std::shared_ptr<TextButton>> allButtons;
    
    std::shared_ptr<TextButton> addInstrumentButton;                // 0
    std::shared_ptr<TextButton> addResonatorModuleButton;           // 1
    std::shared_ptr<TextButton> editResonatorModuleButton;          // 2
    std::shared_ptr<TextButton> removeResonatorModuleButton;        // 3
    std::shared_ptr<TextButton> editInOutputsButton;                // 4
    std::shared_ptr<TextButton> editConnectionButton;               // 5
    std::shared_ptr<TextButton> savePresetButton;                   // 6
    std::shared_ptr<TextButton> loadPresetButton;                   // 7

    std::shared_ptr<Label> instructionsLabel1;
    std::shared_ptr<Label> instructionsLabel2;
    std::shared_ptr<Label> connectionLabel;
    String instructionsText;

    // Collection of all combo boxes
    // 0 - Connection Type
    // 1 - Input Output
    std::vector<std::shared_ptr<ComboBox>> allComboBoxes;
    std::shared_ptr<ComboBox> connectionTypeBox;

    std::vector<std::shared_ptr<Slider>> allSliders;
    std::shared_ptr<Slider> massRatioSlider;
    double curSliderValue = 0;

    Action action = noAction;
    ApplicationState applicationState = normalState;
    
    bool comboBoxChangeBool = false;
//    bool init = false;
    ConnectionType connectionType = rigid;
    
    Instrument::ConnectionInfo* currentlyActiveConnection = nullptr;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
