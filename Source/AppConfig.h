/*
  ==============================================================================

    AppConfig.h
    Created: 16 May 2022 10:14:17am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
// Build configuration
// 0: Standalone (testing and creating presets),
// 1: Testing for Unity (Editor and sliders + all presets)
// 2: Build for Unity
#define BUILD_CONFIG 0

#include <JuceHeader.h>
namespace AppConfig
{
    static StringArray presetFilesToIncludeInUnity = {
        "Guitar_xml",
        "Harp_xml"
    };
};
