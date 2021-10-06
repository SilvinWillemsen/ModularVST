/*
  ==============================================================================

    Global.h
    Created: 4 Sep 2021 3:30:39pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../eigen/Eigen/Eigen"

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

enum ConnectionType
{
    rigid = 1,
    linearSpring,
    nonlinearSpring,
};

enum BoundaryCondition
{
    clampedBC,
    simplySupportedBC,
    freeBC,
};

namespace Global
{
    static const int margin = 20;
    static const int buttonHeight = 40;
    static const int buttonWidth = 80;
    static const int connRadius = 5;

    static const int listBoxRowHeight = 40;

    static const double defaultLinSpringCoeff = 1000.0;
    static const double defaultNonLinSpringCoeff = 1e8;
    static const double defaultConnDampCoeff = 0.0;
    static const double eps = 1e-15;

    static NamedValueSet defaultStringParameters {
        {"L", 1.0},
        {"T", 300.0},
        {"rho", 7850.0},
        {"A", 0.0005 * 0.0005 * double_Pi},
        {"E", 2e11},
        {"I", pow (0.0005, 4) * double_Pi * 0.25},
        {"sig0", 1.0},
        {"sig1", 0.005}
    };

    static NamedValueSet defaultBarParameters {
        {"L", 1},
        {"T", 0}, // just so that it can work with experimentation (can be removed eventually)
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
