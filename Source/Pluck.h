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
    Pluck (int ID, int N);
    ~Pluck() override;
    
    void drawExciter (Graphics& g) override;
    
    void initialise (NamedValueSet& parameters) override;
    void calculate (std::vector<double*>& u) override;
    
    double getEnergy() override;

    void updateStates() override;
    void hiResTimerCallback() override;
    
    void mouseEntered (const double x, const double y, int height) override;
    void mouseExited() override;
    
    void setResHeight (int r) { resHeight = r; };
    
    void saveOutput() override;
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
    
    int cLoc;
    double alpha;
    
    bool pickIsAbove;
    bool plucked = false;
    int pluckedCounter = 0;
    int pluckedCounterLimit = 1000;
    int pluckSgn;
    double forceLimitOh = 500;
        
    double totDampEnergy = 0;
    double totPowEnergy = 0;
    double prevDampEnergy = 0;
    double prevPowEnergy = 0;
    double prevTotEnergy = 0;
    
    double resHeight = 100;
    bool singlePoint = false;
    double width = 0;
    
    std::vector<double> I, ItoDraw;
    double IJ = 0;
    
#ifdef SAVE_OUTPUT
    std::ofstream statesSave;
    int counter = 0;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pluck)
};
