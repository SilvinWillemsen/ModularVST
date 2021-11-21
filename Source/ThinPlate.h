/*
  ==============================================================================

    ThinPlate.h
    Created: 13 Oct 2021 2:40:46pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "StiffMembrane.h"

//==============================================================================
/*
    A Thin Plate is a stiff membrane without tension
*/

class ThinPlate  : public StiffMembrane
{
public:
    ThinPlate (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo = InOutInfo(), BoundaryCondition bc = clampedBC);
    ~ThinPlate() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThinPlate)
};
