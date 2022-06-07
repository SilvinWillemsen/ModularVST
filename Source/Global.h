/*
  ==============================================================================

    Global.h
    Created: 4 Sep 2021 3:30:39pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
//#define USE_EIGEN     // use overlapping connections (and therefore the eigen library) or not
//#define CALC_ENERGY // calculate (and print) energy or not
//#define SAVE_OUTPUT
//#define NO_EDITOR // build for unity
//#define EDITOR_AND_SLIDERS

#define USE_RESET_BUTTON

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
    editResonatorModulesAction,
    removeResonatorModuleAction,
    
    editResonatorGroupsAction,
    addResonatorGroupAction,
    removeResonatorGroupAction,
    addResonatorToGroupAction,
//    interactWithGroupAction,
//    noInteractionWithGroupAction,

    editInOutputsAction,
    
    addConnectionAction, // only for presets
    addInputAction,
    addOutputAction,
    editConnectionAction,

    changeMassRatioAction,
    changeActiveConnectionAction,
    loadPresetAction,
    savePresetAction,
    savePresetFromWindowAction,
    exciteAction,
    changeExcitationAction,
    
    graphicsToggleAction,
    
    refreshEditorAction,
    setStatesToZeroAction
    
};

enum ApplicationState
{
    normalState,
    removeResonatorModuleState,
    editInOutputsState,
    editConnectionState,
    moveConnectionState,
    firstConnectionState,
    editResonatorGroupsState

};

enum ResonatorModuleType
{
    stiffString = 1, // set to 1 for the combo box options
    bar,
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

enum ExcitationType
{
    noExcitation = 0, // not included in the combo box options
    pluck,
    hammer,
    bow,
};

enum BoundaryCondition
{
    clampedBC,
    simplySupportedBC,
    freeBC,
};

enum PresetResult
{
    success,
    applicationIsNotEmpty,
    fileNotFound,
    loadingCancelled,
    presetNotLoaded
};

namespace Global
{
    static const bool showGraphicsToggle = true;

    static const bool loadPresetAtStartUp = false;
#ifdef NO_EDITOR
    static const bool loadFromBinary = true;
#else
    static const bool loadFromBinary = false;
#endif
    static const bool bowAtStartup = false;
    static const bool pluckAtStartup = false;
    static const int samplesToRecord = 1000;

    static const int margin = 10;
    static const int buttonHeight = 40;
    static const int buttonWidth = 80;
    static const int connRadius = 5;

    static const int selectionRadius = 10;
    static const int massRatioRadius = 5;
    static const int inOutputWidth = 4;
    static const int arrowHeight = 40;

    static const int listBoxRowHeight = 40;

    static const bool alwaysShowInOuts = false;
    static const double oneDOutputScaling = 1000.0;
    static const double twoDOutputScaling = 10000.0;
    static const double stringVisualScaling = 200.0;

    static const double excitationVisualWidth = 6;

    // default parameters
    static const double defaultLinSpringCoeff = 1e8;
    static const double defaultNonLinSpringCoeff = 1e10;
    static const double defaultConnDampCoeff = 0.0;
    static const double eps = 1e-15;


    static StringArray inOutInstructions = {
        "Left-Click: add a stereo output.",
        "Right-click: remove output.",
        "CTRL-click: add left output.",
        "ALT-click: add right output."
    };

    static StringArray connectionInstructions = {
        "Click-and-drag: move connection.",
        "Right-click: remove connection.",
        "CTRL-click: add connection."
    };

    static StringArray groupInstructions = {
        "Left-click: add to current group.",
        "Right-click: remove from group."
    };


    static NamedValueSet defaultStringParametersAdvanced {
        {"L", 1.0},
        {"T", 300.0},
        {"rho", 7850.0},
        {"A", 0.0005 * 0.0005 * double_Pi},
        {"E", 2e11},
        {"I", pow (0.0005, 4) * double_Pi * 0.25},
        {"sig0", 1.0},
        {"sig1", 0.005}
    };

    static NamedValueSet defaultStringParameters {
        {"f0", 110.00},
        {"r", 0.0005}
    };

    static NamedValueSet defaultBarParametersAdvanced {
        {"L", 0.5},
        {"rho", 1190},
        {"A", 0.004 * 0.004 * double_Pi},
        {"E", 3.2e9},
        {"I", pow (0.004, 4) * double_Pi * 0.25},
        {"sig0", 1},
        {"sig1", 0.05}
    };

    static NamedValueSet defaultBarParameters {
        {"L", 0.5},
        {"b", 2.69e-3},
        {"H", 0.0075},
        {"rho", 1190}
    };


    static NamedValueSet defaultMembraneParametersAdvanced {
        {"Lx", 0.5},
        {"Ly", 1},
        {"rho", 10},
        {"H", 0.001},
        {"T", 50}, // check whether this is good
        {"sig0", 0.5},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultThinPlateParametersAdvanced {
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

    static NamedValueSet defaultStiffMembraneParametersAdvanced {
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

    static NamedValueSet fullNames {
        {"L", "Length"},
        {"T", "Tension"},
        {"rho", "Density"},
        {"A", "Cross-sec. Area"},
        {"E", "Youngs Mod."},
        {"I", "Mom. of Inert."},
        {"sig0", "Freq. Indep. Damping"},
        {"sig1", "Freq. Dep. Damping"},
        {"Lx", "Length (hor)"},
        {"Ly", "Length (vert)"},
        {"H", "Thickness"},
        {"nu", "Poissons Ratio"},
        {"b", "Width"},
        {"f0", "Fundamental Freq."},
        {"r", "Radius"}
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

    static double interpolation (double* uVec, int bp, double alpha)
    {
        return uVec[bp - 1] * (alpha * (alpha - 1) * (alpha - 2)) / -6.0
        + uVec[bp] * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0
        + uVec[bp + 1] * (alpha * (alpha + 1) * (alpha - 2)) / -2.0
        + uVec[bp + 2] * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
    }

    static void extrapolation (double* uVec, int bp, double alpha, double val)
    {
        uVec[bp - 1] = uVec[bp - 1] + val * (alpha * (alpha - 1) * (alpha - 2)) / -6.0;
        uVec[bp] = uVec[bp] + val * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0;
        uVec[bp + 1] = uVec[bp + 1] + val * (alpha * (alpha + 1) * (alpha - 2)) / -2.0;
        uVec[bp + 2] = uVec[bp + 2] + val * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
        
    }
};
