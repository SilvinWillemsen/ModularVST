/*
  ==============================================================================

    Bar.h
    Created: 9 Sep 2021 1:18:37pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "StiffString.h"
//==============================================================================
/*
    A Bar is a Stiff String without tension
*/
class Bar  : public StiffString
{
public:
    Bar (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc = simplySupportedBC);
    ~Bar() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bar)
};
