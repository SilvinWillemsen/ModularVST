/*
  ==============================================================================

    Global.h
    Created: 4 Sep 2021 3:30:39pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum Action
{
    noAction,
    
    addInstrumentAction,
    addResonatorModuleAction,
    addConnectionAction,
    cancelConnectionAction
};

enum ApplicationState
{
    normalState,
    addConnectionState,
    firstConnectionState
};

enum ResonatorModuleType
{
    stiffString = 1,
    bar,
    acousticTube,
    membrane,
    thinPlate,
};


namespace Global
{
    static int margin = 20;
    static int buttonHeight = 40;
    static int buttonWidth = 80;
    static int connRadius = 5;

    static const int listBoxRowHeight = 40;

    static NamedValueSet defaultStringParameters {
        {"L", 1.0},
        {"T", 300.0},
        {"rho", 7850.0},
        {"A", 0.0005 * 0.0005 * double_Pi},
        {"E", 2.0e11},
        {"I", pow (0.0005, 4) * double_Pi * 0.25},
        {"sig0", 1.0},
        {"sig1", 0.005}
    };

    static NamedValueSet defaultBarParameters {
        {"L", 1},
        {"rho", 7850},
        {"A", 0.0005 * 0.0005 * double_Pi},
        {"E", 2e11},
        {"I", pow (0.0005, 4) * double_Pi * 0.25},
        {"sig0", 1},
        {"sig1", 0.005}
    };

    static double limit (double val, double min, double max)
    {
        if (val < min)
        {
            val = min;
            return val;
        }
        else if (val > max)
        {
            val = max;
            return val;
        }
        return val;
    }

};
