//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  StringUtil.cpp
//
//  Low-level string functions.
//  Convert characters to Unicode, upper-/lowercase conversion, string formatting etc.
//

#if defined(_MSC_VER) // MSVC compiler.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include "utils/StringUtil.h"

#include <algorithm>
#include <codecvt>
#include <locale>

namespace Utils
{
    namespace String
    {
        // Unicode case conversion mapping table, based on data from this site:
        // https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_71/nls/rbagslowtoupmaptable.htm
        static wchar_t unicodeLowercase[] = {
            (wchar_t)0x0061, (wchar_t)0x0062, (wchar_t)0x0063, (wchar_t)0x0064, (wchar_t)0x0065,
            (wchar_t)0x0066, (wchar_t)0x0067, (wchar_t)0x0068, (wchar_t)0x0069, (wchar_t)0x006A,
            (wchar_t)0x006B, (wchar_t)0x006C, (wchar_t)0x006D, (wchar_t)0x006E, (wchar_t)0x006F,
            (wchar_t)0x0070, (wchar_t)0x0071, (wchar_t)0x0072, (wchar_t)0x0073, (wchar_t)0x0074,
            (wchar_t)0x0075, (wchar_t)0x0076, (wchar_t)0x0077, (wchar_t)0x0078, (wchar_t)0x0079,
            (wchar_t)0x007A, (wchar_t)0x00E0, (wchar_t)0x00E1, (wchar_t)0x00E2, (wchar_t)0x00E3,
            (wchar_t)0x00E4, (wchar_t)0x00E5, (wchar_t)0x00E6, (wchar_t)0x00E7, (wchar_t)0x00E8,
            (wchar_t)0x00E9, (wchar_t)0x00EA, (wchar_t)0x00EB, (wchar_t)0x00EC, (wchar_t)0x00ED,
            (wchar_t)0x00EE, (wchar_t)0x00EF, (wchar_t)0x00F0, (wchar_t)0x00F1, (wchar_t)0x00F2,
            (wchar_t)0x00F3, (wchar_t)0x00F4, (wchar_t)0x00F5, (wchar_t)0x00F6, (wchar_t)0x00F8,
            (wchar_t)0x00F9, (wchar_t)0x00FA, (wchar_t)0x00FB, (wchar_t)0x00FC, (wchar_t)0x00FD,
            (wchar_t)0x00FE, (wchar_t)0x00FF, (wchar_t)0x0101, (wchar_t)0x0103, (wchar_t)0x0105,
            (wchar_t)0x0107, (wchar_t)0x0109, (wchar_t)0x010B, (wchar_t)0x010D, (wchar_t)0x010F,
            (wchar_t)0x0111, (wchar_t)0x0113, (wchar_t)0x0115, (wchar_t)0x0117, (wchar_t)0x0119,
            (wchar_t)0x011B, (wchar_t)0x011D, (wchar_t)0x011F, (wchar_t)0x0121, (wchar_t)0x0123,
            (wchar_t)0x0125, (wchar_t)0x0127, (wchar_t)0x0129, (wchar_t)0x012B, (wchar_t)0x012D,
            (wchar_t)0x012F, (wchar_t)0x0131, (wchar_t)0x0133, (wchar_t)0x0135, (wchar_t)0x0137,
            (wchar_t)0x013A, (wchar_t)0x013C, (wchar_t)0x013E, (wchar_t)0x0140, (wchar_t)0x0142,
            (wchar_t)0x0144, (wchar_t)0x0146, (wchar_t)0x0148, (wchar_t)0x014B, (wchar_t)0x014D,
            (wchar_t)0x014F, (wchar_t)0x0151, (wchar_t)0x0153, (wchar_t)0x0155, (wchar_t)0x0157,
            (wchar_t)0x0159, (wchar_t)0x015B, (wchar_t)0x015D, (wchar_t)0x015F, (wchar_t)0x0161,
            (wchar_t)0x0163, (wchar_t)0x0165, (wchar_t)0x0167, (wchar_t)0x0169, (wchar_t)0x016B,
            (wchar_t)0x016D, (wchar_t)0x016F, (wchar_t)0x0171, (wchar_t)0x0173, (wchar_t)0x0175,
            (wchar_t)0x0177, (wchar_t)0x017A, (wchar_t)0x017C, (wchar_t)0x017E, (wchar_t)0x0183,
            (wchar_t)0x0185, (wchar_t)0x0188, (wchar_t)0x018C, (wchar_t)0x0192, (wchar_t)0x0199,
            (wchar_t)0x01A1, (wchar_t)0x01A3, (wchar_t)0x01A5, (wchar_t)0x01A8, (wchar_t)0x01AD,
            (wchar_t)0x01B0, (wchar_t)0x01B4, (wchar_t)0x01B6, (wchar_t)0x01B9, (wchar_t)0x01BD,
            (wchar_t)0x01C6, (wchar_t)0x01C9, (wchar_t)0x01CC, (wchar_t)0x01CE, (wchar_t)0x01D0,
            (wchar_t)0x01D2, (wchar_t)0x01D4, (wchar_t)0x01D6, (wchar_t)0x01D8, (wchar_t)0x01DA,
            (wchar_t)0x01DC, (wchar_t)0x01DF, (wchar_t)0x01E1, (wchar_t)0x01E3, (wchar_t)0x01E5,
            (wchar_t)0x01E7, (wchar_t)0x01E9, (wchar_t)0x01EB, (wchar_t)0x01ED, (wchar_t)0x01EF,
            (wchar_t)0x01F3, (wchar_t)0x01F5, (wchar_t)0x01FB, (wchar_t)0x01FD, (wchar_t)0x01FF,
            (wchar_t)0x0201, (wchar_t)0x0203, (wchar_t)0x0205, (wchar_t)0x0207, (wchar_t)0x0209,
            (wchar_t)0x020B, (wchar_t)0x020D, (wchar_t)0x020F, (wchar_t)0x0211, (wchar_t)0x0213,
            (wchar_t)0x0215, (wchar_t)0x0217, (wchar_t)0x0253, (wchar_t)0x0254, (wchar_t)0x0257,
            (wchar_t)0x0258, (wchar_t)0x0259, (wchar_t)0x025B, (wchar_t)0x0260, (wchar_t)0x0263,
            (wchar_t)0x0268, (wchar_t)0x0269, (wchar_t)0x026F, (wchar_t)0x0272, (wchar_t)0x0275,
            (wchar_t)0x0283, (wchar_t)0x0288, (wchar_t)0x028A, (wchar_t)0x028B, (wchar_t)0x0292,
            (wchar_t)0x03AC, (wchar_t)0x03AD, (wchar_t)0x03AE, (wchar_t)0x03AF, (wchar_t)0x03B1,
            (wchar_t)0x03B2, (wchar_t)0x03B3, (wchar_t)0x03B4, (wchar_t)0x03B5, (wchar_t)0x03B6,
            (wchar_t)0x03B7, (wchar_t)0x03B8, (wchar_t)0x03B9, (wchar_t)0x03BA, (wchar_t)0x03BB,
            (wchar_t)0x03BC, (wchar_t)0x03BD, (wchar_t)0x03BE, (wchar_t)0x03BF, (wchar_t)0x03C0,
            (wchar_t)0x03C1, (wchar_t)0x03C3, (wchar_t)0x03C4, (wchar_t)0x03C5, (wchar_t)0x03C6,
            (wchar_t)0x03C7, (wchar_t)0x03C8, (wchar_t)0x03C9, (wchar_t)0x03CA, (wchar_t)0x03CB,
            (wchar_t)0x03CC, (wchar_t)0x03CD, (wchar_t)0x03CE, (wchar_t)0x03E3, (wchar_t)0x03E5,
            (wchar_t)0x03E7, (wchar_t)0x03E9, (wchar_t)0x03EB, (wchar_t)0x03ED, (wchar_t)0x03EF,
            (wchar_t)0x0430, (wchar_t)0x0431, (wchar_t)0x0432, (wchar_t)0x0433, (wchar_t)0x0434,
            (wchar_t)0x0435, (wchar_t)0x0436, (wchar_t)0x0437, (wchar_t)0x0438, (wchar_t)0x0439,
            (wchar_t)0x043A, (wchar_t)0x043B, (wchar_t)0x043C, (wchar_t)0x043D, (wchar_t)0x043E,
            (wchar_t)0x043F, (wchar_t)0x0440, (wchar_t)0x0441, (wchar_t)0x0442, (wchar_t)0x0443,
            (wchar_t)0x0444, (wchar_t)0x0445, (wchar_t)0x0446, (wchar_t)0x0447, (wchar_t)0x0448,
            (wchar_t)0x0449, (wchar_t)0x044A, (wchar_t)0x044B, (wchar_t)0x044C, (wchar_t)0x044D,
            (wchar_t)0x044E, (wchar_t)0x044F, (wchar_t)0x0451, (wchar_t)0x0452, (wchar_t)0x0453,
            (wchar_t)0x0454, (wchar_t)0x0455, (wchar_t)0x0456, (wchar_t)0x0457, (wchar_t)0x0458,
            (wchar_t)0x0459, (wchar_t)0x045A, (wchar_t)0x045B, (wchar_t)0x045C, (wchar_t)0x045E,
            (wchar_t)0x045F, (wchar_t)0x0461, (wchar_t)0x0463, (wchar_t)0x0465, (wchar_t)0x0467,
            (wchar_t)0x0469, (wchar_t)0x046B, (wchar_t)0x046D, (wchar_t)0x046F, (wchar_t)0x0471,
            (wchar_t)0x0473, (wchar_t)0x0475, (wchar_t)0x0477, (wchar_t)0x0479, (wchar_t)0x047B,
            (wchar_t)0x047D, (wchar_t)0x047F, (wchar_t)0x0481, (wchar_t)0x0491, (wchar_t)0x0493,
            (wchar_t)0x0495, (wchar_t)0x0497, (wchar_t)0x0499, (wchar_t)0x049B, (wchar_t)0x049D,
            (wchar_t)0x049F, (wchar_t)0x04A1, (wchar_t)0x04A3, (wchar_t)0x04A5, (wchar_t)0x04A7,
            (wchar_t)0x04A9, (wchar_t)0x04AB, (wchar_t)0x04AD, (wchar_t)0x04AF, (wchar_t)0x04B1,
            (wchar_t)0x04B3, (wchar_t)0x04B5, (wchar_t)0x04B7, (wchar_t)0x04B9, (wchar_t)0x04BB,
            (wchar_t)0x04BD, (wchar_t)0x04BF, (wchar_t)0x04C2, (wchar_t)0x04C4, (wchar_t)0x04C8,
            (wchar_t)0x04CC, (wchar_t)0x04D1, (wchar_t)0x04D3, (wchar_t)0x04D5, (wchar_t)0x04D7,
            (wchar_t)0x04D9, (wchar_t)0x04DB, (wchar_t)0x04DD, (wchar_t)0x04DF, (wchar_t)0x04E1,
            (wchar_t)0x04E3, (wchar_t)0x04E5, (wchar_t)0x04E7, (wchar_t)0x04E9, (wchar_t)0x04EB,
            (wchar_t)0x04EF, (wchar_t)0x04F1, (wchar_t)0x04F3, (wchar_t)0x04F5, (wchar_t)0x04F9,
            (wchar_t)0x0561, (wchar_t)0x0562, (wchar_t)0x0563, (wchar_t)0x0564, (wchar_t)0x0565,
            (wchar_t)0x0566, (wchar_t)0x0567, (wchar_t)0x0568, (wchar_t)0x0569, (wchar_t)0x056A,
            (wchar_t)0x056B, (wchar_t)0x056C, (wchar_t)0x056D, (wchar_t)0x056E, (wchar_t)0x056F,
            (wchar_t)0x0570, (wchar_t)0x0571, (wchar_t)0x0572, (wchar_t)0x0573, (wchar_t)0x0574,
            (wchar_t)0x0575, (wchar_t)0x0576, (wchar_t)0x0577, (wchar_t)0x0578, (wchar_t)0x0579,
            (wchar_t)0x057A, (wchar_t)0x057B, (wchar_t)0x057C, (wchar_t)0x057D, (wchar_t)0x057E,
            (wchar_t)0x057F, (wchar_t)0x0580, (wchar_t)0x0581, (wchar_t)0x0582, (wchar_t)0x0583,
            (wchar_t)0x0584, (wchar_t)0x0585, (wchar_t)0x0586, (wchar_t)0x10D0, (wchar_t)0x10D1,
            (wchar_t)0x10D2, (wchar_t)0x10D3, (wchar_t)0x10D4, (wchar_t)0x10D5, (wchar_t)0x10D6,
            (wchar_t)0x10D7, (wchar_t)0x10D8, (wchar_t)0x10D9, (wchar_t)0x10DA, (wchar_t)0x10DB,
            (wchar_t)0x10DC, (wchar_t)0x10DD, (wchar_t)0x10DE, (wchar_t)0x10DF, (wchar_t)0x10E0,
            (wchar_t)0x10E1, (wchar_t)0x10E2, (wchar_t)0x10E3, (wchar_t)0x10E4, (wchar_t)0x10E5,
            (wchar_t)0x10E6, (wchar_t)0x10E7, (wchar_t)0x10E8, (wchar_t)0x10E9, (wchar_t)0x10EA,
            (wchar_t)0x10EB, (wchar_t)0x10EC, (wchar_t)0x10ED, (wchar_t)0x10EE, (wchar_t)0x10EF,
            (wchar_t)0x10F0, (wchar_t)0x10F1, (wchar_t)0x10F2, (wchar_t)0x10F3, (wchar_t)0x10F4,
            (wchar_t)0x10F5, (wchar_t)0x1E01, (wchar_t)0x1E03, (wchar_t)0x1E05, (wchar_t)0x1E07,
            (wchar_t)0x1E09, (wchar_t)0x1E0B, (wchar_t)0x1E0D, (wchar_t)0x1E0F, (wchar_t)0x1E11,
            (wchar_t)0x1E13, (wchar_t)0x1E15, (wchar_t)0x1E17, (wchar_t)0x1E19, (wchar_t)0x1E1B,
            (wchar_t)0x1E1D, (wchar_t)0x1E1F, (wchar_t)0x1E21, (wchar_t)0x1E23, (wchar_t)0x1E25,
            (wchar_t)0x1E27, (wchar_t)0x1E29, (wchar_t)0x1E2B, (wchar_t)0x1E2D, (wchar_t)0x1E2F,
            (wchar_t)0x1E31, (wchar_t)0x1E33, (wchar_t)0x1E35, (wchar_t)0x1E37, (wchar_t)0x1E39,
            (wchar_t)0x1E3B, (wchar_t)0x1E3D, (wchar_t)0x1E3F, (wchar_t)0x1E41, (wchar_t)0x1E43,
            (wchar_t)0x1E45, (wchar_t)0x1E47, (wchar_t)0x1E49, (wchar_t)0x1E4B, (wchar_t)0x1E4D,
            (wchar_t)0x1E4F, (wchar_t)0x1E51, (wchar_t)0x1E53, (wchar_t)0x1E55, (wchar_t)0x1E57,
            (wchar_t)0x1E59, (wchar_t)0x1E5B, (wchar_t)0x1E5D, (wchar_t)0x1E5F, (wchar_t)0x1E61,
            (wchar_t)0x1E63, (wchar_t)0x1E65, (wchar_t)0x1E67, (wchar_t)0x1E69, (wchar_t)0x1E6B,
            (wchar_t)0x1E6D, (wchar_t)0x1E6F, (wchar_t)0x1E71, (wchar_t)0x1E73, (wchar_t)0x1E75,
            (wchar_t)0x1E77, (wchar_t)0x1E79, (wchar_t)0x1E7B, (wchar_t)0x1E7D, (wchar_t)0x1E7F,
            (wchar_t)0x1E81, (wchar_t)0x1E83, (wchar_t)0x1E85, (wchar_t)0x1E87, (wchar_t)0x1E89,
            (wchar_t)0x1E8B, (wchar_t)0x1E8D, (wchar_t)0x1E8F, (wchar_t)0x1E91, (wchar_t)0x1E93,
            (wchar_t)0x1E95, (wchar_t)0x1EA1, (wchar_t)0x1EA3, (wchar_t)0x1EA5, (wchar_t)0x1EA7,
            (wchar_t)0x1EA9, (wchar_t)0x1EAB, (wchar_t)0x1EAD, (wchar_t)0x1EAF, (wchar_t)0x1EB1,
            (wchar_t)0x1EB3, (wchar_t)0x1EB5, (wchar_t)0x1EB7, (wchar_t)0x1EB9, (wchar_t)0x1EBB,
            (wchar_t)0x1EBD, (wchar_t)0x1EBF, (wchar_t)0x1EC1, (wchar_t)0x1EC3, (wchar_t)0x1EC5,
            (wchar_t)0x1EC7, (wchar_t)0x1EC9, (wchar_t)0x1ECB, (wchar_t)0x1ECD, (wchar_t)0x1ECF,
            (wchar_t)0x1ED1, (wchar_t)0x1ED3, (wchar_t)0x1ED5, (wchar_t)0x1ED7, (wchar_t)0x1ED9,
            (wchar_t)0x1EDB, (wchar_t)0x1EDD, (wchar_t)0x1EDF, (wchar_t)0x1EE1, (wchar_t)0x1EE3,
            (wchar_t)0x1EE5, (wchar_t)0x1EE7, (wchar_t)0x1EE9, (wchar_t)0x1EEB, (wchar_t)0x1EED,
            (wchar_t)0x1EEF, (wchar_t)0x1EF1, (wchar_t)0x1EF3, (wchar_t)0x1EF5, (wchar_t)0x1EF7,
            (wchar_t)0x1EF9, (wchar_t)0x1F00, (wchar_t)0x1F01, (wchar_t)0x1F02, (wchar_t)0x1F03,
            (wchar_t)0x1F04, (wchar_t)0x1F05, (wchar_t)0x1F06, (wchar_t)0x1F07, (wchar_t)0x1F10,
            (wchar_t)0x1F11, (wchar_t)0x1F12, (wchar_t)0x1F13, (wchar_t)0x1F14, (wchar_t)0x1F15,
            (wchar_t)0x1F20, (wchar_t)0x1F21, (wchar_t)0x1F22, (wchar_t)0x1F23, (wchar_t)0x1F24,
            (wchar_t)0x1F25, (wchar_t)0x1F26, (wchar_t)0x1F27, (wchar_t)0x1F30, (wchar_t)0x1F31,
            (wchar_t)0x1F32, (wchar_t)0x1F33, (wchar_t)0x1F34, (wchar_t)0x1F35, (wchar_t)0x1F36,
            (wchar_t)0x1F37, (wchar_t)0x1F40, (wchar_t)0x1F41, (wchar_t)0x1F42, (wchar_t)0x1F43,
            (wchar_t)0x1F44, (wchar_t)0x1F45, (wchar_t)0x1F51, (wchar_t)0x1F53, (wchar_t)0x1F55,
            (wchar_t)0x1F57, (wchar_t)0x1F60, (wchar_t)0x1F61, (wchar_t)0x1F62, (wchar_t)0x1F63,
            (wchar_t)0x1F64, (wchar_t)0x1F65, (wchar_t)0x1F66, (wchar_t)0x1F67, (wchar_t)0x1F80,
            (wchar_t)0x1F81, (wchar_t)0x1F82, (wchar_t)0x1F83, (wchar_t)0x1F84, (wchar_t)0x1F85,
            (wchar_t)0x1F86, (wchar_t)0x1F87, (wchar_t)0x1F90, (wchar_t)0x1F91, (wchar_t)0x1F92,
            (wchar_t)0x1F93, (wchar_t)0x1F94, (wchar_t)0x1F95, (wchar_t)0x1F96, (wchar_t)0x1F97,
            (wchar_t)0x1FA0, (wchar_t)0x1FA1, (wchar_t)0x1FA2, (wchar_t)0x1FA3, (wchar_t)0x1FA4,
            (wchar_t)0x1FA5, (wchar_t)0x1FA6, (wchar_t)0x1FA7, (wchar_t)0x1FB0, (wchar_t)0x1FB1,
            (wchar_t)0x1FD0, (wchar_t)0x1FD1, (wchar_t)0x1FE0, (wchar_t)0x1FE1, (wchar_t)0x24D0,
            (wchar_t)0x24D1, (wchar_t)0x24D2, (wchar_t)0x24D3, (wchar_t)0x24D4, (wchar_t)0x24D5,
            (wchar_t)0x24D6, (wchar_t)0x24D7, (wchar_t)0x24D8, (wchar_t)0x24D9, (wchar_t)0x24DA,
            (wchar_t)0x24DB, (wchar_t)0x24DC, (wchar_t)0x24DD, (wchar_t)0x24DE, (wchar_t)0x24DF,
            (wchar_t)0x24E0, (wchar_t)0x24E1, (wchar_t)0x24E2, (wchar_t)0x24E3, (wchar_t)0x24E4,
            (wchar_t)0x24E5, (wchar_t)0x24E6, (wchar_t)0x24E7, (wchar_t)0x24E8, (wchar_t)0x24E9,
            (wchar_t)0xFF41, (wchar_t)0xFF42, (wchar_t)0xFF43, (wchar_t)0xFF44, (wchar_t)0xFF45,
            (wchar_t)0xFF46, (wchar_t)0xFF47, (wchar_t)0xFF48, (wchar_t)0xFF49, (wchar_t)0xFF4A,
            (wchar_t)0xFF4B, (wchar_t)0xFF4C, (wchar_t)0xFF4D, (wchar_t)0xFF4E, (wchar_t)0xFF4F,
            (wchar_t)0xFF50, (wchar_t)0xFF51, (wchar_t)0xFF52, (wchar_t)0xFF53, (wchar_t)0xFF54,
            (wchar_t)0xFF55, (wchar_t)0xFF56, (wchar_t)0xFF57, (wchar_t)0xFF58, (wchar_t)0xFF59,
            (wchar_t)0xFF5A, (wchar_t)0x0000};

        static wchar_t unicodeUppercase[] = {
            (wchar_t)0x0041, (wchar_t)0x0042, (wchar_t)0x0043, (wchar_t)0x0044, (wchar_t)0x0045,
            (wchar_t)0x0046, (wchar_t)0x0047, (wchar_t)0x0048, (wchar_t)0x0049, (wchar_t)0x004A,
            (wchar_t)0x004B, (wchar_t)0x004C, (wchar_t)0x004D, (wchar_t)0x004E, (wchar_t)0x004F,
            (wchar_t)0x0050, (wchar_t)0x0051, (wchar_t)0x0052, (wchar_t)0x0053, (wchar_t)0x0054,
            (wchar_t)0x0055, (wchar_t)0x0056, (wchar_t)0x0057, (wchar_t)0x0058, (wchar_t)0x0059,
            (wchar_t)0x005A, (wchar_t)0x00C0, (wchar_t)0x00C1, (wchar_t)0x00C2, (wchar_t)0x00C3,
            (wchar_t)0x00C4, (wchar_t)0x00C5, (wchar_t)0x00C6, (wchar_t)0x00C7, (wchar_t)0x00C8,
            (wchar_t)0x00C9, (wchar_t)0x00CA, (wchar_t)0x00CB, (wchar_t)0x00CC, (wchar_t)0x00CD,
            (wchar_t)0x00CE, (wchar_t)0x00CF, (wchar_t)0x00D0, (wchar_t)0x00D1, (wchar_t)0x00D2,
            (wchar_t)0x00D3, (wchar_t)0x00D4, (wchar_t)0x00D5, (wchar_t)0x00D6, (wchar_t)0x00D8,
            (wchar_t)0x00D9, (wchar_t)0x00DA, (wchar_t)0x00DB, (wchar_t)0x00DC, (wchar_t)0x00DD,
            (wchar_t)0x00DE, (wchar_t)0x0178, (wchar_t)0x0100, (wchar_t)0x0102, (wchar_t)0x0104,
            (wchar_t)0x0106, (wchar_t)0x0108, (wchar_t)0x010A, (wchar_t)0x010C, (wchar_t)0x010E,
            (wchar_t)0x0110, (wchar_t)0x0112, (wchar_t)0x0114, (wchar_t)0x0116, (wchar_t)0x0118,
            (wchar_t)0x011A, (wchar_t)0x011C, (wchar_t)0x011E, (wchar_t)0x0120, (wchar_t)0x0122,
            (wchar_t)0x0124, (wchar_t)0x0126, (wchar_t)0x0128, (wchar_t)0x012A, (wchar_t)0x012C,
            (wchar_t)0x012E, (wchar_t)0x0049, (wchar_t)0x0132, (wchar_t)0x0134, (wchar_t)0x0136,
            (wchar_t)0x0139, (wchar_t)0x013B, (wchar_t)0x013D, (wchar_t)0x013F, (wchar_t)0x0141,
            (wchar_t)0x0143, (wchar_t)0x0145, (wchar_t)0x0147, (wchar_t)0x014A, (wchar_t)0x014C,
            (wchar_t)0x014E, (wchar_t)0x0150, (wchar_t)0x0152, (wchar_t)0x0154, (wchar_t)0x0156,
            (wchar_t)0x0158, (wchar_t)0x015A, (wchar_t)0x015C, (wchar_t)0x015E, (wchar_t)0x0160,
            (wchar_t)0x0162, (wchar_t)0x0164, (wchar_t)0x0166, (wchar_t)0x0168, (wchar_t)0x016A,
            (wchar_t)0x016C, (wchar_t)0x016E, (wchar_t)0x0170, (wchar_t)0x0172, (wchar_t)0x0174,
            (wchar_t)0x0176, (wchar_t)0x0179, (wchar_t)0x017B, (wchar_t)0x017D, (wchar_t)0x0182,
            (wchar_t)0x0184, (wchar_t)0x0187, (wchar_t)0x018B, (wchar_t)0x0191, (wchar_t)0x0198,
            (wchar_t)0x01A0, (wchar_t)0x01A2, (wchar_t)0x01A4, (wchar_t)0x01A7, (wchar_t)0x01AC,
            (wchar_t)0x01AF, (wchar_t)0x01B3, (wchar_t)0x01B5, (wchar_t)0x01B8, (wchar_t)0x01BC,
            (wchar_t)0x01C4, (wchar_t)0x01C7, (wchar_t)0x01CA, (wchar_t)0x01CD, (wchar_t)0x01CF,
            (wchar_t)0x01D1, (wchar_t)0x01D3, (wchar_t)0x01D5, (wchar_t)0x01D7, (wchar_t)0x01D9,
            (wchar_t)0x01DB, (wchar_t)0x01DE, (wchar_t)0x01E0, (wchar_t)0x01E2, (wchar_t)0x01E4,
            (wchar_t)0x01E6, (wchar_t)0x01E8, (wchar_t)0x01EA, (wchar_t)0x01EC, (wchar_t)0x01EE,
            (wchar_t)0x01F1, (wchar_t)0x01F4, (wchar_t)0x01FA, (wchar_t)0x01FC, (wchar_t)0x01FE,
            (wchar_t)0x0200, (wchar_t)0x0202, (wchar_t)0x0204, (wchar_t)0x0206, (wchar_t)0x0208,
            (wchar_t)0x020A, (wchar_t)0x020C, (wchar_t)0x020E, (wchar_t)0x0210, (wchar_t)0x0212,
            (wchar_t)0x0214, (wchar_t)0x0216, (wchar_t)0x0181, (wchar_t)0x0186, (wchar_t)0x018A,
            (wchar_t)0x018E, (wchar_t)0x018F, (wchar_t)0x0190, (wchar_t)0x0193, (wchar_t)0x0194,
            (wchar_t)0x0197, (wchar_t)0x0196, (wchar_t)0x019C, (wchar_t)0x019D, (wchar_t)0x019F,
            (wchar_t)0x01A9, (wchar_t)0x01AE, (wchar_t)0x01B1, (wchar_t)0x01B2, (wchar_t)0x01B7,
            (wchar_t)0x0386, (wchar_t)0x0388, (wchar_t)0x0389, (wchar_t)0x038A, (wchar_t)0x0391,
            (wchar_t)0x0392, (wchar_t)0x0393, (wchar_t)0x0394, (wchar_t)0x0395, (wchar_t)0x0396,
            (wchar_t)0x0397, (wchar_t)0x0398, (wchar_t)0x0399, (wchar_t)0x039A, (wchar_t)0x039B,
            (wchar_t)0x039C, (wchar_t)0x039D, (wchar_t)0x039E, (wchar_t)0x039F, (wchar_t)0x03A0,
            (wchar_t)0x03A1, (wchar_t)0x03A3, (wchar_t)0x03A4, (wchar_t)0x03A5, (wchar_t)0x03A6,
            (wchar_t)0x03A7, (wchar_t)0x03A8, (wchar_t)0x03A9, (wchar_t)0x03AA, (wchar_t)0x03AB,
            (wchar_t)0x038C, (wchar_t)0x038E, (wchar_t)0x038F, (wchar_t)0x03E2, (wchar_t)0x03E4,
            (wchar_t)0x03E6, (wchar_t)0x03E8, (wchar_t)0x03EA, (wchar_t)0x03EC, (wchar_t)0x03EE,
            (wchar_t)0x0410, (wchar_t)0x0411, (wchar_t)0x0412, (wchar_t)0x0413, (wchar_t)0x0414,
            (wchar_t)0x0415, (wchar_t)0x0416, (wchar_t)0x0417, (wchar_t)0x0418, (wchar_t)0x0419,
            (wchar_t)0x041A, (wchar_t)0x041B, (wchar_t)0x041C, (wchar_t)0x041D, (wchar_t)0x041E,
            (wchar_t)0x041F, (wchar_t)0x0420, (wchar_t)0x0421, (wchar_t)0x0422, (wchar_t)0x0423,
            (wchar_t)0x0424, (wchar_t)0x0425, (wchar_t)0x0426, (wchar_t)0x0427, (wchar_t)0x0428,
            (wchar_t)0x0429, (wchar_t)0x042A, (wchar_t)0x042B, (wchar_t)0x042C, (wchar_t)0x042D,
            (wchar_t)0x042E, (wchar_t)0x042F, (wchar_t)0x0401, (wchar_t)0x0402, (wchar_t)0x0403,
            (wchar_t)0x0404, (wchar_t)0x0405, (wchar_t)0x0406, (wchar_t)0x0407, (wchar_t)0x0408,
            (wchar_t)0x0409, (wchar_t)0x040A, (wchar_t)0x040B, (wchar_t)0x040C, (wchar_t)0x040E,
            (wchar_t)0x040F, (wchar_t)0x0460, (wchar_t)0x0462, (wchar_t)0x0464, (wchar_t)0x0466,
            (wchar_t)0x0468, (wchar_t)0x046A, (wchar_t)0x046C, (wchar_t)0x046E, (wchar_t)0x0470,
            (wchar_t)0x0472, (wchar_t)0x0474, (wchar_t)0x0476, (wchar_t)0x0478, (wchar_t)0x047A,
            (wchar_t)0x047C, (wchar_t)0x047E, (wchar_t)0x0480, (wchar_t)0x0490, (wchar_t)0x0492,
            (wchar_t)0x0494, (wchar_t)0x0496, (wchar_t)0x0498, (wchar_t)0x049A, (wchar_t)0x049C,
            (wchar_t)0x049E, (wchar_t)0x04A0, (wchar_t)0x04A2, (wchar_t)0x04A4, (wchar_t)0x04A6,
            (wchar_t)0x04A8, (wchar_t)0x04AA, (wchar_t)0x04AC, (wchar_t)0x04AE, (wchar_t)0x04B0,
            (wchar_t)0x04B2, (wchar_t)0x04B4, (wchar_t)0x04B6, (wchar_t)0x04B8, (wchar_t)0x04BA,
            (wchar_t)0x04BC, (wchar_t)0x04BE, (wchar_t)0x04C1, (wchar_t)0x04C3, (wchar_t)0x04C7,
            (wchar_t)0x04CB, (wchar_t)0x04D0, (wchar_t)0x04D2, (wchar_t)0x04D4, (wchar_t)0x04D6,
            (wchar_t)0x04D8, (wchar_t)0x04DA, (wchar_t)0x04DC, (wchar_t)0x04DE, (wchar_t)0x04E0,
            (wchar_t)0x04E2, (wchar_t)0x04E4, (wchar_t)0x04E6, (wchar_t)0x04E8, (wchar_t)0x04EA,
            (wchar_t)0x04EE, (wchar_t)0x04F0, (wchar_t)0x04F2, (wchar_t)0x04F4, (wchar_t)0x04F8,
            (wchar_t)0x0531, (wchar_t)0x0532, (wchar_t)0x0533, (wchar_t)0x0534, (wchar_t)0x0535,
            (wchar_t)0x0536, (wchar_t)0x0537, (wchar_t)0x0538, (wchar_t)0x0539, (wchar_t)0x053A,
            (wchar_t)0x053B, (wchar_t)0x053C, (wchar_t)0x053D, (wchar_t)0x053E, (wchar_t)0x053F,
            (wchar_t)0x0540, (wchar_t)0x0541, (wchar_t)0x0542, (wchar_t)0x0543, (wchar_t)0x0544,
            (wchar_t)0x0545, (wchar_t)0x0546, (wchar_t)0x0547, (wchar_t)0x0548, (wchar_t)0x0549,
            (wchar_t)0x054A, (wchar_t)0x054B, (wchar_t)0x054C, (wchar_t)0x054D, (wchar_t)0x054E,
            (wchar_t)0x054F, (wchar_t)0x0550, (wchar_t)0x0551, (wchar_t)0x0552, (wchar_t)0x0553,
            (wchar_t)0x0554, (wchar_t)0x0555, (wchar_t)0x0556, (wchar_t)0x10A0, (wchar_t)0x10A1,
            (wchar_t)0x10A2, (wchar_t)0x10A3, (wchar_t)0x10A4, (wchar_t)0x10A5, (wchar_t)0x10A6,
            (wchar_t)0x10A7, (wchar_t)0x10A8, (wchar_t)0x10A9, (wchar_t)0x10AA, (wchar_t)0x10AB,
            (wchar_t)0x10AC, (wchar_t)0x10AD, (wchar_t)0x10AE, (wchar_t)0x10AF, (wchar_t)0x10B0,
            (wchar_t)0x10B1, (wchar_t)0x10B2, (wchar_t)0x10B3, (wchar_t)0x10B4, (wchar_t)0x10B5,
            (wchar_t)0x10B6, (wchar_t)0x10B7, (wchar_t)0x10B8, (wchar_t)0x10B9, (wchar_t)0x10BA,
            (wchar_t)0x10BB, (wchar_t)0x10BC, (wchar_t)0x10BD, (wchar_t)0x10BE, (wchar_t)0x10BF,
            (wchar_t)0x10C0, (wchar_t)0x10C1, (wchar_t)0x10C2, (wchar_t)0x10C3, (wchar_t)0x10C4,
            (wchar_t)0x10C5, (wchar_t)0x1E00, (wchar_t)0x1E02, (wchar_t)0x1E04, (wchar_t)0x1E06,
            (wchar_t)0x1E08, (wchar_t)0x1E0A, (wchar_t)0x1E0C, (wchar_t)0x1E0E, (wchar_t)0x1E10,
            (wchar_t)0x1E12, (wchar_t)0x1E14, (wchar_t)0x1E16, (wchar_t)0x1E18, (wchar_t)0x1E1A,
            (wchar_t)0x1E1C, (wchar_t)0x1E1E, (wchar_t)0x1E20, (wchar_t)0x1E22, (wchar_t)0x1E24,
            (wchar_t)0x1E26, (wchar_t)0x1E28, (wchar_t)0x1E2A, (wchar_t)0x1E2C, (wchar_t)0x1E2E,
            (wchar_t)0x1E30, (wchar_t)0x1E32, (wchar_t)0x1E34, (wchar_t)0x1E36, (wchar_t)0x1E38,
            (wchar_t)0x1E3A, (wchar_t)0x1E3C, (wchar_t)0x1E3E, (wchar_t)0x1E40, (wchar_t)0x1E42,
            (wchar_t)0x1E44, (wchar_t)0x1E46, (wchar_t)0x1E48, (wchar_t)0x1E4A, (wchar_t)0x1E4C,
            (wchar_t)0x1E4E, (wchar_t)0x1E50, (wchar_t)0x1E52, (wchar_t)0x1E54, (wchar_t)0x1E56,
            (wchar_t)0x1E58, (wchar_t)0x1E5A, (wchar_t)0x1E5C, (wchar_t)0x1E5E, (wchar_t)0x1E60,
            (wchar_t)0x1E62, (wchar_t)0x1E64, (wchar_t)0x1E66, (wchar_t)0x1E68, (wchar_t)0x1E6A,
            (wchar_t)0x1E6C, (wchar_t)0x1E6E, (wchar_t)0x1E70, (wchar_t)0x1E72, (wchar_t)0x1E74,
            (wchar_t)0x1E76, (wchar_t)0x1E78, (wchar_t)0x1E7A, (wchar_t)0x1E7C, (wchar_t)0x1E7E,
            (wchar_t)0x1E80, (wchar_t)0x1E82, (wchar_t)0x1E84, (wchar_t)0x1E86, (wchar_t)0x1E88,
            (wchar_t)0x1E8A, (wchar_t)0x1E8C, (wchar_t)0x1E8E, (wchar_t)0x1E90, (wchar_t)0x1E92,
            (wchar_t)0x1E94, (wchar_t)0x1EA0, (wchar_t)0x1EA2, (wchar_t)0x1EA4, (wchar_t)0x1EA6,
            (wchar_t)0x1EA8, (wchar_t)0x1EAA, (wchar_t)0x1EAC, (wchar_t)0x1EAE, (wchar_t)0x1EB0,
            (wchar_t)0x1EB2, (wchar_t)0x1EB4, (wchar_t)0x1EB6, (wchar_t)0x1EB8, (wchar_t)0x1EBA,
            (wchar_t)0x1EBC, (wchar_t)0x1EBE, (wchar_t)0x1EC0, (wchar_t)0x1EC2, (wchar_t)0x1EC4,
            (wchar_t)0x1EC6, (wchar_t)0x1EC8, (wchar_t)0x1ECA, (wchar_t)0x1ECC, (wchar_t)0x1ECE,
            (wchar_t)0x1ED0, (wchar_t)0x1ED2, (wchar_t)0x1ED4, (wchar_t)0x1ED6, (wchar_t)0x1ED8,
            (wchar_t)0x1EDA, (wchar_t)0x1EDC, (wchar_t)0x1EDE, (wchar_t)0x1EE0, (wchar_t)0x1EE2,
            (wchar_t)0x1EE4, (wchar_t)0x1EE6, (wchar_t)0x1EE8, (wchar_t)0x1EEA, (wchar_t)0x1EEC,
            (wchar_t)0x1EEE, (wchar_t)0x1EF0, (wchar_t)0x1EF2, (wchar_t)0x1EF4, (wchar_t)0x1EF6,
            (wchar_t)0x1EF8, (wchar_t)0x1F08, (wchar_t)0x1F09, (wchar_t)0x1F0A, (wchar_t)0x1F0B,
            (wchar_t)0x1F0C, (wchar_t)0x1F0D, (wchar_t)0x1F0E, (wchar_t)0x1F0F, (wchar_t)0x1F18,
            (wchar_t)0x1F19, (wchar_t)0x1F1A, (wchar_t)0x1F1B, (wchar_t)0x1F1C, (wchar_t)0x1F1D,
            (wchar_t)0x1F28, (wchar_t)0x1F29, (wchar_t)0x1F2A, (wchar_t)0x1F2B, (wchar_t)0x1F2C,
            (wchar_t)0x1F2D, (wchar_t)0x1F2E, (wchar_t)0x1F2F, (wchar_t)0x1F38, (wchar_t)0x1F39,
            (wchar_t)0x1F3A, (wchar_t)0x1F3B, (wchar_t)0x1F3C, (wchar_t)0x1F3D, (wchar_t)0x1F3E,
            (wchar_t)0x1F3F, (wchar_t)0x1F48, (wchar_t)0x1F49, (wchar_t)0x1F4A, (wchar_t)0x1F4B,
            (wchar_t)0x1F4C, (wchar_t)0x1F4D, (wchar_t)0x1F59, (wchar_t)0x1F5B, (wchar_t)0x1F5D,
            (wchar_t)0x1F5F, (wchar_t)0x1F68, (wchar_t)0x1F69, (wchar_t)0x1F6A, (wchar_t)0x1F6B,
            (wchar_t)0x1F6C, (wchar_t)0x1F6D, (wchar_t)0x1F6E, (wchar_t)0x1F6F, (wchar_t)0x1F88,
            (wchar_t)0x1F89, (wchar_t)0x1F8A, (wchar_t)0x1F8B, (wchar_t)0x1F8C, (wchar_t)0x1F8D,
            (wchar_t)0x1F8E, (wchar_t)0x1F8F, (wchar_t)0x1F98, (wchar_t)0x1F99, (wchar_t)0x1F9A,
            (wchar_t)0x1F9B, (wchar_t)0x1F9C, (wchar_t)0x1F9D, (wchar_t)0x1F9E, (wchar_t)0x1F9F,
            (wchar_t)0x1FA8, (wchar_t)0x1FA9, (wchar_t)0x1FAA, (wchar_t)0x1FAB, (wchar_t)0x1FAC,
            (wchar_t)0x1FAD, (wchar_t)0x1FAE, (wchar_t)0x1FAF, (wchar_t)0x1FB8, (wchar_t)0x1FB9,
            (wchar_t)0x1FD8, (wchar_t)0x1FD9, (wchar_t)0x1FE8, (wchar_t)0x1FE9, (wchar_t)0x24B6,
            (wchar_t)0x24B7, (wchar_t)0x24B8, (wchar_t)0x24B9, (wchar_t)0x24BA, (wchar_t)0x24BB,
            (wchar_t)0x24BC, (wchar_t)0x24BD, (wchar_t)0x24BE, (wchar_t)0x24BF, (wchar_t)0x24C0,
            (wchar_t)0x24C1, (wchar_t)0x24C2, (wchar_t)0x24C3, (wchar_t)0x24C4, (wchar_t)0x24C5,
            (wchar_t)0x24C6, (wchar_t)0x24C7, (wchar_t)0x24C8, (wchar_t)0x24C9, (wchar_t)0x24CA,
            (wchar_t)0x24CB, (wchar_t)0x24CC, (wchar_t)0x24CD, (wchar_t)0x24CE, (wchar_t)0x24CF,
            (wchar_t)0xFF21, (wchar_t)0xFF22, (wchar_t)0xFF23, (wchar_t)0xFF24, (wchar_t)0xFF25,
            (wchar_t)0xFF26, (wchar_t)0xFF27, (wchar_t)0xFF28, (wchar_t)0xFF29, (wchar_t)0xFF2A,
            (wchar_t)0xFF2B, (wchar_t)0xFF2C, (wchar_t)0xFF2D, (wchar_t)0xFF2E, (wchar_t)0xFF2F,
            (wchar_t)0xFF30, (wchar_t)0xFF31, (wchar_t)0xFF32, (wchar_t)0xFF33, (wchar_t)0xFF34,
            (wchar_t)0xFF35, (wchar_t)0xFF36, (wchar_t)0xFF37, (wchar_t)0xFF38, (wchar_t)0xFF39,
            (wchar_t)0xFF3A, (wchar_t)0x0000};

        unsigned int chars2Unicode(const std::string& stringArg, size_t& cursor)
        {
            unsigned const char checkCharType = stringArg[cursor];
            unsigned int result = '?';

            // 0xxxxxxx, one byte character.
            if (checkCharType <= 0x7F) {
                // 0xxxxxxx
                result = (stringArg[cursor++]);
            }
            // 11110xxx, four byte character.
            else if (checkCharType >= 0xF0 && cursor < stringArg.length() - 2) {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                result = (stringArg[cursor++] & 0x07) << 18;
                result |= (stringArg[cursor++] & 0x3F) << 12;
                result |= (stringArg[cursor++] & 0x3F) << 6;
                result |= stringArg[cursor++] & 0x3F;
            }
            // 1110xxxx, three byte character.
            else if (checkCharType >= 0xE0 && cursor < stringArg.length() - 1) {
                // 1110xxxx 10xxxxxx 10xxxxxx
                result = (stringArg[cursor++] & 0x0F) << 12;
                result |= (stringArg[cursor++] & 0x3F) << 6;
                result |= stringArg[cursor++] & 0x3F;
            }
            // 110xxxxx, two byte character.
            else if (checkCharType >= 0xC0 && cursor < stringArg.length()) {
                // 110xxxxx 10xxxxxx
                result = (stringArg[cursor++] & 0x1F) << 6;
                result |= stringArg[cursor++] & 0x3F;
            }
            else {
                // Error, invalid character.
                ++cursor;
            }

            return result;
        }

        std::string unicode2Chars(const unsigned int unicodeArg)
        {
            std::string result;

            // Normal UTF-8 ASCII character.
            if (unicodeArg < 0x80) {
                result += (unicodeArg & 0xFF);
            }
            // Two-byte character.
            else if (unicodeArg < 0x800) {
                result += ((unicodeArg >> 6) & 0xFF) | 0xC0;
                result += (unicodeArg & 0x3F) | 0x80;
            }
            // Three-byte character.
            else if (unicodeArg < 0xFFFF) {
                result += ((unicodeArg >> 12) & 0xFF) | 0xE0;
                result += ((unicodeArg >> 6) & 0x3F) | 0x80;
                result += (unicodeArg & 0x3F) | 0x80;
            }
            // Four-byte character.
            else if (unicodeArg <= 0x1fffff) {
                result += ((unicodeArg >> 18) & 0xFF) | 0xF0;
                result += ((unicodeArg >> 12) & 0x3F) | 0x80;
                result += ((unicodeArg >> 6) & 0x3F) | 0x80;
                result += (unicodeArg & 0x3F) | 0x80;
            }
            else {
                // Error, invalid character.
                result += '?';
            }

            return result;
        }

        std::string getFirstCharacter(const std::string& stringArg, bool toUpper)
        {
            std::string firstChar;
            unsigned const char checkCharType = stringArg.front();

            // Normal UTF-8 ASCII character.
            if (checkCharType <= 0x7F)
                (toUpper) ? firstChar = toupper(stringArg.front()) : firstChar = stringArg.front();
            // Four-byte Unicode character.
            else if (checkCharType >= 0xF0)
                firstChar = stringArg.substr(0, 4);
            // Three-byte Unicode character.
            else if (checkCharType >= 0xE0)
                firstChar = stringArg.substr(0, 3);
            // Two-byte Unicode character.
            else if (checkCharType >= 0xC0)
                firstChar = stringArg.substr(0, 2);

            return firstChar;
        }

        size_t nextCursor(const std::string& stringArg, const size_t cursor)
        {
            size_t result = cursor;

            while (result < stringArg.length()) {
                ++result;

                // Break if current character is not 10xxxxxx
                if ((stringArg[result] & 0xC0) != 0x80)
                    break;
            }

            return result;
        }

        size_t prevCursor(const std::string& stringArg, const size_t cursor)
        {
            size_t result = cursor;

            while (result > 0) {
                --result;

                // Break if current character is not 10xxxxxx
                if ((stringArg[result] & 0xC0) != 0x80)
                    break;
            }

            return result;
        }

        size_t moveCursor(const std::string& stringArg, const size_t cursor, const int amount)
        {
            size_t result = cursor;

            if (amount > 0) {
                for (int i = 0; i < amount; ++i)
                    result = nextCursor(stringArg, result);
            }
            else if (amount < 0) {
                for (int i = amount; i < 0; ++i)
                    result = prevCursor(stringArg, result);
            }

            return result;
        }

        std::string toLower(const std::string& stringArg)
        {
            std::string stringLower;
            unsigned char checkCharType;

            for (size_t i = 0; i < stringArg.length();) {
                checkCharType = stringArg[i];
                // Normal UTF-8 ASCII character.
                if (checkCharType <= 0x7F) {
                    stringLower += static_cast<char>(tolower(stringArg[i]));
                    ++i;
                }
                // Four-byte Unicode character, no case conversion done.
                else if (checkCharType >= 0xF0) {
                    stringLower += stringArg.substr(i, 4);
                    i += 4;
                }
                // Three-byte Unicode character, no case conversion done.
                else if (checkCharType >= 0xE0) {
                    stringLower += stringArg.substr(i, 3);
                    i += 3;
                }
                // Two-byte Unicode character.
                else if (checkCharType >= 0xC0) {
                    // Extract the Unicode code point from the two bytes.
                    wchar_t firstChar = (stringArg[i] & 0x1F) << 6;
                    wchar_t secondChar = stringArg[i + 1] & 0x3F;
                    wchar_t unicodeChar = firstChar | secondChar;

                    // Try to find an entry for the character in the Unicode uppercase table.
                    wchar_t* charIndex = std::wcschr(unicodeUppercase, unicodeChar);

                    if (charIndex != nullptr) {
                        wchar_t lowerChar = *(unicodeLowercase + (charIndex - unicodeUppercase));
                        // Convert back to string format.
                        typedef std::codecvt_utf8<wchar_t> convert_type;
                        std::wstring_convert<convert_type, wchar_t> byteConverter;

                        stringLower += byteConverter.to_bytes(lowerChar);
                    }
                    else {
                        // We normally end up here if the character is a space, or if it's
                        // already in lowercase.
                        stringLower += stringArg[i];
                        stringLower += stringArg[i + 1];
                    }
                    i += 2;
                }
            }

            return stringLower;
        }

        std::string toUpper(const std::string& stringArg)
        {
            std::string stringUpper;
            unsigned char checkCharType;

            for (size_t i = 0; i < stringArg.length();) {
                checkCharType = stringArg[i];
                // Normal UTF-8 ASCII character.
                if (checkCharType <= 0x7F) {
                    stringUpper += static_cast<char>(toupper(stringArg[i]));
                    ++i;
                }
                // Four-byte Unicode character, no case conversion done.
                else if (checkCharType >= 0xF0) {
                    stringUpper += stringArg.substr(i, 4);
                    i += 4;
                }
                // Three-byte Unicode character, no case conversion done.
                else if (checkCharType >= 0xE0) {
                    stringUpper += stringArg.substr(i, 3);
                    i += 3;
                }
                // Two-byte Unicode character.
                else if (checkCharType >= 0xC0) {
                    // Extract the Unicode code point from the two bytes.
                    wchar_t firstChar = (stringArg[i] & 0x1F) << 6;
                    wchar_t secondChar = stringArg[i + 1] & 0x3F;
                    wchar_t unicodeChar = firstChar | secondChar;

                    // Try to find an entry for the character in the Unicode lowercase table.
                    wchar_t* charIndex = std::wcschr(unicodeLowercase, unicodeChar);

                    if (charIndex != nullptr) {
                        wchar_t upperChar = *(unicodeUppercase + (charIndex - unicodeLowercase));
                        // Convert back to string format.
                        typedef std::codecvt_utf8<wchar_t> convert_type;
                        std::wstring_convert<convert_type, wchar_t> byteConverter;

                        stringUpper += byteConverter.to_bytes(upperChar);
                    }
                    else {
                        // We normally end up here if the character is a space, or if it's
                        // already in uppercase.
                        stringUpper += stringArg[i];
                        stringUpper += stringArg[i + 1];
                    }
                    i += 2;
                }
            }

            return stringUpper;
        }

        std::string toCamelCase(const std::string& stringArg)
        {
            std::string line = stringArg;
            bool active = true;

            for (int i = 0; line[i] != '\0'; ++i) {
                if (std::isalpha(line[i])) {
                    if (active) {
                        line[i] = Utils::String::toUpper(std::string(1, line[i]))[0];
                        active = false;
                    }
                    else
                        line[i] = Utils::String::toLower(std::string(1, line[i]))[0];
                }
                else if (line[i] == ' ')
                    active = true;
            }

            return line;
        }

        std::string trim(const std::string& stringArg)
        {
            std::string trimString = stringArg;

            // Trim leading and trailing whitespaces.
            trimString.erase(trimString.begin(),
                             std::find_if(trimString.begin(), trimString.end(), [](char c) {
                                 return !std::isspace(static_cast<unsigned char>(c));
                             }));
            trimString.erase(
                std::find_if(trimString.rbegin(), trimString.rend(),
                             [](char c) { return !std::isspace(static_cast<unsigned char>(c)); })
                    .base(),
                trimString.end());

            return trimString;
        }

        std::string replace(const std::string& stringArg,
                            const std::string& from,
                            const std::string& to)
        {
            std::string result{stringArg};

            // The outer loop makes sure that we're eliminating all repeating occurances
            // of the 'from' value.
            while (result.find(from) != std::string::npos) {
                // Prevent endless loops.
                if (from == to)
                    break;

                std::string replaced;
                size_t lastPos{0};
                size_t findPos{0};

                while ((findPos = result.find(from, lastPos)) != std::string::npos) {
                    replaced.append(result, lastPos, findPos - lastPos).append(to);
                    lastPos = findPos + from.length();
                }

                replaced.append(result.substr(lastPos));
                result = replaced;

                // Prevent endless loops.
                if (to.find(from) != std::string::npos)
                    break;
            }
            return result;
        }

        std::wstring stringToWideString(const std::string& stringArg)
        {
            typedef std::codecvt_utf8<wchar_t> convert_type;
            std::wstring_convert<convert_type, wchar_t> stringConverter;

            return stringConverter.from_bytes(stringArg);
        }

        std::string wideStringToString(const std::wstring& stringArg)
        {
            typedef std::codecvt_utf8<wchar_t> convert_type;
            std::wstring_convert<convert_type, wchar_t> stringConverter;

            return stringConverter.to_bytes(stringArg);
        }

        bool startsWith(const std::string& stringArg, const std::string& start)
        {
            return (stringArg.find(start) == 0);
        }

        bool endsWith(const std::string& stringArg, const std::string& end)
        {
            return (stringArg.find(end) == (stringArg.size() - end.size()));
        }

        std::string removeParenthesis(const std::string& stringArg)
        {
            static std::vector<char> remove = {'(', ')', '[', ']'};
            std::string stringRemove = stringArg;
            size_t start;
            size_t end;
            bool done = false;

            while (!done) {
                done = true;

                for (size_t i = 0; i < remove.size(); i += 2) {
                    end = stringRemove.find_first_of(remove[i + 1]);
                    start = stringRemove.find_last_of(remove[i + 0], end);

                    if ((start != std::string::npos) && (end != std::string::npos)) {
                        stringRemove.erase(start, end - start + 1);
                        done = false;
                    }
                }
            }

            return trim(stringRemove);
        }

        std::vector<std::string> delimitedStringToVector(const std::string& stringArg,
                                                         const std::string& delimiter,
                                                         bool sort,
                                                         bool caseInsensitive)
        {
            std::vector<std::string> vectorResult;
            size_t start = 0;
            size_t delimPos = stringArg.find(delimiter);

            while (delimPos != std::string::npos) {
                vectorResult.push_back(stringArg.substr(start, delimPos - start));
                start = delimPos + 1;
                delimPos = stringArg.find(delimiter, start);
            }

            vectorResult.push_back(stringArg.substr(start));
            if (sort) {
                if (caseInsensitive)
                    std::sort(std::begin(vectorResult), std::end(vectorResult),
                              [](std::string a, std::string b) {
                                  return std::toupper(a.front()) < std::toupper(b.front());
                              });
                else
                    std::sort(vectorResult.begin(), vectorResult.end());
            }

            return vectorResult;
        }

        std::string vectorToDelimitedString(std::vector<std::string> vectorArg,
                                            const std::string& delimiter,
                                            bool caseInsensitive)
        {
            std::string resultString;

            if (caseInsensitive) {
                std::sort(std::begin(vectorArg), std::end(vectorArg),
                          [](std::string a, std::string b) {
                              return std::toupper(a.front()) < std::toupper(b.front());
                          });
            }
            else {
                std::sort(vectorArg.begin(), vectorArg.end());
            }

            for (std::vector<std::string>::const_iterator it = vectorArg.cbegin();
                 it != vectorArg.cend(); ++it)
                resultString += (resultString.length() ? delimiter : "") + (*it);

            return resultString;
        }

        std::string scramble(const std::string& input, const std::string& key)
        {
            std::string buffer = input;

            for (size_t i = 0; i < input.size(); ++i)
                buffer[i] = input[i] ^ key[i];

            return buffer;
        }

    } // namespace String

} // namespace Utils
