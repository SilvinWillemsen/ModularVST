/*
  ==============================================================================

    Membrane.h
    Created: 13 Oct 2021 2:40:20pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "StiffMembrane.h"

//==============================================================================
/*
    A Membrane is a stiff membrane without stiffness
*/
class Membrane  : public StiffMembrane
{
public:
    Membrane (ResonatorModuleType rmt, NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc = simplySupportedBC);
    ~Membrane() override;
    
    void calculate() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Membrane)
};
