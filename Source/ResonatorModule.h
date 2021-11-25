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
#include "ExciterModule.h"
#include "Pluck.h"
#include "Bow.h"
#include "InOutInfo.h"
//==============================================================================
/*
 Things that need to be initialised in the constructor of a resonator module (inheriting from this class):
    -initialise module
*/

class ResonatorModule  : public juce::Component, public ChangeBroadcaster//, public Timer
{
public:
    ResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo = InOutInfo(), BoundaryCondition bc = clampedBC);
    ~ResonatorModule() override;
    
    virtual void initialise (int fs) = 0;
    void setStatesToZero();
    
    bool isModuleReady() { return moduleIsReady; };
    bool isJustReady() { return justReady; };
    void setNotJustReady() { justReady = false; };
    void unReadyModule() { moduleIsReady = false; }; // for module removal
    void readyModule() { moduleIsReady = true; }; // for module removal

    // Scheme functions
    virtual void calculate() = 0;   // Calculate the FD scheme
    void update();                  // Update internal system states
    
    // Connection
    double getStateAt (int idx, int time);
    void addForce (double force, int idx, double customMassRatio);
//    void addToStateAt (int idx);
    
    // energy
    double getTotalEnergy() { return getKinEnergy() + getPotEnergy() + getDampEnergy() + getInputEnergy(); };
    
    // Output
    virtual float getOutput (int idx) = 0;
        
    // Raised cosine excitation
    bool shouldExciteRaisedCos() { return rcExcitationFlag; };
    virtual void exciteRaisedCos() {};
    
    // Excite using excitation module
    void excite() { if (excitationActive) getExciterModule()->calculate (u); };

    void setApplicationState (ApplicationState a) { applicationState = a; };
    
    int getMouseLoc() { return mouseLoc; };
    void setMouseLoc (int m) { mouseLoc = m; };
        
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
       
    void setModifier (ModifierKeys mod) { modifier = mod; };
    ModifierKeys getModifier() { return modifier; };

    NamedValueSet& getParameters() { return parameters; }; // for presets
    
    void changeTotInputs (bool increment) { if (increment) ++totInputs; else --totInputs; };
    void changeTotOutputs (bool increment) { if (increment) ++totOutputs; else --totOutputs; };

    int getTotInputs() { return totInputs; };
    int getTotOutputs() { return totOutputs; };
    
    void setParameters (NamedValueSet& p) { parameters = p; };
    
    InOutInfo* getInOutInfo() { return &inOutInfo; };

    void setExcitationType (ExcitationType e);
    ExcitationType getExcitationType() { return excitationType; };
    
//    void timerCallback() override;
    bool isExcitationActive() { return excitationActive; };
    void setExcitationActive (bool a) { excitationActive = a; };
    
    std::shared_ptr<ExciterModule> getExciterModule() { return exciterModule; };
    virtual void initialiseExciterModule() {};
    
    long getCalcCounter() { return calcCounter; };

    bool isChildOfHighlightedInstrument() { return childOfHighlightedInstrument; };
    void setChildOfHighlightedInstrument (bool c) { childOfHighlightedInstrument = c; };
    
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
    
    std::vector<double*> u;                     // state pointers
    std::vector<std::vector<double>> uStates;   // state vectors
    
    InOutInfo inOutInfo;
    
    BoundaryCondition bc;

    bool rcExcitationFlag = false;
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
    
    bool alreadyExcited = false; // for hover excitation
    
    long calcCounter = 0;
    
private:
    int ID; // Holds the index in the vector of resonator modules in the instrument
    bool moduleIsReady = false; // Becomes true when the u vectors are initialised
    bool justReady = false; // Becomes true when the u vectors are initialised
    ResonatorModuleType resonatorModuleType; // what type of resonator is this
    bool is1D;
    int mouseLoc = -1;

    double connectionDivisionTerm = -1;
    ModifierKeys modifier; // modifier for connections (left / right mouse click + click-n-drag with ctrl)
    
    NamedValueSet parameters;

    int totInputs = 0;
    int totOutputs = 0;
    
    ExcitationType excitationType = noExcitation;
    std::shared_ptr<ExciterModule> exciterModule;
    bool excitationActive = Global::bowAtStartup ? true : false;
    
    bool childOfHighlightedInstrument = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonatorModule)
};
