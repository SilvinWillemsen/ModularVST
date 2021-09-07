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

class ControlPanel  : public juce::Component, public ChangeBroadcaster, public Button::Listener
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
    void toggleAddConnectionButton (bool t) { addConnectionButton->setEnabled (t); };
    void toggleAddInstrumentButton (bool t) { addInstrumentButton->setEnabled (t); };

    void setApplicationState (ApplicationState a) { applicationState = a; };
    
private:
    
    std::unique_ptr<TextButton> addInstrumentButton;
    std::unique_ptr<TextButton> addResonatorModuleButton;
    std::unique_ptr<TextButton> addConnectionButton;

    Action action = noAction;
    ApplicationState applicationState = normalState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
