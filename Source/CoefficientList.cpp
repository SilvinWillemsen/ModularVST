/*
  ==============================================================================

    CoefficientList.cpp
    Created: 5 Sep 2021 1:48:02pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CoefficientList.h"

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
    for (auto coefficient : coefficients)
        removeCoefficient (coefficient);
    
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
    g.drawText (parameters.getName (rowNumber).toString() + " = " + parameters.getValueAt (rowNumber).toString(),
                Global::margin,
                0,
                width * 0.5,
                height, Justification::centredLeft);
}

Component* CoefficientList::refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
//    if (rowNumber < coefficients.size())
//    {
////        for (int i = 0; i < coefficients.size(); ++i)
////            if (existingComponentToUpdate == coefficients[i].get())
////                return existingComponentToUpdate;
//
//        return coefficients[rowNumber].get();
//    }
    return nullptr;
}

std::shared_ptr<CoefficientComponent> CoefficientList::addCoefficient (Identifier& name, double value)
{
    coefficients.push_back (std::make_shared<CoefficientComponent> (name.toString(), value));
    std::shared_ptr<CoefficientComponent> newCoeff = coefficients[coefficients.size() - 1];
    addAndMakeVisible (newCoeff.get());
    return newCoeff;
}

std::shared_ptr<CoefficientComponent> CoefficientList::addCoefficient (std::shared_ptr<CoefficientComponent> coefficient)
{
    coefficients.push_back (coefficient);
    std::shared_ptr<CoefficientComponent> newCoeff = coefficients[coefficients.size() - 1];
    addAndMakeVisible (newCoeff.get());
    return newCoeff;
}

void CoefficientList::removeCoefficient (std::shared_ptr<CoefficientComponent> coeffToRemove, bool eraseFromVector)
{
    for (int i = 0; i < coefficients.size(); ++i)
        if (coefficients[i] == coeffToRemove)
        {
            coefficients[i]->setVisible (false);
            coefficients[i].reset();
            
            if (eraseFromVector)
                coefficients.erase(coefficients.begin() + i);
        }
}

void CoefficientList::emptyCoefficientList (bool update)
{
    for (auto coeff : coefficients)
        removeCoefficient (coeff, false);
    
    for (int i = static_cast<int> (coefficients.size() - 1); i >= 0; --i)
        coefficients.erase(coefficients.begin() + i);
    
    if (update)
        repaintAndUpdate();
}

void CoefficientList::loadCoefficientsFromObject (std::vector<std::shared_ptr<CoefficientComponent>>& coefficientsFromObject)
{
    emptyCoefficientList (false);
    for (auto coeff : coefficientsFromObject)
    {
        coeff->setVisible (true);
        addCoefficient (coeff);
    }
    repaintAndUpdate();
}

NamedValueSet CoefficientList::getNamedValueSet (StringArray coefficientNames)
{
    NamedValueSet namedValueSet;
    for (auto coeff : coefficients)
        if (coefficientNames.isEmpty()) // if the coefficientNames vector is empty, return all coefficients
            namedValueSet.set (coeff.get()->getName(), coeff.get()->getValue());
        else if (coefficientNames.contains (coeff.get()->getName()))
            namedValueSet.set (coeff.get()->getName(), coeff.get()->getValue());
    
    return namedValueSet;
}

void CoefficientList::printCoefficients()
{
    for (auto coeff : coefficients)
        std::cout << coeff->getLabelName() << std::endl;
}


//void CoefficientList::setParameters (NamedValueSet& p)
//{
//    parameters = p;
//    valueEditors.clear();
//    for (int i = 0; i < p.size(); ++i)
//    {
//        valueEditors.add (new TextEditor());
//        valueEditors[i]->addListener (this);
//        addAndMakeVisible (valueEditors[i]);
//        valueEditors[i]->setText (parameters.getValueAt(i).toString());
//    }
//}

void CoefficientList::listBoxItemClicked (int row, const MouseEvent& e)
{
    sendChangeMessage();
}
