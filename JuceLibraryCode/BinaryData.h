/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Marimba_xml;
    const int            Marimba_xmlSize = 25132;

    extern const char*   Timpani_xml;
    const int            Timpani_xmlSize = 1036;

    extern const char*   Cello_xml;
    const int            Cello_xmlSize = 2160;

    extern const char*   BanjoLele_xml;
    const int            BanjoLele_xmlSize = 6636;

    extern const char*   Guitar_xml;
    const int            Guitar_xmlSize = 7715;

    extern const char*   Harp_xml;
    const int            Harp_xmlSize = 19991;

    extern const char*   EmptyInstrument_xml;
    const int            EmptyInstrument_xmlSize = 52;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 7;

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
