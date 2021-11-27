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

    int getDlgPreset() { return dlgPreset; };
    void setDlgPreset(int d) { dlgPreset = d; };

private:
    std::string filename;
    std::unique_ptr<TextButton> savePresetButton;

    Action action = noAction;

    std::unique_ptr<TextEditor> filenameEditor;

    int dlgPreset = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavePresetWindow)
};