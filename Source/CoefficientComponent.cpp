/*
  ==============================================================================

    CoefficientComponent.cpp
    Created: 5 Sep 2021 1:48:14pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CoefficientComponent.h"

CoefficientComponent::CoefficientComponent (const String& name, double val) : value (val)
{
    this->setName (name);
    // labels
    label.setFont (Font("Latin Modern Math", "Regular", 16.0));
    label.setColour (Label::textColourId, Colours::white);
    
    addAndMakeVisible (label);
    valueEditor = std::make_unique<TextEditor>();
    valueEditor->setText (String(value));
    addAndMakeVisible (valueEditor.get());
    update (true);
    
}

CoefficientComponent::~CoefficientComponent()
{
    std::cout << getName() << " is removed from the heap." << std::endl;
}

void CoefficientComponent::paint (Graphics& g)
{
    g.fillAll (Colours::grey);
}

void CoefficientComponent::resized()
{
    Rectangle<int> totalArea = getLocalBounds();
    totalArea.reduce (Global::margin * 0.5, Global::margin / 2.0);
    
//    coefficientButton.setBounds (totalArea.removeFromLeft (Global::buttonWidth));
//    removeButton.setBounds (totalArea.removeFromRight (Global::buttonHeight));
//    totalArea.removeFromRight (Global::margin);
//    editButton.setBounds (totalArea.removeFromRight (Global::buttonHeight));
//    totalArea.removeFromLeft (Global::margin);
//    totalArea.removeFromRight (Global::margin);
//
    label.setBounds (totalArea.removeFromLeft(getWidth() * 0.5));
    valueEditor->setBounds (totalArea);

    
}

void CoefficientComponent::buttonClicked (Button* button)
{
//    if (button == &coefficientButton)
//    {
//        action = insertCoeff;
//    }
//    if (button == &editButton)
//    {
//        action = editCoeff;
//    }
//    if (button == &removeButton)
//    {
//        action = removeCoeff;
//    }
    
    sendChangeMessage();
}

void CoefficientComponent::update (bool init)
{
    std::cout << getName() << std::endl;
    label.setText (getName(), sendNotification);
    resized();
}

void CoefficientComponent::mouseDown (const MouseEvent& e)
{
    std::cout << getName() << std::endl;
}
