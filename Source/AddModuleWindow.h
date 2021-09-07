/*
  ==============================================================================

    AddModuleWindow.h
    Created: 4 Sep 2021 3:25:48pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "CoefficientList.h"
//==============================================================================
/*
*/
class AddModuleWindow  : public juce::Component,
                         public Button::Listener,
                         public ChangeBroadcaster,
                         public ChangeListener,
                         public ComboBox::Listener,
                         public TextEditor::Listener
{
public:
    AddModuleWindow (ChangeListener* audioProcessorEditor);
    ~AddModuleWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (Button* button) override;
    void setAction (Action a) { action = a; };

    ResonatorModuleType getResonatorModuleType() { return resonatorModuleType; };
    NamedValueSet& getParameters() { return coefficientList->getParameters(); };
    
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    
    // Texteditor
    void textEditorReturnKeyPressed (TextEditor&) override;
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
private:
    
    std::unique_ptr<TextButton> addModuleButton;
    std::unique_ptr<TextButton> testButton;
    std::unique_ptr<ComboBox> resonatorTypeBox;
    std::unique_ptr<CoefficientList> coefficientList;
    std::unique_ptr<Label> coeffTopLabel;
    std::unique_ptr<Label> valueLabel;

    Action action = noAction;
    ResonatorModuleType resonatorModuleType;
    
    std::unique_ptr<TextEditor> valueEditor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AddModuleWindow)
};
