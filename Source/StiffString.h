/*
  ==============================================================================

    StiffString.h
    Created: 31 Aug 2021 4:41:41pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "ResonatorModule.h"
//==============================================================================
/*
*/
class StiffString  : public ResonatorModule
{
public:
    StiffString (ResonatorModuleType rmt, NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc = simplySupportedBC);
    ~StiffString() override;

    // initialisation
    void initialise (int fs) override;
    
    // JUCE functions
    void paint (juce::Graphics&) override;
    void resized() override;

    Path visualiseState (Graphics& g);

    void calculate() override;
    float getOutput (int idx) override;
    
    int getNumPoints() override;
    int getNumIntervals() override { return N; } ;

    // interaction
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void excite() override;
    
    double getKinEnergy() override;
    double getPotEnergy() override;
    double getDampEnergy() override;
    double getInputEnergy() override;

    void setExcitationFlagFalse() { excitationFlag = false; };
    
private:
    
    // Model parameters
    double L, rho, A, T, E, I, cSq, kappaSq, sig0, sig1, lambdaSq, muSq, h, k;

    /* Scheme variables
        - Adiv for u^{n+1} (that all terms get divided by)
        - B for u^n
        - C for u^{n-1}
        - S for precalculated sigma terms
    */
    double Adiv, B0, B1, B2, C0, C1, S0, S1, Bss;

    float excitationLoc = 0.5;    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StiffString)
};
