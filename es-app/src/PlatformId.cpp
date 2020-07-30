//
//  PlatformId.cpp
//
//  Index of all supported systems/platforms.
//

#include "PlatformId.h"

#include <string.h>

namespace PlatformIds
{
    const char* PlatformNames[PLATFORM_COUNT + 1] = {
        "unknown", // Nothing set.

        "3do",
        "amiga",
        "amstradcpc",
        "apple2",
        "arcade",
        "atari800",
        "atari2600",
        "atari5200",
        "atari7800",
        "atarilynx",
        "atarist",
        "atarijaguar",
        "atarijaguarcd",
        "atarixe",
        "colecovision",
        "c64",  // Commodore 64
        "intellivision",
        "macintosh",
        "xbox",
        "xbox360",
        "msx",
        "neogeo",
        "ngp",  // Neo Geo Pocket
        "ngpc",  // Neo Geo Pocket Color
        "n3ds",  // Nintendo 3DS
        "n64",  // Nintendo 64
        "nds",  // Nintendo DS
        "fds",  // Famicom Disk System
        "nes",  // Nintendo Entertainment System
        "channelf",  // Fairchild Channel F
        "gb",  // Game Boy
        "gba",  // Game Boy Advance
        "gbc",  // Game Boy Color
        "gamecube",  // GameCube
        "wii",
        "wiiu",
        "virtualboy",
        "gameandwatch",
        "openbor",
        "dos",
        "pc",
        "sega32x",
        "segacd",
        "dreamcast",
        "gamegear",
        "genesis",  // Sega Genesis
        "mastersystem",  // Sega Master System
        "megadrive",  // Sega Mega Drive
        "saturn",  // Sega Saturn
        "sg-1000",
        "psx",
        "ps2",
        "ps3",
        "ps4",
        "psvita",
        "psp", // PlayStation Portable
        "snes", // Super Nintendo Entertainment System
        "scummvm",
        "x68000",
        "solarus",
        "pcengine",  // (Aka turbografx-16), HuCards only
        "pcenginecd",  // (Aka turbografx-16), CD-ROMs only
        "wonderswan",
        "wonderswancolor",
        "zxspectrum",
        "zx81",
        "odyssey2",
        "vectrex",
        "trs-80",
        "coco",
        "ags",
        "astrocade",
        "bbcmicro",
        "cavestory",
        "daphne",
        "doom",
        "dragon32",
        "famicom",
        "love",
        "lutro",
        "mess",
        "moonlight",
        "oric",
        "pcfx",
        "residualvm",
        "samcoupe",
        "supergrafx",
        "stratagus",
        "videopac",
        "zmachine",
        "ti99",

        "ignore",  // Do not allow scraping for this system.
        "invalid"
    };

    PlatformId getPlatformId(const char* str)
    {
        if (str == nullptr)
            return PLATFORM_UNKNOWN;

        for (unsigned int i = 1; i < PLATFORM_COUNT; i++) {
            if (strcmp(PlatformNames[i], str) == 0)
                return (PlatformId)i;
        }

        return PLATFORM_UNKNOWN;
    }

    const char* getPlatformName(PlatformId id)
    {
        return PlatformNames[id];
    }
}
