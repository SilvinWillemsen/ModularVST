/*
  ==============================================================================

    ResonatorModule.h
    Created: 3 Sep 2021 11:38:22am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
//==============================================================================
/*
 Things that need to be initialised in the constructor of a resonator module (inheriting from this class):
    -initialise module
*/

class ResonatorModule  : public juce::Component, public ChangeBroadcaster
{
public:
    ResonatorModule (NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc);
    ~ResonatorModule() override;
    
    virtual void initialise (int fs) = 0;
    void setStatesToZero();
    
    bool isModuleReady() { return moduleIsReady; };
    
    // Scheme functions
    virtual void calculate() = 0;   // Calculate the FD scheme
    void update();                  // Update internal system states
    
    // Connection
    double getStateAt (int idx, int time);
    void addForce (double force, int idx);
//    void addToStateAt (int idx);
    
    // energy
    double getTotalEnergy() { return getKinEnergy() + getPotEnergy() + getDampEnergy() + getInputEnergy(); };
    
    // Output
    virtual float getOutput() = 0;
        
    bool shouldExcite() { return excitationFlag; };
    virtual void excite() {};
    
    void setApplicationState (ApplicationState a) { applicationState = a; };
    
    int getConnLoc() { return connLoc; };
    void setConnLoc (int c) { connLoc = c; };
        
    // getters
    int getID() { return ID; };
    int getNumIntervals() { return N; };
    virtual int getNumPoints() = 0;

    int getVisualScaling() { return visualScaling; };
    double getConnectionDivisionTerm() { return connectionDivisionTerm; };
    void setConnectionDivisionTerm (double cDT) { connectionDivisionTerm = cDT; };
    
protected:
    // Initialises the module. Must be called at the end of the constructor of the module inheriting from ResonatorModule
    void initialiseModule();

    virtual double getKinEnergy() {};
    virtual double getPotEnergy() {};
    virtual double getDampEnergy() {};
    virtual double getInputEnergy() {};
    
    double k;
    
    // Number of intervals
    int N = -1;

    NamedValueSet parameters;
    
    std::vector<double*> u;                     // state pointers
    std::vector<std::vector<double>> uStates;   // state vectors
    
    BoundaryCondition bc;

    bool excitationFlag = false;
    Component* parentComponent;
    ApplicationState applicationState = normalState;

    
    int visualScaling;
    double totalEnergy;
    
    double kinEnergy, potEnergy;
    double dampEnergy = 0;
    double inputEnergy = 0;
    double prevDampEnergy = 0;
    double prevInputEnergy = 0;
    double dampTot = 0;
    double inputTot = 0;
    

private:
    int ID; // Holds the index in the vector of resonator modules in the instrument
    bool moduleIsReady = false; // Becomes true when the u vectors are initialised
    
    int connLoc = -1;

    double connectionDivisionTerm = -1;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonatorModule)
};
