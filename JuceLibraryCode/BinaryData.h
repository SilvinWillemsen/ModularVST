/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   EmptyInstrument_xml;
    const int            EmptyInstrument_xmlSize = 52;

    extern const char*   guitar_xml;
    const int            guitar_xmlSize = 7272;

    extern const char*   GuitarWithoutBody_xml;
    const int            GuitarWithoutBody_xmlSize = 4805;

    extern const char*   Harp_xml;
    const int            Harp_xmlSize = 7820;

    extern const char*   HarpGroups_xml;
    const int            HarpGroups_xmlSize = 8148;

    extern const char*   lastPreset_txt;
    const int            lastPreset_txtSize = 14;

    extern const char*   lastSavedPreset_xml;
    const int            lastSavedPreset_xmlSize = 7904;

    extern const char*   SanturUnedited_xml;
    const int            SanturUnedited_xmlSize = 7706;

    extern const char*   Shamisen_xml;
    const int            Shamisen_xmlSize = 3768;

    extern const char*   SimpleString_xml;
    const int            SimpleString_xmlSize = 563;

    extern const char*   TwoStrings_xml;
    const int            TwoStrings_xmlSize = 1080;

    extern const char*   Ukulele_xml;
    const int            Ukulele_xmlSize = 564;

    extern const char*   WeirdInstrument_xml;
    const int            WeirdInstrument_xmlSize = 4109;

    extern const char*   WeirdInstrument2_xml;
    const int            WeirdInstrument2_xmlSize = 4810;

    extern const char*   LICENSE;
    const int            LICENSESize = 1068;

    extern const char*   README_md;
    const int            README_mdSize = 295;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 16;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
