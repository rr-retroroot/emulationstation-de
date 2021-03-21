//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  Sound.cpp
//
//  Higher-level audio functions.
//  Reading theme sound setings, playing audio samples etc.
//

#include "Sound.h"

#include "resources/ResourceManager.h"
#include "AudioManager.h"
#include "Log.h"
#include "Settings.h"
#include "ThemeData.h"

NavigationSounds* NavigationSounds::sInstance = nullptr;

std::map<std::string, std::shared_ptr<Sound>> Sound::sMap;

std::shared_ptr<Sound> Sound::get(const std::string& path)
{
    auto it = sMap.find(path);
    if (it != sMap.cend())
        return it->second;

    std::shared_ptr<Sound> sound = std::shared_ptr<Sound>(new Sound(path));
    AudioManager::getInstance()->registerSound(sound);
    sMap[path] = sound;
    return sound;
}

std::shared_ptr<Sound> Sound::getFromTheme(const std::shared_ptr<ThemeData>& theme,
        const std::string& view, const std::string& element)
{
    if (!theme) {
        LOG(LogDebug) << "Sound::getFromTheme(): Using fallback sound file for \""
                << element << "\"";
        return get(ResourceManager::getInstance()->getResourcePath(":/sounds/" + element + ".wav"));
    }

    LOG(LogDebug) << "Sound::getFromTheme(): Looking for tag <sound name=\"" << element << "\">";

    const ThemeData::ThemeElement* elem = theme->getElement(view, element, "sound");
    if (!elem || !elem->has("path")) {
        LOG(LogDebug) << "Sound::getFromTheme(): " << "Tag not found, using fallback sound file";
        return get(ResourceManager::getInstance()->getResourcePath(":/sounds/" + element + ".wav"));
    }

    LOG(LogDebug) << "Sound::getFromTheme(): Tag found, ready to load theme sound file";
    return get(elem->get<std::string>("path"));
}

Sound::Sound(
        const std::string& path)
        : mSampleData(nullptr),
        mSamplePos(0),
        mSampleLength(0),
        playing(false)
{
    loadFile(path);
}

Sound::~Sound()
{
}

void Sound::loadFile(const std::string& path)
{
    mPath = path;
    init();
}

void Sound::init()
{
    if (mSampleData != nullptr)
        deinit();

    if (mPath.empty())
        return;

    // Load WAV file via SDL.
    SDL_AudioSpec wave;
    Uint8* data = nullptr;
    Uint32 dlen = 0;
    if (SDL_LoadWAV(mPath.c_str(), &wave, &data, &dlen) == nullptr) {
        LOG(LogError) << "Failed to load theme navigation sound file:";
        LOG(LogError) << SDL_GetError();
        return;
    }

    // Convert sound file to the format required by ES-DE.
    SDL_AudioStream *conversionStream = SDL_NewAudioStream(wave.format, wave.channels, wave.freq,
            AudioManager::sAudioFormat.format, AudioManager::sAudioFormat.channels,
            AudioManager::sAudioFormat.freq);

    if (conversionStream == nullptr) {
        LOG(LogError) << "Failed to create sample conversion stream:";
        LOG(LogError) << SDL_GetError();
        return;
    }

    if (SDL_AudioStreamPut(conversionStream, data, dlen) == -1) {
        LOG(LogError) << "Failed to put samples in the conversion stream:";
        LOG(LogError) << SDL_GetError();
        SDL_FreeAudioStream(conversionStream);
        return;
    }

    int sampleLength = SDL_AudioStreamAvailable(conversionStream);

    Uint8* converted = new Uint8[sampleLength];
    if (SDL_AudioStreamGet(conversionStream, converted, sampleLength) == -1) {
        LOG(LogError) << "Failed to convert sound file '" << mPath << "':";
        LOG(LogError) << SDL_GetError();
        SDL_FreeAudioStream(conversionStream);
        delete[] converted;
        return;
    }

    mSampleData = converted;
    mSampleLength = sampleLength;
    mSamplePos = 0;
    mSampleFormat.freq = AudioManager::sAudioFormat.freq;
    mSampleFormat.channels = AudioManager::sAudioFormat.channels;
    mSampleFormat.format = AudioManager::sAudioFormat.format;
    SDL_FreeAudioStream(conversionStream);
    SDL_FreeWAV(data);
}

void Sound::deinit()
{
    playing = false;

    if (mSampleData != nullptr) {
        SDL_LockAudioDevice(AudioManager::sAudioDevice);
        delete[] mSampleData;
        mSampleData = nullptr;
        mSampleLength = 0;
        mSamplePos = 0;
        SDL_UnlockAudioDevice(AudioManager::sAudioDevice);
        sMap.erase(mPath);
    }
}

void Sound::play()
{
    if (mSampleData == nullptr)
        return;

    if (!Settings::getInstance()->getBool("NavigationSounds"))
        return;

    if (!AudioManager::getInstance()->getHasAudioDevice())
        return;

    SDL_LockAudioDevice(AudioManager::sAudioDevice);

    if (playing)
        // Replay from start. rewind the sample to the beginning.
        mSamplePos = 0;
    else
        // Flag our sample as playing.
        playing = true;

    SDL_UnlockAudioDevice(AudioManager::sAudioDevice);
    // Tell the AudioManager to start playing samples.
    AudioManager::getInstance()->play();
}

bool Sound::isPlaying() const
{
    return playing;
}

void Sound::stop()
{
    // Flag our sample as not playing and rewind its position.
    SDL_LockAudioDevice(AudioManager::sAudioDevice);
    playing = false;
    mSamplePos = 0;
    SDL_UnlockAudioDevice(AudioManager::sAudioDevice);
}

const Uint8* Sound::getData() const
{
    return mSampleData;
}

Uint32 Sound::getPosition() const
{
    return mSamplePos;
}

void Sound::setPosition(Uint32 newPosition)
{
    mSamplePos = newPosition;
    if (mSamplePos >= mSampleLength) {
        // Got to or beyond the end of the sample. stop playing.
        playing = false;
        mSamplePos = 0;
    }
}

Uint32 Sound::getLength() const
{
    return mSampleLength;
}

NavigationSounds* NavigationSounds::getInstance()
{
    if (sInstance == nullptr)
        sInstance = new NavigationSounds();

    return sInstance;
}

void NavigationSounds::deinit()
{
    if (sInstance) {
        for (auto sound : navigationSounds) {
            AudioManager::getInstance()->unregisterSound(sound);
            sound->deinit();
        }
        navigationSounds.clear();
        delete sInstance;
    }

    sInstance = nullptr;
}

void NavigationSounds::loadThemeNavigationSounds(const std::shared_ptr<ThemeData>& theme)
{
    if (theme) {
        LOG(LogDebug) << "NavigationSounds::loadThemeNavigationSounds(): "
                "Theme set includes navigation sound support, loading custom sounds";
    }
    else {
        LOG(LogDebug) << "NavigationSounds::loadThemeNavigationSounds(): "
                "Theme set does not include navigation sound support, using fallback sounds";
    }

    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "systembrowse")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "quicksysselect")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "select")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "back")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "scroll")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "favorite")));
    navigationSounds.push_back(std::move(Sound::getFromTheme(theme, "all", "launch")));
}

void NavigationSounds::playThemeNavigationSound(NavigationSoundsID soundID)
{
    NavigationSounds::getInstance()->navigationSounds[soundID]->play();
}

bool NavigationSounds::isPlayingThemeNavigationSound(NavigationSoundsID soundID)
{
    return NavigationSounds::getInstance()->navigationSounds[soundID]->isPlaying();
}
