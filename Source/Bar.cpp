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
Bar::Bar (NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc) : StiffString (parameters, fs, ID, instrument, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

Bar::~Bar()
{
}
