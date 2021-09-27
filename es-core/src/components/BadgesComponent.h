//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  BadgesComponent.h
//
//  Game badges icons.
//  Used by gamelist views.
//

#ifndef ES_CORE_COMPONENTS_BADGES_COMPONENT_H
#define ES_CORE_COMPONENTS_BADGES_COMPONENT_H

#include "FlexboxComponent.h"
#include "GuiComponent.h"
#include "ImageComponent.h"
#include "renderers/Renderer.h"

#define NUM_SLOTS 4
#define SLOT_FAVORITE "favorite"
#define SLOT_COMPLETED "completed"
#define SLOT_KIDS "kidgame"
#define SLOT_BROKEN "broken"
#define SLOT_ALTERNATIVE_EMULATOR "altemulator"

class TextureResource;

class BadgesComponent : public FlexboxComponent
{
public:
    BadgesComponent(Window* window);
    ~BadgesComponent();

    static std::shared_ptr<BadgesComponent>& getInstance();

    std::string getValue() const override;
    // Should be a list of strings.
    void setValue(const std::string& value) override;

    virtual void applyTheme(const std::shared_ptr<ThemeData>& theme,
                            const std::string& view,
                            const std::string& element,
                            unsigned int properties) override;

private:
    static std::vector<std::string> mSlots;
    std::map<std::string, std::string> mBadgeIcons;
    std::map<std::string, ImageComponent> mImageComponents;
};

#endif // ES_CORE_COMPONENTS_BADGES_COMPONENT_H
