/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   WeirdInstrument_xml;
    const int            WeirdInstrument_xmlSize = 4109;

    extern const char*   Harp_xml;
    const int            Harp_xmlSize = 7820;

    extern const char*   TwoStringsOctave_xml;
    const int            TwoStringsOctave_xmlSize = 1081;

    extern const char*   guitar_xml;
    const int            guitar_xmlSize = 7272;

    extern const char*   EmptyInstrument_xml;
    const int            EmptyInstrument_xmlSize = 52;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

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
