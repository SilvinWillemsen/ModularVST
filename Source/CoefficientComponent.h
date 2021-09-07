/*
  ==============================================================================

    CoefficientComponent.h
    Created: 5 Sep 2021 1:48:14pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"

//==============================================================================
/*
*/

class CoefficientComponent    : public Component,
                                public Button::Listener,
                                public ChangeBroadcaster
{
public:
    CoefficientComponent (const String& name, double value);
    ~CoefficientComponent();

    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;

    void update (bool init = false);
    
    Action getAction() { return action; }
    
    double getValue() { return value; };
        
    void mouseDown (const MouseEvent& e) override;
    
    String getLabelName() { return label.getText(); };
private:
//    String name;
    double value;
    
    Label label;
    
    std::unique_ptr<TextEditor> valueEditor;
        
    Action action = noAction;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoefficientComponent)
};
