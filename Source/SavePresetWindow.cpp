/*
  ==============================================================================
    SavePresetWindow.cpp
    Created: 25 Nov 2021 2:09:56pm
    Author:  tlasi
  ==============================================================================
*/

#include "SavePresetWindow.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <sys/stat.h>

//==============================================================================
SavePresetWindow::SavePresetWindow(ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    savePresetButton = std::make_unique<TextButton> ("Save to file");
    savePresetButton->addListener (this);
    addAndMakeVisible(savePresetButton.get());

   // addChangeListener (audioProcessorEditor);

    filenameEditor = std::make_unique<TextEditor>();
    filenameEditor->addListener (this);
    filenameEditor->setInputRestrictions (0, "qwertyuiopasdfghjklzxcvbnm1234567890_QWERTYUIOPASDFGHJKLZXCVBNM");

    addAndMakeVisible (filenameEditor.get());
    
    addChangeListener (audioProcessorEditor);

    // initialise to string

    setSize (400, 300);
}

SavePresetWindow::~SavePresetWindow()
{
}



void SavePresetWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.
       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void SavePresetWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totalArea = getLocalBounds();
    Rectangle<int> buttonArea = totalArea.removeFromBottom(Global::buttonHeight + Global::margin);
    buttonArea.reduce(Global::margin, Global::margin);

    savePresetButton->setBounds(buttonArea.removeFromRight (100));

    totalArea.reduce(Global::margin, Global::margin);

    Rectangle<int> labelArea = totalArea.removeFromTop (Global::buttonHeight);
    totalArea.removeFromLeft (Global::margin);
    filenameEditor->setBounds (totalArea.removeFromTop (Global::buttonHeight));
}

void SavePresetWindow::buttonClicked (Button* button)
{
    if (button == savePresetButton.get())
    {
        ModularVSTAudioProcessor proces;
        action = savePresetAction;
        DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>();
        std::cout << "Saving Preset" << std::endl;
        dlgPreset = 1;
        dw->exitModalState(1);
        String name = proces.getPresetPath() + filename + ".xml";
        const char* c = name.toUTF8();

        struct stat buffer;
        auto status(stat(c, &buffer) == 0);
        // If file with this name already exists...
        if (status) 
        {
            String message = "File with a name \"" + filename + ".xml\" already exists, would you like to overwrite existing preset?";
            //AlertWindow("File with this name exists", message, "QuestionIcon");
            NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType::QuestionIcon, "File with this name exists", message, this, ModalCallbackFunction::create ([&] (int r)                                                         {
                            if (r == 0)
                            {
                                String message = "Preset has not been saved";
                                NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::QuestionIcon, "Preset not saved", message, nullptr);
                            } else if (r == 1)
                            {
                                action = savePresetFromWindowAction;
                                sendChangeMessage();
                            }
                                
            }));
        }
        else
        // Otherwise, save preset
        {
            action = savePresetFromWindowAction;
            sendChangeMessage();
        }
        juce::Logger::getCurrentLogger()->outputDebugString("Debug");
    }
}

void SavePresetWindow::textEditorTextChanged (TextEditor& TE)
{
    // save the name as a string for filename
    // coefficientList->repaintAndUpdate();
    auto value = TE.getTextValue().toString();
     
    filename = value.String::toStdString();
}

void SavePresetWindow::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    filenameEditor->setVisible (true);
    filenameEditor->setText ("lastSavedPreset");

}
