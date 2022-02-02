//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  ISimpleGameListView.h
//
//  Interface that defines a simple gamelist view.
//

#ifndef ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H

#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "views/gamelist/IGameListView.h"

#include <stack>

class ISimpleGameListView : public IGameListView
{
public:
    ISimpleGameListView(Window* window, FileData* root);
    virtual ~ISimpleGameListView();

    // Called when a FileData* is added, has its metadata changed, or is removed.
    virtual void onFileChanged(FileData* file, bool reloadGameList) override;

    // Called whenever the theme changes.
    virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

    virtual FileData* getCursor() override = 0;
    virtual void setCursor(FileData*) override = 0;
    virtual void addPlaceholder(FileData*) override = 0;

    virtual bool input(InputConfig* config, Input input) override;
    virtual void launch(FileData* game) override = 0;

    virtual const std::vector<std::string>& getFirstLetterIndex() override = 0;

    // These functions are used to retain the folder cursor history, for instance
    // during a view reload. The calling function stores the history temporarily.
    void copyCursorHistory(std::vector<FileData*>& cursorHistory) override
    {
        cursorHistory = mCursorStackHistory;
    };
    void populateCursorHistory(std::vector<FileData*>& cursorHistory) override
    {
        mCursorStackHistory = cursorHistory;
    };

protected:
    virtual std::string getQuickSystemSelectRightButton() = 0;
    virtual std::string getQuickSystemSelectLeftButton() = 0;
    virtual void populateList(const std::vector<FileData*>& files, FileData* firstEntry) = 0;

    void generateGamelistInfo(FileData* cursor, FileData* firstEntry);
    void generateFirstLetterIndex(const std::vector<FileData*>& files);

    TextComponent mHeaderText;
    ImageComponent mHeaderImage;
    ImageComponent mBackground;

    std::vector<GuiComponent*> mThemeExtras;
    std::stack<FileData*> mCursorStack;
    std::vector<FileData*> mCursorStackHistory;
    // This game is randomly selected in the grouped custom collections view.
    FileData* mRandomGame;

    std::vector<std::string> mFirstLetterIndex;

    unsigned int mGameCount;
    unsigned int mFavoritesGameCount;
    unsigned int mFilteredGameCount;
    unsigned int mFilteredGameCountAll;
    bool mIsFiltered;
    bool mIsFolder;
};

#endif // ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H