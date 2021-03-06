/*
  ==============================================================================

    Global.h
    Created: 4 Sep 2021 3:30:39pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include "AppConfig.h"
//#define USE_EIGEN     // use overlapping connections (and therefore the eigen library) or not
//#define CALC_ENERGY // calculate (and print) energy or not
//#define SAVE_OUTPUT

#if (BUILD_CONFIG == 1) // Testing for Unity
    #define EDITOR_AND_SLIDERS
    #define LOAD_ALL_UNITY_INSTRUMENTS
#elif (BUILD_CONFIG == 2) // Build for Unity
    #define NO_EDITOR
    #define LOAD_ALL_UNITY_INSTRUMENTS
#endif

#include <JuceHeader.h>

#ifdef USE_EIGEN
    #include "../eigen/Eigen/Eigen"
#endif

#ifdef SAVE_OUTPUT
    #include <iostream>
    #include <fstream>
#endif

// For extern "C" functions
#if _MSC_VER // this is defined when compiling with Visual Studio
    #define DLLExport __declspec(dllexport) // Visual Studio needs annotating exported functions with this
#else
    #define DLLExport __attribute__((visibility("default"))) // Mac does too, although google disagrees (possibly JUCE changes the visibility somehow)
#endif

#define USE_RESET_BUTTON

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
    editDensityAction,
    densitySliderAction,
    updateDensityAction,

    changeActiveConnectionAction,
    loadPresetAction,
    loadBinaryPresetAction,
    loadPresetFromWindowAction,
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
    editDensityState,
    moveConnectionState,
    firstConnectionState,
    editResonatorGroupsState

};

enum ResonatorModuleType
{
    noResonatorModule = 0, // set to 1 for the combo box options
    stiffString,
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
    static const bool loadPresetAtStartUp = true;

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
    static const double oneDOutputScaling = 0.05;
    static const double twoDOutputScaling = 1;
    static const double stringVisualScaling = 200.0;

    static const double excitationVisualWidth = 6;

    // default parameters
    static const double defaultLinSpringCoeff = 1e8;
    static const double defaultNonLinSpringCoeff = 1e10;
    static const double defaultConnDampCoeff = 0.01;
    static const double eps = 1e-15;

    static StringArray presetFilesToIncludeInUnity = AppConfig::presetFilesToIncludeInUnity;

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

    static StringArray densityInstructions = {
        "Click on a resonator to change its density."
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
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"rho", 1400},
        {"H", 2.54e-4},
        {"T", 2312.0}, // check whether this is good
        {"sig0", 1.0},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultMembraneParameters{
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"f0", 170},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultThinPlateParametersAdvanced {
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"rho", 7850},
        {"H", 0.0025},
        {"E", 1.14e10},
        {"nu", 0.3},
        {"sig0", 1},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultThinPlateParameters{
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"stiffness", 2.0},
        {"maxPoints", 2000}
    };

    static NamedValueSet defaultStiffMembraneParametersAdvanced {
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"rho", 1400},
        {"H", 2.54e-4},
        {"T", 2312.0}, // check whether this is good
        {"E", 0.76e9},
        {"nu", 0.3},
        {"sig0", 1},
        {"sig1", 0.005},
        {"maxPoints", 2000}
    };
    static NamedValueSet defaultStiffMembraneParameters{
        {"Lx", 0.75},
        {"Ly", 0.25},
        {"f0", 170},
        {"stiffness", 1.0},
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
            return min;
        }
        else if (val > max)
        {
            return max;
        }
        return val;
    }

//    static double interpolation (double* uVec, int bp, double alpha)
//    {
//        return uVec[bp];
//    }

    static double interpolation (double* uVec, int bp, double alpha)
    {
        return uVec[bp - 1] * (alpha * (alpha - 1) * (alpha - 2)) / -6.0
        + uVec[bp] * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0
        + uVec[bp + 1] * (alpha * (alpha + 1) * (alpha - 2)) / -2.0
        + uVec[bp + 2] * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
    }

//    static void extrapolation (double* uVec, int bp, double alpha, double val)
//    {
//        uVec[bp] = uVec[bp] + val;
//    }
    static void extrapolation (double* uVec, int bp, double alpha, double val)
    {
        uVec[bp - 1] = uVec[bp - 1] + val * (alpha * (alpha - 1) * (alpha - 2)) / -6.0;
        uVec[bp] = uVec[bp] + val * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0;
        uVec[bp + 1] = uVec[bp + 1] + val * (alpha * (alpha + 1) * (alpha - 2)) / -2.0;
        uVec[bp + 2] = uVec[bp + 2] + val * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;

    }

    static double interpolation2D (double* uVec, int bpX, int bpY, double alphaX, double alphaY, int Nx)
    {
        return (1.0 - alphaX) * (1.0 - alphaY) * uVec[bpX + bpY*Nx]
                    + (1.0 - alphaX) * alphaY * uVec[bpX + (bpY+1)*Nx]
                    + alphaX * (1.0 - alphaY) * uVec[bpX+1 + bpY*Nx]
                    + alphaX * alphaY * uVec[bpX+1 + (bpY+1)*Nx];
    }

    static void extrapolation2D (double* uVec, int bpX, int bpY, double alphaX, double alphaY, double val, int Nx)
    {
        uVec[bpX + bpY*Nx] = uVec[bpX + bpY*Nx] + val * (1.0 - alphaX) * (1.0 - alphaY);
        uVec[bpX + (bpY+1)*Nx] = uVec[bpX + (bpY+1)*Nx] + val * (1.0 - alphaX) * alphaY;
        uVec[bpX+1 + bpY*Nx] = uVec[bpX+1 + bpY*Nx] + val * alphaX * (1.0 - alphaY);
        uVec[bpX+1 + (bpY+1)*Nx] = uVec[bpX+1 + (bpY+1)*Nx] + val * alphaX * alphaY;

    }

};
