//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  PlatformId.h
//
//  Index of all supported systems/platforms.
//

#ifndef ES_APP_PLATFORM_ID_H
#define ES_APP_PLATFORM_ID_H

#include <string>

namespace PlatformIds
{
    enum PlatformId : unsigned int {
        PLATFORM_UNKNOWN = 0,

        THREEDO, // Names can't start with a number.
        COLECO_ADAM,
        GAMEENGINE_ADVENTUREGAMESTUDIO,
        COMMODORE_AMIGA,
        COMMODORE_AMIGA_CD32,
        AMSTRAD_CPC,
        GOOGLE_ANDROID,
        APPLE_II,
        APPLE_IIGS,
        ARCADE,
        ARCADIA_2001,
        ACORN_ARCHIMEDES,
        ARDUBOY,
        BALLY_ASTROCADE,
        ATARI_2600,
        ATARI_5200,
        ATARI_7800,
        ATARI_800,
        ATARI_JAGUAR,
        ATARI_JAGUAR_CD,
        ATARI_LYNX,
        ATARI_ST,
        ATARI_XE,
        ATOMISWAVE,
        BBC_MICRO,
        COMMODORE_64,
        CAVESTORY,
        PHILIPS_CDI,
        COMMODORE_CDTV,
        FAIRCHILD_CHANNELF,
        TANDY_COLOR_COMPUTER,
        COLECOVISION,
        VTECH_CREATIVISION,
        DAPHNE,
        DOS,
        DRAGON32,
        SEGA_DREAMCAST,
        GAMEENGINE_EASYRPG,
        ACORN_ELECTRON,
        NINTENDO_FAMICOM,
        NINTENDO_FAMICOM_DISK_SYSTEM,
        ADOBE_FLASH,
        FUJITSU_FM_7,
        FUJITSU_FM_TOWNS,
        FUTURE_PINBALL,
        BIT_CORPORATION_GAMATE,
        NINTENDO_GAME_AND_WATCH,
        TIGER_GAME_COM,
        SEGA_GAME_GEAR,
        NINTENDO_GAME_BOY,
        NINTENDO_GAME_BOY_ADVANCE,
        NINTENDO_GAME_BOY_COLOR,
        NINTENDO_GAMECUBE,
        SEGA_GENESIS,
        HARTUNG_GAME_MASTER,
        AMSTRAD_GX4000,
        INTELLIVISION,
        LCD_GAMES,
        GAMEENGINE_LOVE,
        GAMEENGINE_LOWRES_NX,
        GAMEENGINE_LUTRO,
        APPLE_MACINTOSH,
        SEGA_MASTER_SYSTEM,
        SEGA_MEGA_DRIVE,
        CREATRONIC_MEGA_DUCK,
        MESS,
        MOONLIGHT,
        THOMSON_MOTO,
        MSX,
        MSX2,
        MSX_TURBO_R,
        GAMEENGINE_MUGEN,
        NINTENDO_3DS,
        NINTENDO_64,
        SEGA_NAOMI,
        NINTENDO_DS,
        SNK_NEO_GEO,
        SNK_NEO_GEO_CD,
        NINTENDO_ENTERTAINMENT_SYSTEM,
        NOKIA_NGAGE,
        SNK_NEO_GEO_POCKET,
        SNK_NEO_GEO_POCKET_COLOR,
        VIDEOPAC_ODYSSEY2,
        GAMEENGINE_OPENBOR,
        TANGERINE_ORIC,
        PALM_OS,
        PC,
        NEC_PC_8800,
        NEC_PC_9800,
        NEC_PC_ENGINE,
        NEC_PC_ENGINE_CD,
        NEC_PCFX,
        MICROSOFT_WINDOWS,
        GAMEENGINE_PICO8,
        COMMODORE_PLUS4,
        NINTENDO_POKEMON_MINI,
        SONY_PLAYSTATION_2,
        SONY_PLAYSTATION_3,
        SONY_PLAYSTATION_4,
        SONY_PLAYSTATION_PORTABLE,
        SONY_PLAYSTATION_VITA,
        SONY_PLAYSTATION,
        CASIO_PV1000,
        GAMEENGINE_RESIDUALVM,
        SAMCOUPE,
        NINTENDO_SATELLAVIEW,
        SEGA_SATURN,
        GAMEENGINE_SCUMMVM,
        EPOCH_SCV,
        SEGA_32X,
        SEGA_CD,
        SEGA_SG1000,
        NINTENDO_SUPER_GAME_BOY,
        SUPER_NINTENDO,
        SUPER_NINTENDO_MSU1,
        GAMEENGINE_SOLARUS,
        SPECTRAVIDEO,
        VALVE_STEAM,
        GAMEENGINE_STRATAGUS,
        BANDAI_SUFAMI_TURBO,
        NEC_SUPERGRAFX,
        WATARA_SUPERVISION,
        FUNTECH_SUPER_ACAN,
        NINTENDO_SWITCH,
        TEXAS_INSTRUMENTS_TI99,
        GAMEENGINE_TIC80,
        TANDY_TRS80,
        UZEBOX,
        VECTREX,
        COMMODORE_VIC20,
        NINTENDO_VIRTUAL_BOY,
        VISUAL_PINBALL,
        VTECH_VSMILE,
        GAMEENGINE_WASM4,
        NINTENDO_WII,
        NINTENDO_WII_U,
        MICROSOFT_WINDOWS_3X,
        BANDAI_WONDERSWAN,
        BANDAI_WONDERSWAN_COLOR,
        SHARP_X1,
        SHARP_X68000,
        MICROSOFT_XBOX,
        MICROSOFT_XBOX_360,
        GAMEENGINE_Z_MACHINE,
        SINCLAIR_ZX81_SINCLAR,
        SINCLAIR_ZX_SPECTRUM_NEXT,
        SINCLAIR_ZX_SPECTRUM,

        PLATFORM_IGNORE, // Do not allow scraping for this system.
        PLATFORM_COUNT
    };

    PlatformId getPlatformId(const std::string& str);
    const std::string getPlatformName(PlatformId id);

} // namespace PlatformIds

#endif // ES_APP_PLATFORM_ID_H
