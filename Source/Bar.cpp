/*
  ==============================================================================

    Bar.cpp
    Created: 9 Sep 2021 1:18:37pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Bar.h"

//==============================================================================
Bar::Bar (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo, BoundaryCondition bc) : StiffString (rmt, parameters, advanced, fs, ID, instrument, inOutInfo, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

Bar::~Bar()
{
}
