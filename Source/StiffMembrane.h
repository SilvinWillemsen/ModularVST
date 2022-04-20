/*
  ==============================================================================

    StiffMembrane.h
    Created: 12 Oct 2021 10:38:41am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "ResonatorModule.h"

#include <iostream>
#include <fstream>

//==============================================================================
/*
*/
class StiffMembrane  : public ResonatorModule
{
public:
    StiffMembrane (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo = InOutInfo(), BoundaryCondition bc = clampedBC);
    ~StiffMembrane() override;

    // initialisation
    void initialise (int fs) override;
    void refreshCoefficients() override;

    // JUCE functions
    void paint (juce::Graphics&) override;
    void resized() override;

    Path visualiseState (Graphics& g);

    void calculate() override;
    void exciteRaisedCos() override;
    
    void onlyCalculateMembrane();
    void calculateAll();
    
    float getOutput (int idx) override;
    
    int getNumPoints() override;
    int getNumIntervals() override { return N; }; // should find a way to remove this

    // interaction
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
//    void mouseMove (const MouseEvent& e) override;
    
    void myMouseEnter (const double x, const double y, bool triggeredByMouse) override;
    void myMouseExit (const double x, const double y, bool triggeredByMouse) override;
    void myMouseMove (const double x, const double y, bool triggeredByMouse) override;
    
    double getKinEnergy() override;
    double getPotEnergy() override;
    double getDampEnergy() override;
    double getInputEnergy() override;

    double getMassPerGridPoint() override { return rho * H * h * h; };

    void changeDensity (double rhoToSet) override;
    
    void initialiseExciterModule (std::shared_ptr<ExciterModule> exciterModule) override;

protected:
    
    int maxPoints;
    
    // Model parameters
    double Lx, Ly, rho, H, T, E, D, nu, cSq, kappaSq, sig0, sig1, lambdaSq, muSq, h, k;

    /* Scheme variables
        - Adiv for u^{n+1} (that all terms get divided by)
        - B for u^n
        - C for u^{n-1}
        - S for precalculated sigma terms
    */
    double Adiv, B0, B1, B11, B2, C0, C1, S0, S1, Bss, BssC;

    float excitationLocX = 0.5;
    float excitationLocY = 0.5;

    int excitationWidth = 5;
    
#ifdef SAVE_OUTPUT
    std::ofstream statesSave;
    int samplesToRecord = 100;
    int counter = 0;
#endif
//    typedef void (StiffMembrane::*updateEq)();
//    updateEq test;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StiffMembrane)
};
