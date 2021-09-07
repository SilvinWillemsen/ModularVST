/*
  ==============================================================================

    CoefficientList.h
    Created: 5 Sep 2021 1:48:02pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "CoefficientComponent.h"

//==============================================================================
/*
*/
class CoefficientList    : public Component,//public ListBox,
                           public ListBoxModel, public ChangeBroadcaster
{
public:
    CoefficientList (ChangeListener* addModuleWindow);
    ~CoefficientList();

    void paint (Graphics&) override;
    void resized() override;
    
    void repaintAndUpdate();
    
    std::vector<std::shared_ptr<CoefficientComponent>>& getCoefficients() { return coefficients; };
    
    int getNumRows() override { return parameters.size(); };

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected) override;
    
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
           
    std::shared_ptr<CoefficientComponent> addCoefficient (Identifier& name, double value);
    std::shared_ptr<CoefficientComponent> addCoefficient (std::shared_ptr<CoefficientComponent>);

    // if we want to empty the entire vector, we can't erase the vector entries before resetting all shared_ptrs
    void removeCoefficient (std::shared_ptr<CoefficientComponent> coeffToRemove, bool eraseFromVector = true);
    void emptyCoefficientList (bool update = true);

    void loadCoefficientsFromObject (std::vector<std::shared_ptr<CoefficientComponent>>& coefficientsFromObject);

    NamedValueSet getNamedValueSet (StringArray coefficientNames = StringArray());

    void listBoxItemClicked (int row, const MouseEvent&) override;
    void printCoefficients();
    

    NamedValueSet& getParameters() { return parameters; };
    void setParameters (NamedValueSet& p) { parameters = p; };
    void setParameter (double p) { parameters.set (parameters.getName (getSelectedRow()), p); };
    int getSelectedRow() { return list.getSelectedRow(); };
    
    String getSelectedParameter() { return parameters.getName (getSelectedRow()).toString(); };
    String getSelectedParameterValue() { return parameters.getValueAt (getSelectedRow()); };
    void deselect() { list.deselectAllRows(); };
    
    
private:    
    std::vector<std::shared_ptr<CoefficientComponent>> coefficients;
    ListBox list { {}, this };
    NamedValueSet parameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoefficientList)
};

