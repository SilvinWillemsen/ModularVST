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

void InOutInfo::addOutput (double loc, int channel)
{
    if (loc > 1) // then it's an integer (internal handling)
        outLocs.push_back(loc);
    else // preset handling
    {
        jassert (N.size() == 1);
        outLocs.push_back (round(loc * (N[0]+1)));
    }
    outChannels.push_back(channel);
    ++numOutputs;
}

void InOutInfo::addOutput (double locX, double locY, int channel)
{
    if (locX > 1) // then it's an integer (internal handling)
        outLocs.push_back(locX);
    else // preset handling
    {
        jassert (N.size() == 2);
        // LEFT OFF HERE!!
        std::cout << round(locX * (N[0]+1)) << std::endl;
        std::cout << round(locY * (N[1]+1)) << std::endl;

        outLocs.push_back (round(locX * (N[0]+1)) + (round(locY * (N[1]) * (N[0]+1))));
    }
    outChannels.push_back(channel);
    ++numOutputs;
}


void InOutInfo::removeOutput (int idx)
{
    outLocs.erase (outLocs.begin() + idx);
    outChannels.erase (outChannels.begin() + idx);
    --numOutputs;
}
