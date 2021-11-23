/*
  ==============================================================================

    ExcitationPanel.h
    Created: 22 Nov 2021 11:29:36am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"

//==============================================================================
/*
*/
class ExcitationPanel  : public juce::Component,
                        public ChangeBroadcaster,
                        public Button::Listener,
                        public ComboBox::Listener

{
public:
    ExcitationPanel (ChangeListener* audioProcessorEditor);
    ~ExcitationPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    bool getExciteMode() { return exciteMode; };
    ExcitationType getExcitationType() { return static_cast<ExcitationType> (excitationTypeBox->getSelectedId()); };
    Action getAction() { return action; };
    void setAction (Action a) { action = a; };

private:
    std::shared_ptr<ComboBox> excitationTypeBox;
    std::shared_ptr<TextButton> toggleExcitationButton;

    bool exciteMode = false;
    
    Action action = noAction;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExcitationPanel)
};
