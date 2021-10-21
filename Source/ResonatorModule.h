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
    ResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc);
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
        
    // ID
    int getID() { return ID; };
    void setID (int I) { ID = I; } // only to be called when index of resonator in vector changes
    
    virtual int getNumIntervals() = 0;
    virtual int getNumPoints() = 0;
    
    virtual int getNumIntervalsX() {
        if (is1D)
            std::cout << "MODULE IS 1D!!" << std::endl;
        return Nx;
        
    };
    virtual int getNumIntervalsY() {
        if (is1D)
            std::cout << "MODULE IS 1D!!" << std::endl;
        return Ny;
        
    };

    ResonatorModuleType getResonatorModuleType() { return resonatorModuleType; };

    bool isModule1D() { return is1D; };
    
    
    int getVisualScaling() { return visualScaling; };
    double getConnectionDivisionTerm() { return connectionDivisionTerm; };
    void setConnectionDivisionTerm (double cDT) { connectionDivisionTerm = cDT; };
    
protected:
    // Initialises the module. Must be called at the end of the constructor of the module inheriting from ResonatorModule
    void initialiseModule();

    virtual double getKinEnergy() = 0;
    virtual double getPotEnergy() = 0;
    virtual double getDampEnergy() = 0;
    virtual double getInputEnergy() = 0;
    
    double k;
    
    // Number of intervals
    int N = -1;
    
    // 2D variables
    int Nx = -1;
    int Ny = -1;

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
    ResonatorModuleType resonatorModuleType; // what type of resonator is this
    bool is1D;
    int connLoc = -1;

    double connectionDivisionTerm = -1;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonatorModule)
};
