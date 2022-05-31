/*
  ==============================================================================

    AppConfig.h
    Created: 16 May 2022 10:14:17am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
// 0: Build for Unity, 1: Testing for Unity (Editor and sliders + all presets), 2: Make a new preset
#define BUILD_CONFIG 2

#include <JuceHeader.h>
namespace AppConfig
{
    static StringArray presetFilesToIncludeInUnity = {
        "Guitar_xml",
        "Harp_xml",
        "BanjoLele_xml",
        "Timpani_xml",
        "Cello_xml",
        "Marimba_xml",
        "Shamisen_xml"
    };
};
