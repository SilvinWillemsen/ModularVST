/*
  ==============================================================================

    InOutInfo.h
    Created: 21 Nov 2021 6:02:32pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Global.h"

class InOutInfo
{
public:
    InOutInfo (bool defaultInit = true);
    ~InOutInfo();

    void addInput (int loc, int channel = 2);
    void removeInput (int idx);
    
    void addOutput (int loc, int channel = 2);
    void removeOutput (int idx);
    
    int getNumInputs() { return numInputs; };
    int getNumOutputs() { return numOutputs; };
    
    int getInLocAt (int idx) { return inLocs[idx]; };
    int getInChannelAt (int idx) { return inChannels[idx]; };

    int getOutLocAt (int idx) { return outLocs[idx]; };
    int getOutChannelAt (int idx) { return outChannels[idx]; };

    bool isDefaultInit() { return defaultInit; };
private:
    
    // Input locations and channels
    std::vector<int> inLocs;
    std::vector<int> inChannels; // 0 - left; 1 - right; 2 - both
    
    int numInputs = 0;
    
    // Output locations and channels
    std::vector<int> outLocs;
    std::vector<int> outChannels; // 0 - left; 1 - right; 2 - both
    
    int numOutputs = 0;
    bool defaultInit;
};
