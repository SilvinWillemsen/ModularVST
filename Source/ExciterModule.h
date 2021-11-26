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
class ExciterModule : public HighResolutionTimer
{
public:
    ExciterModule (int N = 0, ExciterModuleType exciterModuleType = noExciter);
    virtual ~ExciterModule();

    ExciterModuleType getExciterModuleType() { return exciterModuleType; };
    void setExciterModuleType (ExciterModuleType e) { exciterModuleType = e; };
    
    virtual void drawExciter (Graphics& g) {};
    virtual void initialise (NamedValueSet& parametersFromResonator) {};
    
    virtual void calculate (std::vector<double*>& u) {};
    virtual void updateStates() {};
    virtual double getEnergy() { return 0; };

//    void setNumberOfIntervals (int n) { N = n; };
    
    void setForce (double force) { f = force; };
    
    double getControlParameter() { return controlParameter; };
    void setControlParameter (double c) { controlParameter = c; };
    
    void setExcitationLoc (double loc) { excitationLoc = loc; };
    void setControlLoc (double loc) { controlLoc = loc; }; // ypos

    void hiResTimerCallback() override {};
    
    long getCalcCounter() { return calcCounter; };
    
    virtual void mouseEntered (const MouseEvent& e, int height) {};
    virtual void mouseExited () {};

    bool isModuleReady() { return moduleIsReady; };
    
protected:
    ExciterModuleType exciterModuleType;
    double excitationLoc = 0.5;
    double controlLoc = 0;

    int N = 0;
    double f = 1;
    double controlParameter = 0; // parameter to be controlled by the application. Could be bow velocity fx.
    
    long calcCounter = 0;
    bool moduleIsReady = false;
    bool moduleIsCalculating = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExciterModule)
};
