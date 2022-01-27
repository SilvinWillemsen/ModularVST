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
    ExciterModule (int ID, int N = 0, ExcitationType excitationType = noExcitation);
    virtual ~ExciterModule();

    ExcitationType getExcitationType() { return excitationType; };
    void setExcitationType (ExcitationType e) { excitationType = e; };
    
    virtual void drawExciter (Graphics& g) {};
    virtual void initialise (NamedValueSet& parametersFromResonator) {};
    
    virtual void calculate (std::vector<double*>& u) {};
    virtual void updateStates() {};
    virtual double getEnergy() { return 0; };

//    void setNumberOfIntervals (int n) { N = n; };
    int getID() { return ID; };
    void setForce (double force) { f = force; };
    
    double getControlParameter() { return controlParameter; };
    void setControlParameter (double c) { controlParameter = c; };
    
    void setExcitationLoc (double loc) { excitationLoc = loc; };
    void setControlLoc (double loc) { controlLoc = loc; }; // ypos

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
    protected:
    ExcitationType excitationType;
    double excitationLoc = 0.5;
    double controlLoc = 0;

    int N = 0;
    double f = 1;
    double controlParameter = 6; // parameter to be controlled by the application. Could be bow velocity fx.
    bool trigger = false;
    
    long calcCounter = 0;
    bool moduleIsReady = false;
    bool moduleIsCalculating = false;
    
private:
    int ID = -1;
    Action action = noAction;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExciterModule)
};
