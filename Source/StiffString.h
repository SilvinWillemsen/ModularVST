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
    StiffString (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo = InOutInfo(), BoundaryCondition bc = simplySupportedBC);
    ~StiffString() override;

    // initialisation
    void initialise (int fs) override;
    
    // JUCE functions
    void paint (juce::Graphics&) override;
    void resized() override;

    Path visualiseState (Graphics& g);

    void calculate() override;
    void exciteRaisedCos() override;

    float getOutput (int idx) override;
    
    int getNumPoints() override;
    int getNumIntervals() override { return N; } ;
        
//#ifndef EDITOR_AND_SLIDERS
    // interaction
    void mouseEnter (const MouseEvent& e) override {
        if (isPartOfGroup())
        {
            setEnteredThisResonator (true);
            this->findParentComponentOfClass<Component>()->mouseEnter (e);
        }
        else
        {
            myMouseEnter (e.x, e.y, true);
        }
    };
    void mouseExit (const MouseEvent& e) override {
        if (isPartOfGroup())
        {
            setEnteredThisResonator (false);
            this->findParentComponentOfClass<Component>()->mouseExit (e);
        }
        else
        {
            myMouseExit (e.x, e.y, true);
        }
    };
    void mouseMove (const MouseEvent& e) override {
        if (isPartOfGroup())
            this->findParentComponentOfClass<Component>()->mouseMove (e);
        else
            myMouseMove (e.x, e.y, true);
 };
    // for now uses the mouse event and calls instrument so can't have custom mouse funtions
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
//#endif
    
    void myMouseEnter (const double x, const double y, bool triggeredByMouse) override;
    void myMouseExit (const double x, const double y, bool triggeredByMouse) override;
    void myMouseMove (const double x, const double y, bool triggeredByMouse) override;

    double getKinEnergy() override;
    double getPotEnergy() override;
    double getDampEnergy() override;
    double getInputEnergy() override;
    
    double getMassPerGridPoint() override { return rho * A * h; };
    void initialiseExciterModule (std::shared_ptr<ExciterModule> exciterModule) override;
    
    void saveOutput() override;
    
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

    double prevLoc = 0;
    float excitationLoc = 0.5;
    float yLoc = 0;
    
#ifdef SAVE_OUTPUT
    std::ofstream statesSave;
    int counter = 0;
    bool doneRecording = false;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StiffString)
};
