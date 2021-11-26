/*
  ==============================================================================

    SavePresetWindow.h
    Created: 25 Nov 2021 2:10:14pm
    Author:  tlasi

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "CoefficientList.h"
//==============================================================================
/*
*/
class SavePresetWindow : public juce::Component,
                         public Button::Listener,
                         public ChangeBroadcaster,
                         public ChangeListener,
                         public TextEditor::Listener
{
public:
    SavePresetWindow(ChangeListener* audioProcessorEditor);
    ~SavePresetWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (Button* button) override;
    void setAction (Action a) { action = a; };
    Action getAction() { return action; };
    
    // Texteditor
    void textEditorTextChanged (TextEditor&) override;
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    int getDlgSave() { return dlgSave; };
    void setDlgSave (int d) { dlgSave = d; };
    
private:
    
    std::unique_ptr<TextButton> savePresetButton;

    Action action = noAction;
    
    std::unique_ptr<TextEditor> valueEditor;
    
    int dlgSave = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavePresetWindow)
};
