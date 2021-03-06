/*
  ==============================================================================

    AddModuleWindow.cpp
    Created: 4 Sep 2021 3:25:48pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AddModuleWindow.h"

//==============================================================================
AddModuleWindow::AddModuleWindow (ChangeListener* audioProcessorEditor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addModuleButton = std::make_unique<TextButton> ("Add Module");
    addModuleButton->addListener (this);
    addAndMakeVisible(addModuleButton.get());
    
    advancedSettingsButton = std::make_unique<TextButton> ("Advanced");
    advancedSettingsButton->addListener (this);
    addAndMakeVisible(advancedSettingsButton.get());

    resonatorTypeBox = std::make_unique<ComboBox> ("Resonator Type");
    resonatorTypeBox->addListener (this);
    addAndMakeVisible(resonatorTypeBox.get());
    
    resonatorTypeBox->addItem ("Stiff String", stiffString);
    resonatorTypeBox->addItem ("Bar", bar);
    resonatorTypeBox->addItem ("Membrane", membrane);
    resonatorTypeBox->addItem ("Thin Plate", thinPlate);
    resonatorTypeBox->addItem ("Stiff Membrane", stiffMembrane);
    resonatorTypeBox->setSelectedId (1);
    
    addChangeListener (audioProcessorEditor);
        
    valueEditor = std::make_unique<TextEditor>();
    valueEditor->addListener (this);
    valueEditor->setInputRestrictions (0, "0123456789.eE-");

    addAndMakeVisible (valueEditor.get());

    coeffTopLabel = std::make_unique<Label>();
    coeffTopLabel->setColour (Label::textColourId, Colours::white);
    coeffTopLabel->setText ("Coefficients", dontSendNotification);
    coeffTopLabel->setFont (Font (18.0f));
    addAndMakeVisible (coeffTopLabel.get());

    valueLabel = std::make_unique<Label>();
    valueLabel->setColour (Label::textColourId, Colours::white);
    valueLabel->setFont (Font (18.0f));
    addAndMakeVisible (valueLabel.get());

    coefficientList = std::make_unique<CoefficientList> (this);
    addAndMakeVisible (coefficientList.get());
    
    // initialise to string
    resonatorModuleType = stiffString;
    coefficientList->setParameters (showAdvanced ? Global::defaultStringParametersAdvanced : Global::defaultStringParameters);
    
    setSize (400, 300);
}

AddModuleWindow::~AddModuleWindow()
{
}

void AddModuleWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void AddModuleWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totalArea = getLocalBounds();
    Rectangle<int> buttonArea = totalArea.removeFromBottom(Global::buttonHeight + Global::margin);
    buttonArea.reduce(Global::margin, Global::margin);

    addModuleButton->setBounds(buttonArea.removeFromRight (100));
    advancedSettingsButton->setBounds(buttonArea.removeFromLeft (100));

    totalArea.reduce(Global::margin, Global::margin);
    resonatorTypeBox->setBounds (totalArea.removeFromTop (Global::buttonHeight));
    
    Rectangle<int> labelArea = totalArea.removeFromTop (Global::buttonHeight);
    coeffTopLabel->setBounds (labelArea.removeFromLeft (labelArea.getWidth() * 0.66));
    labelArea.removeFromLeft (Global::margin);
    valueLabel->setBounds (labelArea);
    coefficientList->setBounds (totalArea.removeFromLeft(totalArea.getWidth() * 0.66));
    totalArea.removeFromLeft (Global::margin);
    valueEditor->setBounds (totalArea.removeFromTop (Global::buttonHeight));
}

void AddModuleWindow::buttonClicked (Button* button)
{
    if (button == addModuleButton.get())
    {
        action = addResonatorModuleAction;
        DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>();
        std::cout << "Adding module" << std::endl;
        dlgModal = 1;
        dw->exitModalState (1);
        action = addResonatorModuleFromWindowAction;
        sendChangeMessage();
    }
    else if (button == advancedSettingsButton.get())
    {
        showAdvanced = !showAdvanced;
        triggerComboBox();
        coefficientList->repaintAndUpdate();
    }
}

void AddModuleWindow::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    resonatorModuleType = static_cast<ResonatorModuleType>(comboBoxThatHasChanged->getSelectedId());
    switch (resonatorModuleType)
    {
        case stiffString:
            coefficientList->setParameters (showAdvanced ? Global::defaultStringParametersAdvanced : Global::defaultStringParameters);
            break;
        case bar:
            coefficientList->setParameters (showAdvanced ? Global::defaultBarParametersAdvanced : Global::defaultBarParameters);
            break;
        case membrane:
            coefficientList->setParameters (showAdvanced ? Global::defaultMembraneParametersAdvanced : Global::defaultMembraneParameters);
            break;
        case thinPlate:
            coefficientList->setParameters (showAdvanced ? Global::defaultThinPlateParametersAdvanced : Global::defaultThinPlateParameters);
            break;
        case stiffMembrane:
            coefficientList->setParameters(showAdvanced ? Global::defaultStiffMembraneParametersAdvanced : Global::defaultStiffMembraneParameters);
            break;
        // add other cases as well
        default:
            break;
    }
    valueLabel->setVisible (false);
    valueEditor->setVisible (false);
    coefficientList->deselect();
    coefficientList->repaintAndUpdate();
}

void AddModuleWindow::textEditorTextChanged (TextEditor& TE)
{
    coefficientList->setParameter (TE.getTextValue().toString().getDoubleValue());
    coefficientList->repaintAndUpdate();
}

void AddModuleWindow::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    valueLabel->setVisible (true);
    valueEditor->setVisible (true);
    if (changeBroadcaster == coefficientList.get())
    {
        if(Global::fullNames.contains(coefficientList->getSelectedParameter()))
        {
            var text = (*Global::fullNames.getVarPointer(coefficientList->getSelectedParameter()));
            
            valueLabel->setText(text.toString(), dontSendNotification);
        } else {
            if (coefficientList->getSelectedParameter() == "?")
                valueLabel->setText("", dontSendNotification);
            else
                valueLabel->setText ("Value of: " + coefficientList->getSelectedParameter(), dontSendNotification);
        }
        valueEditor->setText (coefficientList->getSelectedParameterValue(), dontSendNotification);

    }
}
