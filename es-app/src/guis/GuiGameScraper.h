//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiGameScraper.h
//
//  Single game scraping user interface.
//  This interface is triggered from GuiMetaDataEd.
//  GuiScraperSearch is called from here.
//

#ifndef ES_APP_GUIS_GUI_GAME_SCRAPER_H
#define ES_APP_GUIS_GUI_GAME_SCRAPER_H

#include "GuiComponent.h"
#include "components/NinePatchComponent.h"
#include "components/ScrollIndicatorComponent.h"
#include "guis/GuiScraperSearch.h"

class GuiGameScraper : public GuiComponent
{
public:
    GuiGameScraper(Window* window,
                   ScraperSearchParams& params,
                   std::function<void(const ScraperSearchResult&)> doneFunc,
                   bool& savedMediaAndAborted);

    void onSizeChanged() override;

    bool input(InputConfig* config, Input input) override;
    void update(int deltaTime) override;

    virtual std::vector<HelpPrompt> getHelpPrompts() override;
    HelpStyle getHelpStyle() override;

private:
    bool mClose;
    void close();

    ComponentGrid mGrid;
    NinePatchComponent mBox;

    std::shared_ptr<TextComponent> mGameName;
    std::shared_ptr<ImageComponent> mScrollUp;
    std::shared_ptr<ImageComponent> mScrollDown;
    std::shared_ptr<ScrollIndicatorComponent> mScrollIndicator;
    std::shared_ptr<TextComponent> mSystemName;
    std::shared_ptr<GuiScraperSearch> mSearch;
    std::shared_ptr<ComponentGrid> mButtonGrid;
    std::shared_ptr<ComponentList> mResultList;

    ScraperSearchParams mSearchParams;
    bool& mSavedMediaAndAborted;

    std::function<void()> mCancelFunc;
};

#endif // ES_APP_GUIS_GUI_GAME_SCRAPER_H
