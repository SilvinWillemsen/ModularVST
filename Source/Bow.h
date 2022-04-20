/*
  ==============================================================================

    Bow.h
    Created: 23 Nov 2021 10:27:45am
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
class Bow  : public ExciterModule
{
public:
    Bow (int ID, bool isModule1D);
    ~Bow() override;
    
    void drawExciter (Graphics& g) override;
    
    void initialise (NamedValueSet& parameters) override;
    void calculate (std::vector<double*>& u) override;
    
//    double getEnergy() override { return 0; };
    
    void hiResTimerCallback() override;
private:
    // string variables still needed in the NR solve
    double rho, AorH, sig0, k, h;
    double connectionDivisionTerm;

    // bow variables
    double a, cOhSq, kOhhSq, tol, BM, q, qPrev, b, b1, b2;
    double uI, uIPrev, uI1, uI2, uIM1, uIM2, uIPrev1, uIPrevM1; // NR states
    
    double Fb, vB, prevVb;
    
    double prop = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bow)
};
