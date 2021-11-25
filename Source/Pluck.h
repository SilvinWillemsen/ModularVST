/*
  ==============================================================================

    Pluck.h
    Created: 25 Nov 2021 2:42:09pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Global.h"
#include "ExciterModule.h"

//==============================================================================
/*
*/
class Pluck  :  public ExciterModule
{
public:
    Pluck (int N);
    ~Pluck() override;
    
    void drawExciter (Graphics& g) override;
    
    void initialise (NamedValueSet& parameters) override;
    void calculate (std::vector<double*>& u) override;
    
    void updateStates();
    void hiResTimerCallback() override;
private:
    // string variables still needed in the NR solve
    double rho, A, sig0, k, h;
    double connectionDivisionTerm;

    double uStar, wStar; // Intermediate state
    double uI, uIPrev; // Interpolated states
    double K, M, R;
    
    double Kc, KcOrig, alphaC, g, psiPrev, psi, kappaG, Jterm;
    double etaNext, eta, etaPrev, etaStar;
    double wNext, w, wPrev;
    
    bool pickIsAbove = true;
    bool plucked = false;
    int pluckedCounter = 0;
    int pluckedCounterLimit = 1000;
    int pluckSgn = -1;
    double forceLimit = 10;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pluck)
};
