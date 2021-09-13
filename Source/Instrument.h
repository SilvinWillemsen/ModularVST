/*
  ==============================================================================

    Instrument.h
    Created: 4 Sep 2021 12:56:47pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "ResonatorModule.h"
#include "StiffString.h"

//==============================================================================
/*
 The instrument class is a wrapper for various modules and handles the interactions between them.
*/
class Instrument  : public juce::Component, public ChangeBroadcaster, public ChangeListener
{
public:
    Instrument (ChangeListener& audioProcessorEditor, int fs);
    ~Instrument() override;

    void initialise (int fs);
    
    void paint (juce::Graphics&) override;
    void resized() override;

    bool areModulesReady();
    
    // Get the number of resonator modules in the instrument
    int getNumResonatorModules() { return resonators.size(); };
    
    void addResonatorModule(ResonatorModuleType rmt, NamedValueSet& parameters);
    
    // Calculate the schemes of each individual resonator modul
    void calculate();
    
    // Solve interactions between resonator modules
    void solveInteractions();
    
    // Update the resonator modules
    void update();
    
    // Returns the output of all modules
    float getOutput();
    
    // Returns total energy of all modules
    double getTotalEnergy();
    
    // Checks whether modules should be excited
    void checkIfShouldExcite();

    double getFs() { return fs; };
    
    void mouseDown (const MouseEvent& e) override;
//    void mouseMove (const MouseEvent& e) override;

    void setApplicationState (ApplicationState a);
    
    void setStatesToZero() { for (auto res : resonators) res->setStatesToZero(); }
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    void setAddingConnection (bool a) { addingConnection = a; };
    void setConnectionType (ConnectionType c) { currentConnectionType = c; };
    
//    void setChangeListener (ChangeListener* changeListener) { if (getChangeL) addChangelistener (changeListener); };
    ApplicationState getApplicationState() { return applicationState; };
    
private:
    int fs;
    
    // Vector storing information about the connections. [0] resonator index, [1] location
    struct ConnectionInfo
    {
        ConnectionInfo (ConnectionType connType,
                        int resonatorIndex,
                        int location,
                        double K1 = 0,
                        double K3 = 0,
                        double R = 0) : connType (connType),
                                        idx1 (resonatorIndex),
                                        loc1 (location),
                                        K1 (K1), K3 (K3), R (R)
        {};
        void setSecondResonatorParams (int i, int l) { idx2 = i; loc2 = l; connected = true; };
        
        ConnectionType connType;
        int idx1, idx2;
        int loc1, loc2;
        double K1, K3, R;
        bool connected = false;
    };
    
    std::vector<ConnectionInfo> CI;
    
    std::vector<std::shared_ptr<ResonatorModule>> resonators;
    
    ApplicationState applicationState = normalState;
    
    bool painting = true;
    int resonatorModuleHeight = 0;
    bool addingConnection = false;
    
    ConnectionType currentConnectionType = rigid;
    
    double K1, K3, R, rPlus, rMinus, etaNext, eta, etaPrev;
    
    double prevEnergy = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Instrument)
};
