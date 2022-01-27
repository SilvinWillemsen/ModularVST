/*
  ==============================================================================

    CoefficientList.cpp
    Created: 5 Sep 2021 1:48:02pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CoefficientList.h"
#include <iostream>
#include <iomanip>

//==============================================================================
CoefficientList::CoefficientList (ChangeListener* addModuleWindow)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible (list);
    
    list.setColour (ListBox::backgroundColourId, getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    list.setRowHeight (Global::listBoxRowHeight);
    addChangeListener (addModuleWindow);
}

CoefficientList::~CoefficientList()
{
    repaintAndUpdate();
}

void CoefficientList::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

//    g.fillAll (Colours::yellow);   // clear the background
//    if (numRows != 0)
//        for (int i = 0; i < numRows; ++i)
//            paintListBoxItem (i, g, GUIDefines::buttonAreaWidth, GUIDefines::buttonHeight, false);
}

void CoefficientList::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    list.setBounds (getLocalBounds());
}

void CoefficientList::repaintAndUpdate()
{
    repaint();
    list.updateContent();
}

void CoefficientList::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{

    auto alternateColour = getLookAndFeel().findColour (ListBox::backgroundColourId)
    .interpolatedWith (getLookAndFeel().findColour (ListBox::textColourId), 0.03f);
    auto alternateColour2 = getLookAndFeel().findColour (ListBox::backgroundColourId)
    .interpolatedWith (Colours::darkgrey, 0.03f);
//    g.fillAll (Colour::fromRGB(0, rowNumber * 10, 0));
    if (rowNumber % 2)
        g.fillAll (alternateColour);
    else
        g.fillAll (alternateColour2);
    
    if (rowNumber == getSelectedRow())
        g.fillAll (Colours::blue);
    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    std::ostringstream decimalValue;
    double value = parameters.getValueAt (rowNumber);
    if (value < 1)
        decimalValue << std::setprecision (3) << value;
    else
        decimalValue << value;
    
    g.drawText (parameters.getName (rowNumber).toString() + " = " + decimalValue.str(),
                Global::margin,
                0,
                width * 0.5,
                height, Justification::centredLeft);
}

Component* CoefficientList::refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
    return nullptr;
}

void CoefficientList::printParameterNames()
{
    for (int i = 0; i < parameters.size(); ++i)
        std::cout << parameters.getName(i).toString() << std::endl;
}

void CoefficientList::listBoxItemClicked (int row, const MouseEvent& e)
{
    sendChangeMessage();
}

String CoefficientList::getSelectedParameterValue()
{
    std::ostringstream decimalValue;
    double value = parameters.getValueAt (getSelectedRow());
    decimalValue << std::setprecision (3) << value;
        
    return decimalValue.str();
}
