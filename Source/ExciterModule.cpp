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
ExciterModule::ExciterModule (int ID, int N, ExcitationType excitationType) : ID (ID), N (N), excitationType (excitationType)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ExciterModule::~ExciterModule()
{
}

void ExciterModule::updateSmoothExcitation()
{
    excitationLoc = smoothCoeff * excitationLoc + (1.0 - smoothCoeff) * excitationLocToGoTo;
    controlLoc = smoothCoeff * controlLoc + (1.0 - smoothCoeff) * controlLocToGoTo;
}

void ExciterModule::toggleSmoothExcitation()
{
   exciteSmooth = !exciteSmooth;
   if (exciteSmooth)
   {
       excitationLocToGoTo = excitationLoc;
       controlLocToGoTo = controlLoc;
   }
   else
   {
       excitationLoc = excitationLocToGoTo;
       controlLoc = controlLocToGoTo;
   }
   
};
