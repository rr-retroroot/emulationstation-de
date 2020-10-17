//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiVideoScreensaverOptions.cpp
//
//  User interface for the video screensaver options.
//  Submenu to GuiGeneralScreensaverOptions.
//

#include "guis/GuiVideoScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Settings.h"

GuiVideoScreensaverOptions::GuiVideoScreensaverOptions(Window* window, const char* title)
        : GuiScreensaverOptions(window, title)
{
    // Timer for swapping videos.
    auto swap = std::make_shared<SliderComponent>(mWindow, 1.f, 120.f, 1.f, "s");
    swap->setValue((float)(Settings::getInstance()->
            getInt("ScreenSaverSwapVideoTimeout") / (1000)));
    addWithLabel("SWAP VIDEO AFTER (SECS)", swap);
    addSaveFunc([swap] {
        int playNextTimeout = (int)Math::round(swap->getValue()) * (1000);
        Settings::getInstance()->setInt("ScreenSaverSwapVideoTimeout", playNextTimeout);
        PowerSaver::updateTimeouts();
    });

    auto stretch_screensaver = std::make_shared<SwitchComponent>(mWindow);
    stretch_screensaver->setState(Settings::getInstance()->getBool("ScreenSaverStretchVideos"));
    addWithLabel("STRETCH VIDEOS TO SCREEN RESOLUTION", stretch_screensaver);
    addSaveFunc([stretch_screensaver] { Settings::getInstance()->
            setBool("ScreenSaverStretchVideos", stretch_screensaver->getState()); });

    #if defined(_RPI_)
    auto ss_omx = std::make_shared<SwitchComponent>(mWindow);
    ss_omx->setState(Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
    addWithLabel("USE OMX PLAYER FOR SCREENSAVER", ss_omx);
    addSaveFunc([ss_omx, this] { Settings::getInstance()->
            setBool("ScreenSaverOmxPlayer", ss_omx->getState()); });

    // TEMPORARY - Disabled for now, need to find a proper way to make an overlay on top of
    // the videos. The solution with rendering subtitles is not a good solution.
    // And as well the VLC video player subtitles seem to be somehow broken.
    // Render video game name as subtitles.
//    auto ss_info = std::make_shared<OptionListComponent<std::string>>
//            (mWindow,getHelpStyle(), "SHOW GAME INFO", false);
//    std::vector<std::string> info_type;
//    info_type.push_back("always");
//    info_type.push_back("start & end");
//    info_type.push_back("never");
//    for (auto it = info_type.cbegin(); it != info_type.cend(); it++)
//        ss_info->add(*it, *it, Settings::getInstance()->getString("ScreenSaverGameInfo") == *it);
//    addWithLabel("SHOW GAME INFO ON SCREENSAVER", ss_info);
//    addSaveFunc([ss_info, this] { Settings::getInstance()->
//            setString("ScreenSaverGameInfo", ss_info->getSelected()); });

//    ComponentListRow row;

    // Set subtitle position.
//    auto ss_omx_subs_align = std::make_shared<OptionListComponent<std::string>>
//            (mWindow, getHelpStyle(), "GAME INFO ALIGNMENT", false);
//    std::vector<std::string> align_mode;
//    align_mode.push_back("left");
//    align_mode.push_back("center");
//    for (auto it = align_mode.cbegin(); it != align_mode.cend(); it++)
//        ss_omx_subs_align->add(*it, *it, Settings::getInstance()->
//                getString("SubtitleAlignment") == *it);
//    addWithLabel("GAME INFO ALIGNMENT", ss_omx_subs_align);
//    addSaveFunc([ss_omx_subs_align, this] { Settings::getInstance()->
//            setString("SubtitleAlignment", ss_omx_subs_align->getSelected()); });

    // Set font size.
//    auto ss_omx_font_size = std::make_shared<SliderComponent>(mWindow, 1.f, 64.f, 1.f, "h");
//    ss_omx_font_size->setValue((float)(Settings::getInstance()->getInt("SubtitleSize")));
//    addWithLabel("GAME INFO FONT SIZE", ss_omx_font_size);
//    addSaveFunc([ss_omx_font_size] {
//        int subSize = (int)Math::round(ss_omx_font_size->getValue());
//        Settings::getInstance()->setInt("SubtitleSize", subSize);
//    });
    #endif

    auto ss_video_audio = std::make_shared<SwitchComponent>(mWindow);
    ss_video_audio->setState(Settings::getInstance()->getBool("ScreenSaverVideoAudio"));
    addWithLabel("PLAY AUDIO FOR SCREENSAVER VIDEO FILES", ss_video_audio);
    addSaveFunc([ss_video_audio] { Settings::getInstance()->
            setBool("ScreenSaverVideoAudio", ss_video_audio->getState()); });

    #if defined(USE_OPENGL_21)
    // Render scanlines using a shader.
    auto render_scanlines = std::make_shared<SwitchComponent>(mWindow);
    render_scanlines->setState(Settings::getInstance()->getBool("ScreenSaverVideoScanlines"));
    addWithLabel("RENDER SCANLINES", render_scanlines);
    addSaveFunc([render_scanlines] { Settings::getInstance()->
            setBool("ScreenSaverVideoScanlines", render_scanlines->getState()); });

    // Render blur using a shader.
    auto render_blur = std::make_shared<SwitchComponent>(mWindow);
    render_blur->setState(Settings::getInstance()->getBool("ScreenSaverVideoBlur"));
    addWithLabel("RENDER BLUR", render_blur);
    addSaveFunc([render_blur] { Settings::getInstance()->
            setBool("ScreenSaverVideoBlur", render_blur->getState()); });
    #endif
}

GuiVideoScreensaverOptions::~GuiVideoScreensaverOptions()
{
}

void GuiVideoScreensaverOptions::save()
{
    #if defined(_RPI_)
    bool startingStatusNotRisky = (Settings::getInstance()->
            getString("ScreenSaverGameInfo") == "never" ||
            !Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
    #endif
    GuiScreensaverOptions::save();

    #if defined(_RPI_)
    bool endStatusRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") !=
            "never" && Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
    if (startingStatusNotRisky && endStatusRisky) {
        // If before it wasn't risky but now there's a risk of problems, show warning.
        mWindow->pushGui(new GuiMsgBox(mWindow, getHelpStyle(),
        "Using OMX Player and displaying Game Info may result in the video flickering in "
        "some TV modes. If that happens, consider:\n\n• Disabling the \"Show Game Info\" "
        "option;\n• Disabling \"Overscan\" on the Pi configuration menu might help:\nRetroPie > "
        "Raspi-Config > Advanced Options > Overscan > \"No\".\n• Disabling the use of OMX Player "
        "for the screensaver.",
            "GOT IT!", [] { return; }));
    }
    #endif
}
