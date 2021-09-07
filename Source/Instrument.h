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
    
    // Checks whether modules should be excited
    void checkIfShouldExcite();

    double getFs() { return fs; };
    
    void mouseDown (const MouseEvent& e) override;
//    void mouseMove (const MouseEvent& e) override;

    void setApplicationState (ApplicationState a);
    
    void setStatesToZero() { for (auto res : resonators) res->setStatesToZero(); }
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    void setAddingConnection (bool a) { addingConnection = a; };
    
private:
    int fs;
    
//    int mouseX, mouseY;
    // vector of tuples storing the resonator module index and the connection location index
    std::vector<std::vector<int>> connectionLocations;

    std::vector<std::shared_ptr<ResonatorModule>> resonators;
    
    ApplicationState applicationState = normalState;
    
    bool painting = true;
    int resonatorModuleHeight = 0;
    bool addingConnection = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Instrument)
};
