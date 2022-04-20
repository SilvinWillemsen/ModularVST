/*
  ==============================================================================

    ExciterModule.h
    Created: 23 Nov 2021 10:28:44am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
//==============================================================================
/*
*/
class ExciterModule : public HighResolutionTimer, public ChangeBroadcaster
{
public:
    ExciterModule (int ID, bool isModule1D, ExcitationType excitationType = noExcitation);
    virtual ~ExciterModule();

    ExcitationType getExcitationType() { return excitationType; };
    void setExcitationType (ExcitationType e) { excitationType = e; };
    
    virtual void drawExciter (Graphics& g) {};
    virtual void setN (int NtoSet) { N = NtoSet; };
    virtual void setNxNy (int NxToSet, int NyToSet) { Nx = NxToSet; Ny = NyToSet; };
    
    virtual void initialise (NamedValueSet& parametersFromResonator) {};
    
    virtual void calculate (std::vector<double*>& u) {};
    virtual void updateStates() {};
    virtual double getEnergy() { return 0; };

//    void setNumberOfIntervals (int n) { N = n; };
    int getID() { return ID; };
    void setForce (double force) { f = force; };
    
    double getControlParameter() { return controlParameter; };
    void setControlParameter (double c) { controlParameter = c; };
    
    void setExcitationLoc (double loc) {
        if (exciteSmooth)
            excitationLocToGoTo = loc;
        else
            excitationLoc = loc;
    };
    
    void setExcitationLocX (double loc) {
        if (exciteSmooth)
            excitationLocToGoToX = loc;
        else
            excitationLocX = loc;
    };
    
    void setExcitationLocY (double loc) {
        if (exciteSmooth)
            excitationLocToGoToY = loc;
        else
            excitationLocY = loc;
    };

    void setControlLoc (double loc) {
        if (exciteSmooth)
            controlLocToGoTo = loc;
        else
            controlLoc = loc;
        
    }; // ypos

    void updateSmoothExcitation();
    
    void hiResTimerCallback() override {};
    
    long getCalcCounter() { return calcCounter; };
    
    virtual void mouseEntered (const double x, const double y, int height) {};

    virtual void mouseExited () {};

    bool isModuleReady() { return moduleIsReady; };
    
    void setStatesToZero() { action = setStatesToZeroAction; sendChangeMessage(); };
    Action getAction() { return action; };
    void setAction (Action a) { action = a; };
    
    virtual void saveOutput() {};
    
    void triggerExciterModule() { trigger = true; };
    void toggleSmoothExcitation();
    bool shouldExciteSmooth() { return exciteSmooth; };

    bool isModuleCalculating() { return moduleIsCalculating; };
    
protected:
    ExcitationType excitationType;
    double excitationLoc = 0.5;
    
    double excitationLocX = 0.5;
    double excitationLocY = 0.5;

    double controlLoc = 0;

    int N, Nx, Ny;
    double f = 1;
    double controlParameter = 6; // parameter to be controlled by the application. Could be bow velocity fx.
    bool trigger = false;
    
    long calcCounter = 0;
    bool moduleIsReady = false;
    bool moduleIsCalculating = false;
        
    bool isModule1D;
    
private:
    int ID = -1;
    Action action = noAction;
    
    bool exciteSmooth = false;
    double excitationLocToGoTo = 0.5;
    double excitationLocToGoToX = 0.5;
    double excitationLocToGoToY = 0.5;
    
    double controlLocToGoTo = 0;
    
    float smoothCoeff = 0.9999;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExciterModule)
};
