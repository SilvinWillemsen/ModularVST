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

    bool didConnectionTypeComboBoxChange() { return connectionTypeComboBoxChangeBool; };
    void setConnectionTypeComboBoxChangeBoolFalse() { connectionTypeComboBoxChangeBool = false; };
    ConnectionType getConnectionType() { return static_cast<ConnectionType> (connectionTypeBox->getSelectedId()); };
    
    double getCurSliderValue() { return curSliderValue; };
    void setDensitySliderValueFromResonator (std::shared_ptr<ResonatorModule> r) {
        setCurrentlyActiveResonator (r);
        densitySlider->setValue ((double)*r->getParameters().getVarPointer("rho"), dontSendNotification);
    };
    void setCurrentlyActiveResonator (std::shared_ptr<ResonatorModule> r) { currentlyActiveResonator = r.get(); }

    void setInstructionsText (StringArray& instructions);
    void setCurrentlyActiveConnection (Instrument::ConnectionInfo* CI);
    void refreshConnectionLabel(); 
    
    int getNumGroups() { return numGroups; };
    void setNumGroups (int n) { numGroups = n; refreshResonatorGroupBox(); };
    bool didResonatorGroupComboBoxChange() { return resonatorGroupComboBoxChangeBool; };
    void setResonatorGroupComboBoxChangeBoolFalse() { resonatorGroupComboBoxChangeBool = false; };
    int getCurrentResonatorGroup() { return resonatorGroupBox->getSelectedId(); };
    void addResonatorGroup() { ++numGroups; refreshResonatorGroupBox(); };
    void removeResonatorGroup () { --numGroups; refreshResonatorGroupBox(); };
    
    void refreshResonatorGroupBox();
private:
    
    // Collection of all buttons
    
    std::vector<std::shared_ptr<TextButton>> allButtons;
    
    std::shared_ptr<TextButton> addInstrumentButton;
    std::shared_ptr<TextButton> addResonatorModuleButton;
    std::shared_ptr<TextButton> editResonatorModuleButton;
    std::shared_ptr<TextButton> removeResonatorModuleButton;
    std::shared_ptr<TextButton> editInOutputsButton;
    std::shared_ptr<TextButton> editConnectionButton;
    std::shared_ptr<TextButton> editDensityButton;
    std::shared_ptr<TextButton> savePresetButton;
    std::shared_ptr<TextButton> loadPresetButton;
    std::shared_ptr<TextButton> editResonatorGroupsButton;
    std::shared_ptr<TextButton> addResonatorGroupButton;
    std::shared_ptr<TextButton> removeResonatorGroupButton;

    std::shared_ptr<Label> instructionsLabel1;
    std::shared_ptr<Label> instructionsLabel2;
    std::shared_ptr<Label> connectionLabel;
    String instructionsText;

    // Collection of all combo boxes
    // 0 - Connection Type
    // 1 - Input Output
    std::vector<std::shared_ptr<ComboBox>> allComboBoxes;
    std::shared_ptr<ComboBox> connectionTypeBox;
    std::shared_ptr<ComboBox> resonatorGroupBox;
    int numGroups = 0;
    
    std::vector<std::shared_ptr<Slider>> allSliders;
    std::shared_ptr<Slider> densitySlider;
    double curSliderValue = 0;

    Action action = noAction;
    ApplicationState applicationState = normalState;
    
    bool connectionTypeComboBoxChangeBool = false;
    bool resonatorGroupComboBoxChangeBool = false;
//    bool init = false;
    
    Instrument::ConnectionInfo* currentlyActiveConnection = nullptr;
    ResonatorModule* currentlyActiveResonator = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
