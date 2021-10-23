//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  BadgesComponent.cpp
//
//  Game badges icons.
//  Used by the gamelist views.
//

#define SLOT_FAVORITE "favorite"
#define SLOT_COMPLETED "completed"
#define SLOT_KIDGAME "kidgame"
#define SLOT_BROKEN "broken"
#define SLOT_CONTROLLER "controller"
#define SLOT_ALTEMULATOR "altemulator"

#include "components/BadgesComponent.h"

#include "Log.h"
#include "ThemeData.h"
#include "utils/StringUtil.h"

std::vector<ControllerTypes> BadgesComponent::sControllerTypes;

// clang-format off

// The "unknown" controller entry has to be placed last.
ControllerTypes sControllerDefinitions [] = {
    // shortName                      displayName                           fileName
    {"gamepad_generic",               "Gamepad (Generic)",                  ":/graphics/controllers/gamepad_generic.svg"},
    {"gamepad_xbox",                  "Gamepad (Xbox)",                     ":/graphics/controllers/gamepad_xbox.svg"},
    {"gamepad_playstation",           "Gamepad (PlayStation)",              ":/graphics/controllers/gamepad_playstation.svg"},
    {"gamepad_nintendo_nes",          "Gamepad (Nintendo NES)",             ":/graphics/controllers/gamepad_nintendo_nes.svg"},
    {"gamepad_nintendo_snes",         "Gamepad (Nintendo SNES)",            ":/graphics/controllers/gamepad_nintendo_snes.svg"},
    {"gamepad_nintendo_64",           "Gamepad (Nintendo 64)",              ":/graphics/controllers/gamepad_nintendo_64.svg"},
    {"joystick_generic",              "Joystick (Generic)",                 ":/graphics/controllers/joystick_generic.svg"},
    {"joystick_arcade_2_buttons",     "Joystick (Arcade 2 Buttons)",        ":/graphics/controllers/joystick_arcade_2_buttons.svg"},
    {"joystick_arcade_3_buttons",     "Joystick (Arcade 3 Buttons)",        ":/graphics/controllers/joystick_arcade_3_buttons.svg"},
    {"joystick_arcade_4_buttons",     "Joystick (Arcade 4 Buttons)",        ":/graphics/controllers/joystick_arcade_4_buttons.svg"},
    {"joystick_arcade_6_buttons",     "Joystick (Arcade 6 Buttons)",        ":/graphics/controllers/joystick_arcade_6_buttons.svg"},
    {"trackball_generic",             "Trackball (Generic)",                ":/graphics/controllers/trackball_generic.svg"},
    {"lightgun_generic",              "Lightgun (Generic)",                 ":/graphics/controllers/lightgun_generic.svg"},
    {"lightgun_nintendo",             "Lightgun (Nintendo)",                ":/graphics/controllers/lightgun_nintendo.svg"},
    {"keyboard_generic",              "Keyboard (Generic)",                 ":/graphics/controllers/keyboard_generic.svg"},
    {"mouse_generic",                 "Mouse (Generic)",                    ":/graphics/controllers/mouse_generic.svg"},
    {"mouse_amiga",                   "Mouse (Amiga)",                      ":/graphics/controllers/mouse_amiga.svg"},
    {"keyboard_mouse_generic",        "Keyboard and Mouse (Generic)",       ":/graphics/controllers/keyboard_mouse_generic.svg"},
    {"steering_wheel_generic",        "Steering Wheel (Generic)",           ":/graphics/controllers/steering_wheel_generic.svg"},
    {"wii_remote_nintendo",           "Wii Remote (Nintendo)",              ":/graphics/controllers/wii_remote_nintendo.svg"},
    {"wii_remote_nunchuck_nintendo",  "Wii Remote and Nunchuck (Nintendo)", ":/graphics/controllers/wii_remote_nunchuck_nintendo.svg"},
    {"joycon_left_or_right_nintendo", "Joy-Con Left or Right (Nintendo)",   ":/graphics/controllers/joycon_left_or_right_nintendo.svg"},
    {"joycon_pair_nintendo",          "Joy-Con Pair (Nintendo)",            ":/graphics/controllers/joycon_pair_nintendo.svg"},
    {"unknown",                       "Unknown Controller",                 ":/graphics/controllers/unknown.svg"}
};

// clang-format on

BadgesComponent::BadgesComponent(Window* window)
    : GuiComponent{window}
    , mFlexboxItems{}
    , mFlexboxComponent{window, mFlexboxItems}
    , mBadgeTypes{{SLOT_FAVORITE, SLOT_COMPLETED, SLOT_KIDGAME, SLOT_BROKEN, SLOT_CONTROLLER,
                   SLOT_ALTEMULATOR}}
{
    mBadgeIcons[SLOT_FAVORITE] = ":/graphics/badge_favorite.svg";
    mBadgeIcons[SLOT_COMPLETED] = ":/graphics/badge_completed.svg";
    mBadgeIcons[SLOT_KIDGAME] = ":/graphics/badge_kidgame.svg";
    mBadgeIcons[SLOT_BROKEN] = ":/graphics/badge_broken.svg";
    mBadgeIcons[SLOT_CONTROLLER] = ":/graphics/badge_controller.svg";
    mBadgeIcons[SLOT_ALTEMULATOR] = ":/graphics/badge_altemulator.svg";
}

void BadgesComponent::populateControllerTypes()
{
    sControllerTypes.clear();
    for (auto type : sControllerDefinitions)
        sControllerTypes.push_back(type);
}

void BadgesComponent::setBadges(const std::vector<BadgeInfo>& badges)
{
    std::map<std::string, bool> prevVisibility;
    std::map<std::string, std::string> prevPlayers;
    std::map<std::string, std::string> prevController;

    // Save the visibility status to know whether any badges changed.
    for (auto& item : mFlexboxItems) {
        prevVisibility[item.label] = item.visible;
        prevController[item.label] = item.overlayImage.getTexture()->getTextureFilePath();
        item.visible = false;
    }

    for (auto& badge : badges) {
        auto it = std::find_if(
            mFlexboxItems.begin(), mFlexboxItems.end(),
            [badge](FlexboxComponent::FlexboxItem item) { return item.label == badge.badgeType; });

        if (it != mFlexboxItems.end()) {
            it->visible = true;
            if (badge.controllerType != "" &&
                badge.controllerType != it->overlayImage.getTexture()->getTextureFilePath()) {

                auto it2 = std::find_if(sControllerTypes.begin(), sControllerTypes.end(),
                                        [badge](ControllerTypes controllerType) {
                                            return controllerType.shortName == badge.controllerType;
                                        });

                if (it2 != sControllerTypes.cend()) {
                    it->overlayImage.setImage((*it2).fileName);
                }
                else if (badge.controllerType != "")
                    it->overlayImage.setImage(sControllerTypes.back().fileName);
            }
        }
    }

    // Only recalculate the flexbox if any badges changed.
    for (auto& item : mFlexboxItems) {
        if (prevVisibility[item.label] != item.visible ||
            prevController[item.label] != item.label) {
            mFlexboxComponent.onSizeChanged();
            break;
        }
    }
}

const std::string BadgesComponent::getShortName(const std::string& displayName)
{
    auto it = std::find_if(sControllerTypes.begin(), sControllerTypes.end(),
                           [displayName](ControllerTypes controllerType) {
                               return controllerType.displayName == displayName;
                           });
    if (it != sControllerTypes.end())
        return (*it).shortName;
    else
        return "unknown";
}

const std::string BadgesComponent::getDisplayName(const std::string& shortName)
{
    auto it = std::find_if(sControllerTypes.begin(), sControllerTypes.end(),
                           [shortName](ControllerTypes controllerType) {
                               return controllerType.shortName == shortName;
                           });
    if (it != sControllerTypes.end())
        return (*it).displayName;
    else
        return "unknown";
}

void BadgesComponent::render(const glm::mat4& parentTrans)
{
    if (!isVisible())
        return;

    if (mOpacity == 255) {
        mFlexboxComponent.render(parentTrans);
    }
    else {
        mFlexboxComponent.setOpacity(mOpacity);
        mFlexboxComponent.render(parentTrans);
        mFlexboxComponent.setOpacity(255);
    }
}

void BadgesComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                 const std::string& view,
                                 const std::string& element,
                                 unsigned int properties)
{
    populateControllerTypes();

    using namespace ThemeFlags;

    const ThemeData::ThemeElement* elem{theme->getElement(view, element, "badges")};
    if (!elem)
        return;

    if (elem->has("alignment")) {
        const std::string alignment{elem->get<std::string>("alignment")};
        if (alignment != "left" && alignment != "right") {
            LOG(LogWarning) << "BadgesComponent: Invalid theme configuration, <alignment> set to \""
                            << alignment << "\"";
        }
        else {
            mFlexboxComponent.setAlignment(alignment);
        }
    }

    if (elem->has("itemsPerRow")) {
        const float itemsPerRow{elem->get<float>("itemsPerRow")};
        if (itemsPerRow < 1.0f || itemsPerRow > 10.0f) {
            LOG(LogWarning)
                << "BadgesComponent: Invalid theme configuration, <itemsPerRow> set to \""
                << itemsPerRow << "\"";
        }
        else {
            mFlexboxComponent.setItemsPerLine(static_cast<unsigned int>(itemsPerRow));
        }
    }

    if (elem->has("rows")) {
        const float rows{elem->get<float>("rows")};
        if (rows < 1.0f || rows > 10.0f) {
            LOG(LogWarning) << "BadgesComponent: Invalid theme configuration, <rows> set to \""
                            << rows << "\"";
        }
        else {
            mFlexboxComponent.setLines(static_cast<unsigned int>(rows));
        }
    }

    if (elem->has("itemMargin")) {
        glm::vec2 itemMargin = elem->get<glm::vec2>("itemMargin");
        if ((itemMargin.x != -1.0 && itemMargin.y != -1.0) &&
            (itemMargin.x < 0.0f || itemMargin.x > 0.2f || itemMargin.y < 0.0f ||
             itemMargin.y > 0.2f)) {
            LOG(LogWarning)
                << "BadgesComponent: Invalid theme configuration, <itemMargin> set to \""
                << itemMargin.x << " " << itemMargin.y << "\"";
        }
        else {
            mFlexboxComponent.setItemMargin(itemMargin);
        }
    }

    if (elem->has("controllerPos")) {
        const glm::vec2 controllerPos = elem->get<glm::vec2>("controllerPos");
        if (controllerPos.x < -1.0f || controllerPos.x > 2.0f || controllerPos.y < -1.0f ||
            controllerPos.y > 2.0f) {
            LOG(LogWarning)
                << "BadgesComponent: Invalid theme configuration, <controllerPos> set to \""
                << controllerPos.x << " " << controllerPos.y << "\"";
        }
        else {
            mFlexboxComponent.setOverlayPosition(controllerPos);
        }
    }

    if (elem->has("controllerSize")) {
        const float controllerSize = elem->get<float>("controllerSize");
        if (controllerSize < 0.1f || controllerSize > 2.0f) {
            LOG(LogWarning)
                << "BadgesComponent: Invalid theme configuration, <controllerSize> set to \""
                << controllerSize << "\"";
        }
        else {
            mFlexboxComponent.setOverlaySize(controllerSize);
        }
    }

    if (elem->has("slots")) {
        // Replace possible whitespace separators with commas.
        std::string slotsTag = Utils::String::toLower(elem->get<std::string>("slots"));
        for (auto& character : slotsTag) {
            if (std::isspace(character))
                character = ',';
        }
        slotsTag = Utils::String::replace(slotsTag, ",,", ",");
        std::vector<std::string> slots = Utils::String::delimitedStringToVector(slotsTag, ",");

        for (auto slot : slots) {
            if (std::find(mBadgeTypes.cbegin(), mBadgeTypes.cend(), slot) != mBadgeTypes.end()) {
                if (properties & PATH && elem->has(slot))
                    mBadgeIcons[slot] = elem->get<std::string>(slot);

                FlexboxComponent::FlexboxItem item;
                item.label = slot;

                ImageComponent badgeImage{mWindow};
                badgeImage.setForceLoad(true);
                badgeImage.setImage(mBadgeIcons[slot]);
                item.baseImage = badgeImage;
                item.overlayImage = ImageComponent{mWindow};

                mFlexboxItems.push_back(item);
            }
            else {
                LOG(LogError) << "Invalid badge slot \"" << slot << "\" defined";
            }
        }

        for (auto& controllerType : sControllerTypes) {
            if (properties & PATH && elem->has(controllerType.shortName))
                controllerType.fileName = elem->get<std::string>(controllerType.shortName);
        }

        GuiComponent::applyTheme(theme, view, element, properties);

        mFlexboxComponent.setPosition(mPosition);
        mFlexboxComponent.setSize(mSize);
        mFlexboxComponent.setOrigin(mOrigin);
        mFlexboxComponent.setRotation(mRotation);
        mFlexboxComponent.setRotationOrigin(mRotationOrigin);
        mFlexboxComponent.setVisible(mVisible);
        mFlexboxComponent.setDefaultZIndex(mDefaultZIndex);
        mFlexboxComponent.setZIndex(mZIndex);
    }
}
