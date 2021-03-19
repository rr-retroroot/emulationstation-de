//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  InputManager.h
//
//  Low-level input handling.
//  Initiates and maps the keyboard and controllers.
//  Reads and writes the es_input.cfg configuration file.
//

#ifndef ES_CORE_INPUT_MANAGER_H
#define ES_CORE_INPUT_MANAGER_H

#if defined(__linux__) || defined(_WIN64)
#include <SDL2/SDL_joystick.h>
#else
#include "SDL_joystick.h"
#endif

#include <map>
#include <string>

class InputConfig;
class Window;
union SDL_Event;

// You should only ever instantiate one of these, by the way.
class InputManager
{
public:
    virtual ~InputManager();

    static InputManager* getInstance();

    void writeDeviceConfig(InputConfig* config);
    void doOnFinish();
    static std::string getConfigPath();
    static std::string getTemporaryConfigPath();

    void init();
    void deinit();

    int getNumJoysticks();
    int getAxisCountByDevice(int deviceId);
    int getButtonCountByDevice(int deviceId);
    int getNumConfiguredDevices();

    std::string getDeviceGUIDString(int deviceId);

    InputConfig* getInputConfigByDevice(int deviceId);

    bool parseEvent(const SDL_Event& ev, Window* window);

private:
    InputManager();

    static InputManager* sInstance;

    static const int DEADZONE = 23000;

    void loadDefaultKBConfig();

    std::map<SDL_JoystickID, SDL_Joystick*> mJoysticks;
    std::map<SDL_JoystickID, InputConfig*> mInputConfigs;
    InputConfig* mKeyboardInputConfig;
    InputConfig* mCECInputConfig;

    std::map<SDL_JoystickID, int*> mPrevAxisValues;

    bool initialized() const;

    void addJoystickByDeviceIndex(int id);
    void removeJoystickByJoystickID(SDL_JoystickID id);
    // Returns true if successfully loaded, false if not (or if it didn't exist).
    bool loadInputConfig(InputConfig* config);
};

#endif // ES_CORE_INPUT_MANAGER_H
