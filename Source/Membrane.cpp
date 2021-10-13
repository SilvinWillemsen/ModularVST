/*
  ==============================================================================

    Membrane.cpp
    Created: 13 Oct 2021 2:40:20pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Membrane.h"

//==============================================================================
Membrane::Membrane (ResonatorModuleType rmt, NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc) : StiffMembrane (rmt, parameters, fs, ID, instrument, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Membrane::~Membrane()
{
}
