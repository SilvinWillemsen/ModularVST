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
        
    int getNumRows() override { return parameters.size(); };

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected) override;
    
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
           
//    void listBoxItemClicked (int row, const MouseEvent&) override;
    void printParameterNames();
    void selectedRowsChanged (int lastRowSelected) override;

    NamedValueSet& getParameters() { return parameters; };
    void setParameters (const NamedValueSet& p) { parameters = p; };
    void setParameter (double p) { parameters.set (parameters.getName (getSelectedRow()), p); };
    int getSelectedRow() { return list.getSelectedRow(); };
    
    String getSelectedParameter() {
        return (getSelectedRow() == -1) ? "?" : parameters.getName (getSelectedRow()).toString();
        
    };
    String getSelectedParameterValue();
    void deselect() { list.deselectAllRows(); };
    
    
private:    
    ListBox list { {}, this };
    NamedValueSet parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoefficientList)
};

