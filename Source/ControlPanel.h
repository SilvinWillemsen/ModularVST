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
    
    void refresh();
    
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
    
    std::unique_ptr<TextButton> addInstrumentButton;
    std::unique_ptr<TextButton> addResonatorModuleButton;
    std::unique_ptr<TextButton> removeResonatorModuleButton;
    std::unique_ptr<TextButton> editInOutputsButton;

    std::unique_ptr<TextButton> editConnectionButton;
    std::unique_ptr<ComboBox> connectionTypeBox;
    std::unique_ptr<TextButton> savePresetButton;

    Action action = noAction;
    ApplicationState applicationState = normalState;
    
    bool comboBoxChangeBool = false;
//    bool init = false;
    ConnectionType connectionType = rigid;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
