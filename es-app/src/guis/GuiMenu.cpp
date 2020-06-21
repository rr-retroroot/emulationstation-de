//
//	GuiMenu.cpp
//
//	Main menu.
//	Some submenus are covered in separate source files.
//

#include "guis/GuiMenu.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiGeneralScreensaverOptions.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiScraperMenu.h"
#include "guis/GuiSettings.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "views/gamelist/IGameListView.h"
#include "CollectionSystemManager.h"
#include "EmulationStation.h"
#include "Platform.h"
#include "Scripting.h"
#include "SystemData.h"
#include "VolumeControl.h"
#include <SDL_events.h>
#include <algorithm>

GuiMenu::GuiMenu(
		Window* window)
		: GuiComponent(window),
		mMenu(window, "MAIN MENU"),
		mVersion(window)
{
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();

	if (isFullUI)
		addEntry("SCRAPER", 0x777777FF, true, [this] { openScraperSettings(); });

	if (isFullUI)
		addEntry("UI SETTINGS", 0x777777FF, true, [this] { openUISettings(); });

	addEntry("SOUND SETTINGS", 0x777777FF, true, [this] { openSoundSettings(); });

	if (isFullUI)
		addEntry("GAME COLLECTION SETTINGS", 0x777777FF, true, [this] {
				openCollectionSystemSettings(); });

	if (isFullUI)
		addEntry("OTHER SETTINGS", 0x777777FF, true, [this] { openOtherSettings(); });

	if (isFullUI)
		addEntry("CONFIGURE INPUT", 0x777777FF, true, [this] { openConfigInput(); });

	addEntry("QUIT", 0x777777FF, true, [this] {openQuitMenu(); });

	addChild(&mMenu);
	addVersionInfo();
	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2,
			Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::openScraperSettings()
{
	// Open the scrape menu.
	mWindow->pushGui(new GuiScraperMenu(mWindow));
}

void GuiMenu::openSoundSettings()
{
	auto s = new GuiSettings(mWindow, "SOUND SETTINGS");

	// System volume.
	auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	volume->setValue((float)VolumeControl::getInstance()->getVolume());
	s->addWithLabel("SYSTEM VOLUME", volume);
	s->addSaveFunc([volume] { VolumeControl::getInstance()->
			setVolume((int)Math::round(volume->getValue())); });

	if (UIModeController::getInstance()->isUIModeFull()) {
		#if defined(__linux__)
		// audio card
		auto audio_card = std::make_shared<OptionListComponent<std::string>>
				(mWindow, getHelpStyle(), "AUDIO CARD", false);
		std::vector<std::string> audio_cards;
		#ifdef _RPI_
		// RPi Specific  Audio Cards
		audio_cards.push_back("local");
		audio_cards.push_back("hdmi");
		audio_cards.push_back("both");
		#endif
		audio_cards.push_back("default");
		audio_cards.push_back("sysdefault");
		audio_cards.push_back("dmix");
		audio_cards.push_back("hw");
		audio_cards.push_back("plughw");
		audio_cards.push_back("null");
		if (Settings::getInstance()->getString("AudioCard") != "") {
			if (std::find(audio_cards.begin(), audio_cards.end(),
					Settings::getInstance()->getString("AudioCard")) == audio_cards.end()) {
				audio_cards.push_back(Settings::getInstance()->getString("AudioCard"));
			}
		}
		for (auto ac = audio_cards.cbegin(); ac != audio_cards.cend(); ac++)
			audio_card->add(*ac, *ac, Settings::getInstance()->getString("AudioCard") == *ac);
		s->addWithLabel("AUDIO CARD", audio_card);
		s->addSaveFunc([audio_card] {
			Settings::getInstance()->setString("AudioCard", audio_card->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});

		// Volume control device.
		auto vol_dev = std::make_shared<OptionListComponent<std::string>>
				(mWindow, getHelpStyle(), "AUDIO DEVICE", false);
		std::vector<std::string> transitions;
		transitions.push_back("PCM");
		transitions.push_back("Speaker");
		transitions.push_back("Master");
		transitions.push_back("Digital");
		transitions.push_back("Analogue");
		if (Settings::getInstance()->getString("AudioDevice") != "") {
			if (std::find(transitions.begin(), transitions.end(),
					Settings::getInstance()->getString("AudioDevice")) == transitions.end()) {
				transitions.push_back(Settings::getInstance()->getString("AudioDevice"));
			}
		}
		for (auto it = transitions.cbegin(); it != transitions.cend(); it++)
			vol_dev->add(*it, *it, Settings::getInstance()->getString("AudioDevice") == *it);
		s->addWithLabel("AUDIO DEVICE", vol_dev);
		s->addSaveFunc([vol_dev] {
			Settings::getInstance()->setString("AudioDevice", vol_dev->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});
		#endif

		#ifdef _RPI_
		// OMX player Audio Device
		auto omx_audio_dev = std::make_shared<OptionListComponent<std::string>>
				(mWindow, getHelpStyle(), "OMX PLAYER AUDIO DEVICE", false);
		std::vector<std::string> omx_cards;
		// RPi Specific  Audio Cards
		omx_cards.push_back("local");
		omx_cards.push_back("hdmi");
		omx_cards.push_back("both");
		omx_cards.push_back("alsa:hw:0,0");
		omx_cards.push_back("alsa:hw:1,0");
		if (Settings::getInstance()->getString("OMXAudioDev") != "") {
			if (std::find(omx_cards.begin(), omx_cards.end(),
					Settings::getInstance()->getString("OMXAudioDev")) == omx_cards.end()) {
				omx_cards.push_back(Settings::getInstance()->getString("OMXAudioDev"));
			}
		}
		for (auto it = omx_cards.cbegin(); it != omx_cards.cend(); it++)
			omx_audio_dev->add(*it, *it, Settings::getInstance()->getString("OMXAudioDev") == *it);
		s->addWithLabel("OMX PLAYER AUDIO DEVICE", omx_audio_dev);
		s->addSaveFunc([omx_audio_dev] {
			if (Settings::getInstance()->getString("OMXAudioDev") != omx_audio_dev->getSelected())
				Settings::getInstance()->setString("OMXAudioDev", omx_audio_dev->getSelected());
		});
		#endif

		// Video audio.
		auto video_audio = std::make_shared<SwitchComponent>(mWindow);
		video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
		s->addWithLabel("ENABLE AUDIO FOR VIDEO FILES", video_audio);
		s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio",
				video_audio->getState()); });

		// Navigation sounds.
		auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
		sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
		s->addWithLabel("ENABLE NAVIGATION SOUNDS", sounds_enabled);
		s->addSaveFunc([sounds_enabled] {
			if (sounds_enabled->getState() &&
					!Settings::getInstance()->getBool("EnableSounds") &&
					PowerSaver::getMode() == PowerSaver::INSTANT) {
				Settings::getInstance()->setString("PowerSaverMode", "default");
				PowerSaver::init();
			}
			Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState());
		});
	}

	mWindow->pushGui(s);
}

void GuiMenu::openUISettings()
{
	auto s = new GuiSettings(mWindow, "UI SETTINGS");

	// Optionally start in selected system/gamelist.
	auto systemfocus_list = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "GAMELIST ON STARTUP", false);
	systemfocus_list->add("NONE", "", Settings::getInstance()->getString("StartupSystem") == "");
	for (auto it = SystemData::sSystemVector.cbegin();
			it != SystemData::sSystemVector.cend(); it++) {
		if ("retropie" != (*it)->getName())
			systemfocus_list->add((*it)->getName(), (*it)->getName(),
					Settings::getInstance()->getString("StartupSystem") == (*it)->getName());
	}
	s->addWithLabel("GAMELIST TO SHOW ON STARTUP", systemfocus_list);
	s->addSaveFunc([systemfocus_list] {
		Settings::getInstance()->setString("StartupSystem", systemfocus_list->getSelected());
	});

	// GameList view style.
	auto gamelist_style = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "GAMELIST VIEW STYLE", false);
	std::vector<std::string> styles;
	styles.push_back("automatic");
	styles.push_back("basic");
	styles.push_back("detailed");
	styles.push_back("video");
	styles.push_back("grid");

	for (auto it = styles.cbegin(); it != styles.cend(); it++)
		gamelist_style->add(*it, *it, Settings::getInstance()->
				getString("GamelistViewStyle") == *it);
	s->addWithLabel("GAMELIST VIEW STYLE", gamelist_style);
	s->addSaveFunc([gamelist_style] {
		bool needReload = false;
		if (Settings::getInstance()->getString("GamelistViewStyle") !=
				gamelist_style->getSelected())
			needReload = true;
		Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected());
		if (needReload)
			ViewController::get()->reloadAll();
	});

	// Transition style.
	auto transition_style = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "TRANSITION STYLE", false);
	std::vector<std::string> transitions;
	transitions.push_back("fade");
	transitions.push_back("slide");
	transitions.push_back("instant");
	for (auto it = transitions.cbegin(); it != transitions.cend(); it++)
		transition_style->add(*it, *it, Settings::getInstance()->
				getString("TransitionStyle") == *it);
	s->addWithLabel("TRANSITION STYLE", transition_style);
	s->addSaveFunc([transition_style] {
		if (Settings::getInstance()->getString("TransitionStyle") == "instant" &&
			transition_style->getSelected() != "instant" &&
			PowerSaver::getMode() == PowerSaver::INSTANT) {
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
	});

	// Theme selection.
	auto themeSets = ThemeData::getThemeSets();

	if (!themeSets.empty())
	{
		std::map<std::string, ThemeSet>::const_iterator selectedSet =
				themeSets.find(Settings::getInstance()->getString("ThemeSet"));
		if (selectedSet == themeSets.cend())
			selectedSet = themeSets.cbegin();

		auto theme_set = std::make_shared<OptionListComponent<std::string>>
				(mWindow, getHelpStyle(), "THEME SET", false);
		for (auto it = themeSets.cbegin(); it != themeSets.cend(); it++)
			theme_set->add(it->first, it->first, it == selectedSet);
		s->addWithLabel("THEME SET", theme_set);

		Window* window = mWindow;
		s->addSaveFunc([window, theme_set] {
			bool needReload = false;
			std::string oldTheme = Settings::getInstance()->getString("ThemeSet");
			if (oldTheme != theme_set->getSelected())
				needReload = true;

			Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

			if (needReload) {
				Scripting::fireEvent("theme-changed", theme_set->getSelected(), oldTheme);
				CollectionSystemManager::get()->updateSystemsList();
				ViewController::get()->goToStart();
				// TODO - replace this with some sort of signal-based implementation.
				ViewController::get()->reloadAll();
			}
		});
	}

	// UI mode.
	auto UImodeSelection = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "UI MODE", false);
	std::vector<std::string> UImodes = UIModeController::getInstance()->getUIModes();
	for (auto it = UImodes.cbegin(); it != UImodes.cend(); it++)
		UImodeSelection->add(*it, *it, Settings::getInstance()->getString("UIMode") == *it);
	s->addWithLabel("UI MODE", UImodeSelection);
	Window* window = mWindow;
	s->addSaveFunc([ UImodeSelection, window, this] {
		std::string selectedMode = UImodeSelection->getSelected();
		if (selectedMode != "Full") {
			std::string msg = "You are changing the UI to a restricted mode:\n" +
					selectedMode + "\n";
			msg += "This will hide most menu-options to prevent changes to the system.\n";
			msg += "To unlock and return to the full UI, enter this code: \n";
			msg += "\"" + UIModeController::getInstance()->getFormattedPassKeyStr() + "\"\n\n";
			msg += "Do you want to proceed?";
			window->pushGui(new GuiMsgBox(window, this->getHelpStyle(), msg,
				"YES", [selectedMode] {
					LOG(LogDebug) << "Setting UI mode to " << selectedMode;
					Settings::getInstance()->setString("UIMode", selectedMode);
					Settings::getInstance()->saveFile();
			}, "NO",nullptr));
		}
	});

	// Sort favorites on top of the gamelists.
	auto favoritesFirstSwitch = std::make_shared<SwitchComponent>(mWindow);
	favoritesFirstSwitch->setState(Settings::getInstance()->getBool("FavoritesFirst"));
	s->addWithLabel("SORT FAVORITES ON TOP OF GAMELISTS", favoritesFirstSwitch);
	s->addSaveFunc([favoritesFirstSwitch] {
	if (Settings::getInstance()->setBool("FavoritesFirst", favoritesFirstSwitch->getState()))
		for (auto it = SystemData::sSystemVector.cbegin(); it !=
				SystemData::sSystemVector.cend(); it++) {
			// The favorites and recent gamelists never sort favorites on top.
			if ((*it)->getName() == "favorites" || (*it)->getName() == "recent" ||
					(*it)->getName() == "collections")
				continue;
			// Don't re-sort custom collections as they have their own option
			// for whether to sort favorites on top or not (FavFirstCustom).
			if (CollectionSystemManager::get()->getIsCustomCollection((*it)))
				continue;

			FileData* rootFolder = (*it)->getRootFolder();
			rootFolder->sort(getSortTypeFromString(rootFolder->getSortTypeString()),
					Settings::getInstance()->getBool("FavoritesFirst"));
			ViewController::get()->reloadGameListView(*it);

			// Jump to the first row of the gamelist.
			IGameListView* gameList = ViewController::get()->getGameListView((*it)).get();
			gameList->setCursor(gameList->getFirstEntry());
		}
	});

	// Enable filters (ForceDisableFilters).
	auto enable_filter = std::make_shared<SwitchComponent>(mWindow);
	enable_filter->setState(!Settings::getInstance()->getBool("ForceDisableFilters"));
	s->addWithLabel("ENABLE GAMELIST FILTERS", enable_filter);
	s->addSaveFunc([enable_filter] {
		bool filter_is_enabled = !Settings::getInstance()->getBool("ForceDisableFilters");
		Settings::getInstance()->setBool("ForceDisableFilters", !enable_filter->getState());
		if (enable_filter->getState() != filter_is_enabled)
				ViewController::get()->ReloadAndGoToStart();
	});

	// Quick system select (left/right in game list view).
	auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
	quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
	s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
	s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect",
			quick_sys_select->getState()); });

	// Carousel transition option.
	auto move_carousel = std::make_shared<SwitchComponent>(mWindow);
	move_carousel->setState(Settings::getInstance()->getBool("MoveCarousel"));
	s->addWithLabel("CAROUSEL TRANSITIONS", move_carousel);
	s->addSaveFunc([move_carousel] {
		if (move_carousel->getState() &&
			!Settings::getInstance()->getBool("MoveCarousel") &&
			PowerSaver::getMode() == PowerSaver::INSTANT) {
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setBool("MoveCarousel", move_carousel->getState());
	});

	// Hide start menu in Kid Mode.
	auto disable_start = std::make_shared<SwitchComponent>(mWindow);
	disable_start->setState(Settings::getInstance()->getBool("DisableKidStartMenu"));
	s->addWithLabel("DISABLE START MENU IN KID MODE", disable_start);
	s->addSaveFunc([disable_start] { Settings::getInstance()->setBool("DisableKidStartMenu",
			disable_start->getState()); });

	// Show help.
	auto show_help = std::make_shared<SwitchComponent>(mWindow);
	show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
	s->addWithLabel("ON-SCREEN HELP", show_help);
	s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts",
			show_help->getState()); });

	// Screensaver.
	ComponentListRow screensaver_row;
	screensaver_row.elements.clear();
	screensaver_row.addElement(std::make_shared<TextComponent>
			(mWindow, "SCREENSAVER SETTINGS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	screensaver_row.addElement(makeArrow(mWindow), false);
	screensaver_row.makeAcceptInputHandler(std::bind(&GuiMenu::openScreensaverOptions, this));
	s->addRow(screensaver_row);

	mWindow->pushGui(s);
}

void GuiMenu::openOtherSettings()
{
	auto s = new GuiSettings(mWindow, "OTHER SETTINGS");

	// Maximum VRAM.
	auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
	max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
	s->addWithLabel("VRAM LIMIT", max_vram);
	s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM",
			(int)Math::round(max_vram->getValue())); });

	// Fullscreen mode.
	auto fullscreen_mode = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "FULLSCREEN MODE", false);
	std::vector<std::string> screenmode;
	screenmode.push_back("normal");
	screenmode.push_back("borderless");
	for (auto it = screenmode.cbegin(); it != screenmode.cend(); it++)
		fullscreen_mode->add(*it, *it, Settings::getInstance()->getString("FullscreenMode") == *it);
	s->addWithLabel("FULLSCREEN MODE (REQUIRES RESTART)", fullscreen_mode);
	s->addSaveFunc([fullscreen_mode] {
		if (Settings::getInstance()->getString("FullscreenMode") == "normal" &&
			fullscreen_mode->getSelected() != "normal") {
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setString("FullscreenMode", fullscreen_mode->getSelected());
	});

	// Power saver.
	auto power_saver = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "POWER SAVER MODES", false);
	std::vector<std::string> modes;
	modes.push_back("disabled");
	modes.push_back("default");
	modes.push_back("enhanced");
	modes.push_back("instant");
	for (auto it = modes.cbegin(); it != modes.cend(); it++)
		power_saver->add(*it, *it, Settings::getInstance()->getString("PowerSaverMode") == *it);
	s->addWithLabel("POWER SAVER MODES", power_saver);
	s->addSaveFunc([this, power_saver] {
		if (Settings::getInstance()->getString("PowerSaverMode") !=
				"instant" && power_saver->getSelected() == "instant") {
			Settings::getInstance()->setString("TransitionStyle", "instant");
			Settings::getInstance()->setBool("MoveCarousel", false);
			Settings::getInstance()->setBool("EnableSounds", false);
		}
		Settings::getInstance()->setString("PowerSaverMode", power_saver->getSelected());
		PowerSaver::init();
	});

	#ifdef _RPI_
	// Video Player - VideoOmxPlayer.
	auto omx_player = std::make_shared<SwitchComponent>(mWindow);
	omx_player->setState(Settings::getInstance()->getBool("VideoOmxPlayer"));
	s->addWithLabel("USE OMX PLAYER (HW ACCELERATED)", omx_player);
	s->addSaveFunc([omx_player]
	{
		// Need to reload all views to re-create the right video components.
		bool needReload = false;
		if (Settings::getInstance()->getBool("VideoOmxPlayer") != omx_player->getState())
			needReload = true;

		Settings::getInstance()->setBool("VideoOmxPlayer", omx_player->getState());

		if (needReload)
			ViewController::get()->reloadAll();
	});

	#endif

	// When to save game metadata.
	auto gamelistsSaveMode = std::make_shared<OptionListComponent<std::string>>
			(mWindow, getHelpStyle(), "SAVE METADATA", false);
	std::vector<std::string> saveModes;
	saveModes.push_back("on exit");
	saveModes.push_back("always");
	saveModes.push_back("never");

	for (auto it = saveModes.cbegin(); it != saveModes.cend(); it++)
		gamelistsSaveMode->add(*it, *it, Settings::getInstance()->
				getString("SaveGamelistsMode") == *it);
	s->addWithLabel("WHEN TO SAVE GAME METADATA", gamelistsSaveMode);
	s->addSaveFunc([gamelistsSaveMode] {
		Settings::getInstance()->setString("SaveGamelistsMode", gamelistsSaveMode->getSelected());
	});

	// Allow overriding of the launch string per game (the option
	// to disable this is intended primarily for testing purposes).
	auto launchstring_override = std::make_shared<SwitchComponent>(mWindow);
	launchstring_override->setState(Settings::getInstance()->getBool("LaunchstringOverride"));
	s->addWithLabel("ENABLE PER GAME LAUNCH STRING OVERRIDE", launchstring_override);
	s->addSaveFunc([launchstring_override] { Settings::getInstance()->
			setBool("LaunchstringOverride", launchstring_override->getState()); });

	auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
	parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
	s->addWithLabel("ONLY SHOW ROMS FROM GAMELIST.XML FILES", parse_gamelists);
	s->addSaveFunc([parse_gamelists] { Settings::getInstance()->
			setBool("ParseGamelistOnly", parse_gamelists->getState()); });

	auto local_art = std::make_shared<SwitchComponent>(mWindow);
	local_art->setState(Settings::getInstance()->getBool("LocalArt"));
	s->addWithLabel("DISPLAY GAME ART FROM ROM DIRECTORIES", local_art);
	s->addSaveFunc([local_art] { Settings::getInstance()->
			setBool("LocalArt", local_art->getState()); });

	// Hidden files.
	auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
	hidden_files->setState(Settings::getInstance()->getBool("ShowHiddenFiles"));
	s->addWithLabel("SHOW HIDDEN FILES", hidden_files);
	s->addSaveFunc([hidden_files] { Settings::getInstance()->setBool("ShowHiddenFiles",
			hidden_files->getState()); });

	// Framerate.
	auto framerate = std::make_shared<SwitchComponent>(mWindow);
	framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
	s->addWithLabel("SHOW FRAMERATE", framerate);
	s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate",
			framerate->getState()); });

	// Hide Reboot System option in the quit menu.
	auto show_rebootsystem = std::make_shared<SwitchComponent>(mWindow);
	show_rebootsystem->setState(Settings::getInstance()->getBool("ShowRebootSystem"));
	s->addWithLabel("SHOW \"REBOOT SYSTEM\" MENU ENTRY", show_rebootsystem);
	s->addSaveFunc([show_rebootsystem] { Settings::getInstance()->setBool("ShowRebootSystem",
			show_rebootsystem->getState()); });

	// Hide Power Off System option in the quit menu.
	auto show_poweroffsystem = std::make_shared<SwitchComponent>(mWindow);
	show_poweroffsystem->setState(Settings::getInstance()->getBool("ShowPoweroffSystem"));
	s->addWithLabel("SHOW \"POWER OFF SYSTEM\" MENU ENTRY", show_poweroffsystem);
	s->addSaveFunc([show_poweroffsystem] { Settings::getInstance()->setBool("ShowPoweroffSystem",
			show_poweroffsystem->getState()); });

	mWindow->pushGui(s);

}

void GuiMenu::openConfigInput()
{
	Window* window = mWindow;
	window->pushGui(new GuiMsgBox(window, getHelpStyle(),
			"ARE YOU SURE YOU WANT TO CONFIGURE INPUT?", "YES", [window] {
		window->pushGui(new GuiDetectDevice(window, false, nullptr));
	}, "NO", nullptr)
	);
}

void GuiMenu::openQuitMenu()
{
	auto s = new GuiSettings(mWindow, "QUIT");

	Window* window = mWindow;
	HelpStyle style = getHelpStyle();

	ComponentListRow row;
	if (UIModeController::getInstance()->isUIModeFull()) {
		if (Settings::getInstance()->getBool("ShowExit")) {
			row.makeAcceptInputHandler([window, this] {
				window->pushGui(new GuiMsgBox(window, this->getHelpStyle(),
					"REALLY QUIT?", "YES", [] {
						Scripting::fireEvent("quit");
						quitES();
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION",
					Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
		}
	}

	if (Settings::getInstance()->getBool("ShowRebootSystem")) {
		row.elements.clear();
		row.makeAcceptInputHandler([window, this] {
			window->pushGui(new GuiMsgBox(window, this->getHelpStyle(),
					"REALLY REBOOT?", "YES", [] {
					Scripting::fireEvent("quit", "reboot");
					Scripting::fireEvent("reboot");
					if (quitES(QuitMode::REBOOT) != 0)
						LOG(LogWarning) << "Reboot terminated with non-zero result!";
			}, "NO", nullptr));
		});
		row.addElement(std::make_shared<TextComponent>(window, "REBOOT SYSTEM",
				Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		s->addRow(row);
	}

	if (Settings::getInstance()->getBool("ShowPoweroffSystem")) {
		row.elements.clear();
		row.makeAcceptInputHandler([window, this] {
			window->pushGui(new GuiMsgBox(window, this->getHelpStyle(),
					"REALLY POWER OFF?", "YES", [] {
					Scripting::fireEvent("quit", "poweroff");
					Scripting::fireEvent("poweroff");
					if (quitES(QuitMode::POWEROFF) != 0)
						LOG(LogWarning) << "Power off terminated with non-zero result!";
			}, "NO", nullptr));
		});
		row.addElement(std::make_shared<TextComponent>(window, "POWER OFF SYSTEM",
				Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		s->addRow(row);
	}

	mWindow->pushGui(s);
}

void GuiMenu::addVersionInfo()
{
	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0x5E5E5EFF);
	mVersion.setText("EMULATIONSTATION-DE  V" + Utils::String::toUpper(PROGRAM_VERSION_STRING));
	mVersion.setHorizontalAlignment(ALIGN_CENTER);
	addChild(&mVersion);
}

void GuiMenu::openScreensaverOptions() {
	mWindow->pushGui(new GuiGeneralScreensaverOptions(mWindow, "SCREENSAVER SETTINGS"));
}

void GuiMenu::openCollectionSystemSettings() {
	mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color,
		bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);

	// Populate the list.
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if (add_arrow) {
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}

	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

void GuiMenu::close(bool closeAllWindows)
{
	std::function<void()> closeFunc;
	if (!closeAllWindows) {
		closeFunc = [this] { delete this; };
	}
	else {
		Window* window = mWindow;
		closeFunc = [window, this] {
			while (window->peekGui() != ViewController::get())
				delete window->peekGui();
		};
	}
	closeFunc();
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	const bool isStart = config->isMappedTo("start", input);
	if (input.value != 0 && (config->isMappedTo("b", input) || isStart)) {
		close(isStart);
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("b", "close menu"));
	prompts.push_back(HelpPrompt("start", "close menu"));
	return prompts;
}

HelpStyle GuiMenu::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}
