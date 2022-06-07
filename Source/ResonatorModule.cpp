/*
  ==============================================================================

    ResonatorModule.cpp
    Created: 3 Sep 2021 11:38:22am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ResonatorModule.h"

//==============================================================================
ResonatorModule::ResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo, BoundaryCondition bc) : k (1.0 / fs), parameters (parameters), resonatorModuleType(rmt), ID (ID), inOutInfo (inOutInfo), bc (bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    if (rmt == bar || rmt == stiffString)
        is1D = true;
    else
        is1D = false;
    addChangeListener (instrument);
}

ResonatorModule::~ResonatorModule()
{
}

bool ResonatorModule::initialiseModule()
{
//    // Check whether the number of grid points has been set
//    jassert (N > 0);
    
    // Limit on number of points
    if (is1D)
    {
        if (N > 1000)
        {
            errorMsg = "Too many points!";
            return false;
        } else if (N < 10)
        {
            errorMsg = "Too few points!";
            return false;
        }
    } else {
        if (N > 10000)
        {
            errorMsg = "Too many points!";
            return false;
        }
        if (Nx < 5 || Ny < 5)
        {
            errorMsg = "Too few points!";
            return false;
        }
    }
    canInitialise = true;
    
    /*  Make u pointers point to the first index of the state vectors.
        To use u (and obtain a vector from the state vectors) use indices like u[n][l] where,
             - n = 0 is u^{n+1},
             - n = 1 is u^n, and
             - n = 2 is u^{n-1}.
        Also see calculateScheme()
     */
    
    // Initialise vectors
    uStates = std::vector<std::vector<double>> (3,
                                        std::vector<double>(N+1, 0));

    // Initialise pointer vector
    u.resize (3, nullptr);
    
    // Make set memory addresses to first index of the state vectors.
    for (int i = 0; i < 3; ++i)
        u[i] = &uStates[i][0];
    
    jassert (connectionDivisionTerm != -1); // connectionDivisionTerm must have been set in module inheriting from this class

    pluckModule = std::make_shared<Pluck>(getID(), N);
    pluckModule->addChangeListener (this);
    initialiseExciterModule (pluckModule);

    hammerModule = std::make_shared<Hammer>(getID(), N);
    hammerModule->addChangeListener (this);
    initialiseExciterModule (hammerModule);

    bowModule = std::make_shared<Bow>(getID(), N);
    bowModule->addChangeListener (this);
    initialiseExciterModule (bowModule);
    

    curExciterModule = nullptr;
    setExcitationType (noExcitation);

    moduleIsReady = true;
    justReady = true;
    return true;
}

void ResonatorModule::setStatesToZero()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < uStates[i].size(); ++j)
            u[i][j] = 0.0;
}

void ResonatorModule::update()
{
    double* uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
    
    if (curExciterModule != nullptr)
        curExciterModule->updateStates();
}

double ResonatorModule::getStateAt (int idx, int time)
{
//    jassert(connectionLocationIndex >= connectionLocations.size());
    return u[time][idx];
}

void ResonatorModule::addForce (double force, int idx, double customMassRatio)
{
    u[0][idx] += customMassRatio * connectionDivisionTerm * force;
}

void ResonatorModule::setExcitationType (ExcitationType e)
{
    if (excitationType == e)
        return;
    excitationType = e;
    switch (e)
    {
        case pluck:
            curExciterModule = pluckModule;
            break;
        case hammer:
            curExciterModule = hammerModule;
            break;
        case bow:
            curExciterModule = bowModule;
            curExciterModule->setControlParameter (0.2);
            break;
    }

}

void ResonatorModule::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    if (changeBroadcaster == curExciterModule.get())
        if (curExciterModule->getAction() == setStatesToZeroAction)
        {
            action = setStatesToZeroAction;
            sendChangeMessage();
            curExciterModule->setAction (noAction);
        }
}
