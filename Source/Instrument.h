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

// include all types of resonator module here
#include "StiffString.h"
#include "Bar.h"
#include "StiffMembrane.h"
#include "Membrane.h"
#include "ThinPlate.h"


//==============================================================================
/*
 The instrument class is a wrapper for various modules and handles the interactions between them.
*/
class Instrument  : public juce::Component, public ChangeBroadcaster, public ChangeListener
{
public:
    Instrument (ChangeListener& audioProcessorEditor, int fs);
    ~Instrument() override;
    
    // Vector storing information about the connections. [0] resonator index, [1] location
    struct ConnectionInfo
    {
        ConnectionInfo (ConnectionType connType,
                        std::shared_ptr<ResonatorModule> resonator1,
                        int location,
                        ResonatorModuleType resonatorModuleType,
                        double K1 = 0,
                        double K3 = 0,
                        double R = 0) : connType (connType),
                                        res1 (resonator1),
                                        loc1 (location),
                                        rmt1 (resonatorModuleType),
                                        K1 (K1), K3 (K3), R (R)
        {};
        void setSecondResonatorParams (std::shared_ptr<ResonatorModule> resonator2, int l, ResonatorModuleType r)
        {
            if (res1->getID() > resonator2->getID()) // always have res1 have the lower ID than res2
            {
                // copy all values from res1 to res2
                res2 = res1;
                loc2 = loc1;
                rmt2 = rmt1;
                res2 = res1;
                
                res1 = resonator2;
                loc1 = l;
                rmt1 = r;
                
            }
            else
            {
                res2 = resonator2;
                loc2 = l;
                rmt2 = r;
            }
            connected = true;
        };
        
        void setCustomMassRatio (double value) {
            customMassRatio = value / (res1->getConnectionDivisionTerm() / res2->getConnectionDivisionTerm());
        };
        
        double getMassRatio() { return customMassRatio * (res1->getConnectionDivisionTerm() / res2->getConnectionDivisionTerm()); };
        
        ConnectionType connType;
        std::shared_ptr<ResonatorModule> res1, res2;
        int loc1, loc2;
        ResonatorModuleType rmt1, rmt2;

        // connection variables
        double K1, K3, R;
        double etaNext, eta, etaPrev;
        bool connected = false;
        int connectionGroup = -1;
        
        double customMassRatio = 1.0;
        
        
    };
    
    struct InOutInfo
    {
        InOutInfo()
        {
            inResonators.reserve (16);
            inLocs.reserve (16);
            inChannels.reserve (16);

            outResonators.reserve (16);
            outLocs.reserve (16);
            outChannels.reserve (16);
        };

        void addInput (std::shared_ptr<ResonatorModule> res, int loc, int channel = 2)
        {
            res->changeTotInputs (true);
            inResonators.push_back (res);
            inLocs.push_back (loc);
            inChannels.push_back (channel);
            ++numInputs;
        }
        
        void removeInput (int idx)
        {
            inResonators[idx]->changeTotInputs (false);
            inResonators.erase (inResonators.begin() + idx);
            inLocs.erase (inLocs.begin() + idx);
            inChannels.erase (inChannels.begin() + idx);
            --numInputs;

        }
        
        void addOutput (std::shared_ptr<ResonatorModule> res, int loc, int channel = 2)
        {
            res->changeTotOutputs (true);
            outResonators.push_back (res);
            outLocs.push_back (loc);
            outChannels.push_back(channel);
            ++numOutputs;
        }
        
        void removeOutput (int idx)
        {
            outResonators[idx]->changeTotOutputs (false);
            outResonators.erase (outResonators.begin() + idx);
            outLocs.erase (outLocs.begin() + idx);
            outChannels.erase (outChannels.begin() + idx);
            --numOutputs;
        }
        
        // Input resonators, locations and channels
        std::vector<std::shared_ptr<ResonatorModule>> inResonators;
        std::vector<int> inLocs;
        std::vector<int> inChannels; // 0 - left; 1 - right; 2 - both
        
        int numInputs = 0;
        
        // Output resonators, locations and channels
        std::vector<std::shared_ptr<ResonatorModule>> outResonators;
        std::vector<int> outLocs;
        std::vector<int> outChannels; // 0 - left; 1 - right; 2 - both
        
        int numOutputs = 0;

    };
    
    void initialise (int fs);
    
    void paint (juce::Graphics&) override;
    void resized() override;

    bool areModulesReady();
    
    // Get the number of resonator modules in the instrument
    int getNumResonatorModules() { return (int)resonators.size(); };
    std::shared_ptr<ResonatorModule> getResonatorPtr (int idx) { return resonators[idx]; };
    
    // Add/remove a resonator module
    void addResonatorModule(ResonatorModuleType rmt, NamedValueSet& parameters);
    void removeResonatorModule();
    
    void resetResonatorIndices();
    
    // function called from within the addResonatorModule function
    void resetTotalGridPoints();

    // Calculate the schemes of each individual resonator module
    void calculate();
    
    // Solve interactions between resonator modules
    void solveInteractions();
    
    // Update the resonator modules
    void update();
    
    // Returns the output of all modules
    float getOutputL();
    float getOutputR();

    // Calculates total energy of all modules
    void calcTotalEnergy();
    double getTotalEnergy() { return fs * (prevEnergy - totEnergy); };
    
    // Checks whether modules should be excited
    void checkIfShouldExcite();
    
    bool checkIfShouldRemoveResonatorModule() { return shouldRemoveResonatorModule; };
    void setToRemoveResonatorModule () { shouldRemoveResonatorModule = true; };
    
    double getFs() { return fs; };
    
    void mouseDown (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    
    void setApplicationState (ApplicationState a);
    
    void setStatesToZero() { for (auto res : resonators) res->setStatesToZero(); }
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    void setHighlightedInstrument (bool h) { highlightedInstrument = h; };
    void setConnectionType (ConnectionType c);
    
    // Have separate functions (and separate location) for presets
    void addFirstConnection (std::shared_ptr<ResonatorModule> res, ConnectionType connType, int loc);
    void addSecondConnection (std::shared_ptr<ResonatorModule> res, int loc);

//    void setChangeListener (ChangeListener* changeListener) { if (getChangeL) addChangelistener (changeListener); };
    ApplicationState getApplicationState() { return applicationState; };
        
    std::vector<std::vector<int>> getGridPointVector (std::vector<ConnectionInfo*>& CIO);

    bool resetOverlappingConnectionVectors();
    void solveOverlappingConnections (std::vector<ConnectionInfo*>& CIO); // Solve the connections that are overlapping
    
    std::vector<ConnectionInfo>* getConnectionInfo() { return &CI; }; // for presets
    
    InOutInfo* getInOutInfo() { return &inOutInfo; };
    
    bool shouldRemoveInOrOutput() { return (inputToRemove != -1 || outputToRemove != -1); };
    void removeInOrOutput();
    
    ConnectionInfo* getCurrentlyActiveConnection() { return currentlyActiveConnection; };
    void setCurrentlyActiveConnection (ConnectionInfo* CI);
    void setCustomMassRatio (double value) { currentlyActiveConnection->setCustomMassRatio (value); };
    
    Action getAction() { return action; };
    void setAction (Action a) { action = a; };
private:
    InOutInfo inOutInfo;
    
    int fs;
    int totalGridPoints;
    
    std::vector<ConnectionInfo> CI;
    std::vector<std::vector<ConnectionInfo*>> CIOverlapVector; // a vector of groups of overlapping connections
    ConnectionInfo* currentlyActiveConnection = nullptr;
    
    std::vector<std::shared_ptr<ResonatorModule>> resonators;
    
    ApplicationState applicationState = normalState;
    
    bool painting = true;
    int resonatorModuleHeight = 0;
    bool highlightedInstrument = false;
    
    ConnectionType currentConnectionType = rigid;
    
    double K1, K3, R, rPlus, rMinus;
    
    double prevEnergy = 0;
    double totEnergy = 0;
    
    int resonatorToRemoveID = -1;
    int outputToRemove = -1;
    int inputToRemove = -1;
    bool shouldRemoveResonatorModule = false;
    
#ifdef USE_EIGEN
//    Eigen::SparseMatrix<double> IJminP, I, J, IJ, Pmat;
#endif
    
    int currentlySelectedResonator = -1;
    int connectionToMoveIdx;
    bool connectionToMoveIsFirst;
    int prevMouseLoc; // to prevent overlap
    
    Action action = noAction;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Instrument)
};
