/*
  ==============================================================================

    Global.h
    Created: 4 Sep 2021 3:30:39pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
//#define USE_EIGEN // use overlapping connections (and therefore the eigen library) or not
//#define CALC_ENERGY // calculate (and print) energy or not
//#define SAVE_OUTPUT

#include <JuceHeader.h>

#ifdef USE_EIGEN
    #include "../eigen/Eigen/Eigen"
#endif

#ifdef SAVE_OUTPUT
    #include <iostream>
    #include <fstream>
#endif

enum Action
{
    noAction,
    
    addInstrumentAction,
    addResonatorModuleAction,
    addResonatorModuleFromWindowAction,
    removeResonatorModuleAction,
    cancelRemoveResonatorModuleAction,
    editInOutputsAction,
    cancelInOutputsAction,
    editConnectionAction,
    changeMassRatioAction,
    changeActiveConnectionAction,
    cancelConnectionAction,
    savePresetAction
};

enum ApplicationState
{
    normalState,
    removeResonatorModuleState,
    editInOutputsState,
    editConnectionState,
    moveConnectionState,
    firstConnectionState
};

enum ResonatorModuleType
{
    stiffString = 1, // set to 1 for the combo box options
    bar,
    acousticTube,
    membrane,
    thinPlate,
    stiffMembrane,
};

enum ConnectionType
{
    rigid = 1, // set to 1 for the combo box options
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
    static const int selectionRadius = 10;
    static const int massRatioRadius = 5;
    static const int inOutputWidth = 4;

    static const int listBoxRowHeight = 40;

    // default parameters

    static const double defaultLinSpringCoeff = 1000.0;
    static const double defaultNonLinSpringCoeff = 1e8;
    static const double defaultConnDampCoeff = 0.0;
    static const double eps = 1e-15;

    static const double twoDOutputScaling = 10.0;

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
        {"rho", 7850},
        {"A", 0.004 * 0.004 * double_Pi},
        {"E", 2e11},
        {"I", pow (0.004, 4) * double_Pi * 0.25},
        {"sig0", 1},
        {"sig1", 0.005}
    };

    static NamedValueSet defaultMembraneParameters {
        {"Lx", 0.5},
        {"Ly", 1},
        {"rho", 10},
        {"H", 0.001},
        {"T", 50}, // check whether this is good
        {"sig0", 0.5},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultThinPlateParameters {
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"rho", 7850},
        {"H", 0.005},
        {"E", 2e11},
        {"nu", 0.3},
        {"sig0", 1},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultStiffMembraneParameters {
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"rho", 7850},
        {"H", 0.005},
        {"T", 100000}, // check whether this is good
        {"E", 2e11},
        {"nu", 0.3},
        {"sig0", 1},
        {"sig1", 0.005},
        {"maxPoints", 2000}
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
