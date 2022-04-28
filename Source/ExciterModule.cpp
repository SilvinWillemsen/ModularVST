/*
  ==============================================================================

    ExciterModule.cpp
    Created: 23 Nov 2021 10:28:44am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ExciterModule.h"

//==============================================================================
ExciterModule::ExciterModule (int ID, bool isModule1D, ExcitationType excitationType) : ID (ID), isModule1D(isModule1D), excitationType (excitationType)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ExciterModule::~ExciterModule()
{
}
