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
    
    double getEnergy() override;

    void updateStates() override;
    void hiResTimerCallback() override;
    
    void mouseEntered (const MouseEvent& e, int height) override;
    void mouseExited() override;
    
    void setResHeight (int r) { resHeight = r; };
    
private:
    // string variables still needed in the NR solve
    double rho, A, sig0, k, h;
    double connectionDivisionTerm;

    double B1, B2, C1, Adiv;
    double v1, v2, a11, a12, a21, a22, oOdet, solut1, solut2;
    
    double uStar, wStar; // Intermediate state
    double uI, uIPrev; // Interpolated states
    double K, M, R;
    
    double Kc, KcOrig, alphaC, g, kappaG, Jterm;
    double psiPrev = 0;
    double psi = 0;
    double etaNext, eta, etaPrev, etaStar;
    double wNext, w, wPrev;
    
    bool pickIsAbove;
    bool plucked = false;
    int pluckedCounter = 0;
    int pluckedCounterLimit = 1000;
    int pluckSgn;
    double forceLimit = 100;
    
    double maxForce = 0.1;
    
    double totDampEnergy = 0;
    double totPowEnergy = 0;
    double prevDampEnergy = 0;
    double prevPowEnergy = 0;
    double prevTotEnergy = 0;
    
    double resHeight = 100;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pluck)
};
