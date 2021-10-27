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

class ControlPanel  : public juce::Component, public ChangeBroadcaster, public Button::Listener, public ComboBox::Listener
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
    
    void refresh (std::vector<std::shared_ptr<Instrument>>* instruments, int currentlyActiveInstrument);
    
    void toggleAddResonatorButton (bool t) { addResonatorModuleButton->setEnabled (t); };
    void toggleRemoveResonatorButton (bool t) { removeResonatorModuleButton->setEnabled (t); };
    void toggleEditInOutputsButton (bool t) { editInOutputsButton->setEnabled (t); };
    void toggleEditConnectionButton (bool t) { editConnectionButton->setEnabled (t); };
    void toggleAddInstrumentButton (bool t) { addInstrumentButton->setEnabled (t); };
    void toggleConnectionTypeBox (bool t) { connectionTypeBox->setEnabled (t); };

    void setApplicationState (ApplicationState a) { applicationState = a; };
    
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    
    bool didComboBoxChange() { return comboBoxChangeBool; };
    void setComboBoxChangeBoolFalse() { comboBoxChangeBool = false; };
    ConnectionType getConnectionType() { return connectionType; };
    
private:
    
    // Collection of all buttons
    // 0 - Add Instrument
    // 1 - Add Resonator
    // 2 - Remove Resonator
    // 3 - Edit In- Outputs
    // 4 - Edit Connections
    // 5 - Save Preset

    std::vector<std::shared_ptr<TextButton>> allButtons;
    
    std::shared_ptr<TextButton> addInstrumentButton;
    std::shared_ptr<TextButton> addResonatorModuleButton;
    std::shared_ptr<TextButton> removeResonatorModuleButton;
    std::shared_ptr<TextButton> editInOutputsButton;
    std::shared_ptr<TextButton> editConnectionButton;
    std::shared_ptr<TextButton> savePresetButton;
    
    // Collection of all combo boxes
    // 0 - Connection Type
    // 1 - Input Output
    std::vector<std::shared_ptr<ComboBox>> allComboBoxes;
    std::shared_ptr<ComboBox> connectionTypeBox;

    Action action = noAction;
    ApplicationState applicationState = normalState;
    
    bool comboBoxChangeBool = false;
//    bool init = false;
    ConnectionType connectionType = rigid;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
