/*
  ==============================================================================

    InOutInfo.cpp
    Created: 21 Nov 2021 6:02:32pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "InOutInfo.h"

InOutInfo::InOutInfo (bool defaultInit) : defaultInit (defaultInit)
{
    inLocs.reserve (16);
    inChannels.reserve (16);

    outLocs.reserve (16);
    outChannels.reserve (16);
};

InOutInfo::~InOutInfo()
{
}


void InOutInfo::addInput (int loc, int channel)
{
    inLocs.push_back (loc);
    inChannels.push_back (channel);
    ++numInputs;
}

void InOutInfo::removeInput (int idx)
{
    inLocs.erase (inLocs.begin() + idx);
    inChannels.erase (inChannels.begin() + idx);
    --numInputs;

}

void InOutInfo::addOutput (int loc, int channel)
{
    outLocs.push_back (loc);
    outChannels.push_back(channel);
    ++numOutputs;
}

void InOutInfo::removeOutput (int idx)
{
    outLocs.erase (outLocs.begin() + idx);
    outChannels.erase (outChannels.begin() + idx);
    --numOutputs;
}
