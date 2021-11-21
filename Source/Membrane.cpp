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
Membrane::Membrane (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc) : StiffMembrane (rmt, parameters, advanced, fs, ID, instrument, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Membrane::~Membrane()
{
}

void Membrane::calculate()
{
    for (int m = 1; m < Ny; ++m) // clamped boundaries
    {
        for (int l = 1; l < Nx; ++l) // clamped boundaries
        {
            u[0][l + m*Nx] =
                  B0 * u[1][l + m*Nx]
                + B1 * (u[1][l+1 + m*Nx] + u[1][l-1 + m*Nx] + u[1][l + (m+1)*Nx] + u[1][l + (m-1)*Nx])
                + C0 * u[2][l + m*Nx]
                + C1 * (u[2][l+1 + m*Nx] + u[2][l-1 + m*Nx] + u[2][l + (m+1)*Nx] + u[2][l + (m-1)*Nx]);

#ifdef SAVE_OUTPUT
            statesSave << u[1][l + m*Nx] << ",";
#endif
            
        }
#ifdef SAVE_OUTPUT
        statesSave << ";\n";
#endif
    }
#ifdef SAVE_OUTPUT
    ++counter;
    if (counter >= samplesToRecord)
        statesSave.close();
#endif
}
