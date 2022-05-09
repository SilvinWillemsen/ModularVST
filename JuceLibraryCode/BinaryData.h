/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Marimba_xml;
    const int            Marimba_xmlSize = 14778;

    extern const char*   Timpani_xml;
    const int            Timpani_xmlSize = 1036;

    extern const char*   Cello_xml;
    const int            Cello_xmlSize = 2160;

    extern const char*   BanjoLele_xml;
    const int            BanjoLele_xmlSize = 6636;

    extern const char*   Guitar_xml;
    const int            Guitar_xmlSize = 7715;

    extern const char*   WeirdInstrument_xml;
    const int            WeirdInstrument_xmlSize = 4109;

    extern const char*   Harp_xml;
    const int            Harp_xmlSize = 7820;

    extern const char*   TwoStringsOctave_xml;
    const int            TwoStringsOctave_xmlSize = 1081;

    extern const char*   TwoStrings_xml;
    const int            TwoStrings_xmlSize = 1082;

    extern const char*   TwoStringsConnected_xml;
    const int            TwoStringsConnected_xmlSize = 27101;

    extern const char*   TwoStringsConnectedNonlinear_xml;
    const int            TwoStringsConnectedNonlinear_xmlSize = 27569;

    extern const char*   Plate100_xml;
    const int            Plate100_xmlSize = 596;

    extern const char*   Plate400_xml;
    const int            Plate400_xmlSize = 598;

    extern const char*   Plate1600_xml;
    const int            Plate1600_xmlSize = 599;

    extern const char*   Plate2500_xml;
    const int            Plate2500_xmlSize = 599;

    extern const char*   TwoStringsConnectedLinear_xml;
    const int            TwoStringsConnectedLinear_xmlSize = 27218;

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
