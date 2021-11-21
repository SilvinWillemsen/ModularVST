/*
  ==============================================================================

    ThinPlate.cpp
    Created: 13 Oct 2021 2:40:46pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ThinPlate.h"

//==============================================================================
ThinPlate::ThinPlate (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc) : StiffMembrane (rmt, parameters, advanced, fs, ID, instrument, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ThinPlate::~ThinPlate()
{
}
