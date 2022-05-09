//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiGamelistOptions.cpp
//
//  Gamelist options menu for the 'Jump to...' quick selector,
//  game sorting, game filters, and metadata edit.
//
//  The filter interface is covered by GuiGamelistFilter and the
//  metadata edit interface is covered by GuiMetaDataEd.
//

#include "GuiGamelistOptions.h"

#include "CollectionSystemsManager.h"
#include "FileFilterIndex.h"
#include "FileSorts.h"
#include "GuiMetaDataEd.h"
#include "MameNames.h"
#include "Sound.h"
#include "SystemData.h"
#include "guis/GuiGamelistFilter.h"
#include "scrapers/Scraper.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "views/gamelist/IGameListView.h"

GuiGamelistOptions::GuiGamelistOptions(Window* window, SystemData* system)
    : GuiComponent(window)
    , mMenu(window, "OPTIONS")
    , mSystem(system)
    , mFiltersChanged(false)
    , mCancelled(false)
    , mIsCustomCollection(false)
    , mIsCustomCollectionGroup(false)
    , mCustomCollectionSystem(nullptr)
{
    addChild(&mMenu);

    FileData* file = getGamelist()->getCursor();
    // Check if it's a placeholder, which would limit the menu entries presented.
    file->isPlaceHolder();
    mFromPlaceholder = file->isPlaceHolder();
    ComponentListRow row;

    // There is some special logic required for custom collections.
    if (file->getSystem()->isCustomCollection() && file->getPath() != file->getSystem()->getName())
        mIsCustomCollection = true;
    else if (file->getSystem()->isCustomCollection() &&
             file->getPath() == file->getSystem()->getName())
        mIsCustomCollectionGroup = true;

    if (mFromPlaceholder && file->getSystem()->isGroupedCustomCollection())
        mCustomCollectionSystem = file->getSystem();

    // Read the setting for whether folders are sorted on top of the gamelists.
    // Also check if the gamelist only contains folders, as generated by the FileData sorting.
    mFoldersOnTop = Settings::getInstance()->getBool("FoldersOnTop");
    if (file->getType() != PLACEHOLDER)
        mOnlyHasFolders = file->getParent()->getOnlyFoldersFlag();

    // Read the applicable favorite sorting setting depending on whether the
    // system is a custom collection or not.
    if (mIsCustomCollection)
        mFavoritesSorting = Settings::getInstance()->getBool("FavFirstCustom");
    else
        mFavoritesSorting = Settings::getInstance()->getBool("FavoritesFirst");

    if (!mFromPlaceholder) {
        // Jump to letter quick selector.
        row.elements.clear();

        // The letter index is generated in FileData during gamelist sorting.
        mFirstLetterIndex = getGamelist()->getFirstLetterIndex();

        // Don't include the folder name starting characters if folders are sorted on top
        // unless the list only contains folders.
        if (!mOnlyHasFolders && mFoldersOnTop && file->getType() == FOLDER) {
            mCurrentFirstCharacter = ViewController::FOLDER_CHAR;
        }
        else {
            // Check if the currently selected game is a favorite.
            bool isFavorite = false;
            if (mFirstLetterIndex.size() == 1 &&
                mFirstLetterIndex.front() == ViewController::FAVORITE_CHAR)
                isFavorite = true;
            else if (mFirstLetterIndex.size() > 1 &&
                     (mFirstLetterIndex.front() == ViewController::FAVORITE_CHAR ||
                      mFirstLetterIndex[1] == ViewController::FAVORITE_CHAR))
                isFavorite = true;

            // Get the first character of the game name (which could be a Unicode character).
            if (mFavoritesSorting && file->getFavorite() && isFavorite)
                mCurrentFirstCharacter = ViewController::FAVORITE_CHAR;
            else
                mCurrentFirstCharacter = Utils::String::getFirstCharacter(file->getSortName());
        }

        mJumpToLetterList =
            std::make_shared<LetterList>(mWindow, getHelpStyle(), "JUMP TO...", false);

        // Enable key repeat so that the left or right button can be held to cycle through
        // the letters.
        mJumpToLetterList->setKeyRepeat(true, 650, 200);

        // Populate the quick selector.
        for (unsigned int i = 0; i < mFirstLetterIndex.size(); ++i) {
            mJumpToLetterList->add(mFirstLetterIndex[i], mFirstLetterIndex[i], 0);
            if (mFirstLetterIndex[i] == mCurrentFirstCharacter)
                mJumpToLetterList->selectEntry(i);
        }

        if (system->getName() != "recent")
            mMenu.addWithLabel("JUMP TO..", mJumpToLetterList);

        // Add the sorting entry, unless this is the grouped custom collections list.
        if (!mIsCustomCollectionGroup) {
            // Sort list by selected sort type (persistent throughout the program session).
            mListSort = std::make_shared<SortList>(mWindow, getHelpStyle(), "SORT GAMES BY", false);
            FileData* root;
            if (mIsCustomCollection)
                root = getGamelist()->getCursor()->getSystem()->getRootFolder();
            else
                root = mSystem->getRootFolder();

            std::string sortType = root->getSortTypeString();
            unsigned int numSortTypes = static_cast<unsigned int>(FileSorts::SortTypes.size());
            // If it's not a collection, then hide the System sort options.
            if (!root->getSystem()->isCollection())
                numSortTypes -= 2;

            for (unsigned int i = 0; i < numSortTypes; ++i) {
                const FileData::SortType& sort = FileSorts::SortTypes.at(i);
                if (sort.description == sortType)
                    mListSort->add(sort.description, &sort, true);
                else
                    mListSort->add(sort.description, &sort, false);
            }

            // Enable key repeat so that the left or right button can be held to cycle through
            // the sort options.
            mListSort->setKeyRepeat(true, 650, 400);

            // Don't show the sort type option if the gamelist type is recent/last played.
            if (system->getName() != "recent")
                mMenu.addWithLabel("SORT GAMES BY", mListSort);
        }
    }

    // Add the filters entry, unless this is the grouped custom collections system or if there
    // are no games for the system.
    if (!mIsCustomCollectionGroup && system->getRootFolder()->getChildren().size() > 0) {
        if (system->getName() != "recent" && Settings::getInstance()->getBool("GamelistFilters")) {
            row.elements.clear();
            row.addElement(std::make_shared<TextComponent>(mWindow, "FILTER GAMELIST",
                                                           Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                           true);
            row.addElement(makeArrow(mWindow), false);
            row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openGamelistFilter, this));
            mMenu.addRow(row);
        }
    }
    // Add a dummy entry when applicable as the menu looks quite ugly if it's just blank.
    else if (!CollectionSystemsManager::get()->isEditing() &&
             mSystem->getRootFolder()->getChildren().size() == 0 && !mIsCustomCollectionGroup &&
             !mIsCustomCollection) {
        row.elements.clear();
        row.addElement(std::make_shared<TextComponent>(mWindow, "THIS SYSTEM HAS NO GAMES",
                                                       Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                       true);
        mMenu.addRow(row);
    }

    std::string customSystem;
    if (Settings::getInstance()->getBool("UseCustomCollectionsSystem"))
        customSystem = file->getSystem()->getName();
    else
        customSystem = system->getName();

    if (UIModeController::getInstance()->isUIModeFull() &&
        (mIsCustomCollection || mIsCustomCollectionGroup)) {
        if (CollectionSystemsManager::get()->getEditingCollection() != customSystem) {
            row.elements.clear();
            row.addElement(std::make_shared<TextComponent>(mWindow,
                                                           "ADD/REMOVE GAMES TO THIS COLLECTION",
                                                           Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                           true);
            row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::startEditMode, this));
            mMenu.addRow(row);
        }
    }

    if (UIModeController::getInstance()->isUIModeFull() &&
        CollectionSystemsManager::get()->isEditing()) {
        row.elements.clear();
        row.addElement(std::make_shared<TextComponent>(
                           mWindow,
                           "FINISH EDITING '" +
                               Utils::String::toUpper(
                                   CollectionSystemsManager::get()->getEditingCollection()) +
                               "' COLLECTION",
                           Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                       true);
        row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::exitEditMode, this));
        mMenu.addRow(row);
    }

    if (file->getType() == FOLDER) {
        if (UIModeController::getInstance()->isUIModeFull() && !mFromPlaceholder &&
            !(mSystem->isCollection() && file->getType() == FOLDER)) {
            row.elements.clear();
            row.addElement(std::make_shared<TextComponent>(mWindow, "EDIT THIS FOLDER'S METADATA",
                                                           Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                           true);
            row.addElement(makeArrow(mWindow), false);
            row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openMetaDataEd, this));
            mMenu.addRow(row);
        }
    }
    else {
        if (UIModeController::getInstance()->isUIModeFull() && !mFromPlaceholder &&
            !(mSystem->isCollection() && file->getType() == FOLDER)) {
            row.elements.clear();
            row.addElement(std::make_shared<TextComponent>(mWindow, "EDIT THIS GAME'S METADATA",
                                                           Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
                           true);
            row.addElement(makeArrow(mWindow), false);
            row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openMetaDataEd, this));
            mMenu.addRow(row);
        }
    }

    // Buttons. The logic to apply or cancel settings are handled by the destructor.
    if ((!mIsCustomCollectionGroup && system->getRootFolder()->getChildren().size() == 0) ||
        system->getName() == "recent") {
        mMenu.addButton("CLOSE", "close", [&] {
            mCancelled = true;
            delete this;
        });
    }
    else {
        mMenu.addButton("APPLY", "apply", [&] { delete this; });
        mMenu.addButton("CANCEL", "cancel", [&] {
            mCancelled = true;
            delete this;
        });
    }

    // Center the menu.
    setSize(static_cast<float>(Renderer::getScreenWidth()),
            static_cast<float>(Renderer::getScreenHeight()));
    mMenu.setPosition((mSize.x - mMenu.getSize().x) / 2.0f, (mSize.y - mMenu.getSize().y) / 2.0f);
}

GuiGamelistOptions::~GuiGamelistOptions()
{
    // This is required for the situation where scrolling started just before the menu
    // was openened. Without this, the scrolling would run until manually stopped after
    // the menu has been closed.
    ViewController::get()->stopScrolling();

    if (mFiltersChanged) {
        if (!mCustomCollectionSystem) {
            ViewController::get()->reloadGameListView(mSystem);
        }
        else {
            if (!mFromPlaceholder) {
                ViewController::get()->reloadGameListView(mSystem);
            }
            else if (!mCustomCollectionSystem->getRootFolder()
                          ->getChildrenListToDisplay()
                          .empty()) {
                ViewController::get()->reloadGameListView(mSystem);
                getGamelist()->setCursor(
                    mCustomCollectionSystem->getRootFolder()->getChildrenListToDisplay().front());
            }
        }
    }

    if (mCancelled)
        return;

    if (!mFromPlaceholder) {
        FileData* root;

        if (mIsCustomCollection)
            root = getGamelist()->getCursor()->getSystem()->getRootFolder();
        else
            root = mSystem->getRootFolder();

        // If a new sorting type was selected, then sort and update mSortTypeString for the system.
        if (!mIsCustomCollectionGroup &&
            (*mListSort->getSelected()).description != root->getSortTypeString()) {
            // This will also recursively sort children.
            root->sort(*mListSort->getSelected(), mFavoritesSorting);
            root->setSortTypeString((*mListSort->getSelected()).description);

            // Notify that the root folder was sorted (refresh).
            getGamelist()->onFileChanged(root, false);
        }

        // Has the user changed the letter using the quick selector?
        if (mCurrentFirstCharacter != mJumpToLetterList->getSelected()) {
            if (mJumpToLetterList->getSelected() == ViewController::FAVORITE_CHAR ||
                mJumpToLetterList->getSelected() == ViewController::FOLDER_CHAR)
                jumpToFirstRow();
            else
                jumpToLetter();
        }
    }

    if (mSystem->getRootFolder()->getChildren().size() != 0 && mSystem->getName() != "recent")
        NavigationSounds::getInstance().playThemeNavigationSound(SCROLLSOUND);
}

void GuiGamelistOptions::openGamelistFilter()
{
    GuiGamelistFilter* ggf;

    auto filtersChangedFunc = [this](bool filtersChanged) {
        if (!mFiltersChanged)
            mFiltersChanged = filtersChanged;
    };

    if (mIsCustomCollection)
        ggf = new GuiGamelistFilter(mWindow, getGamelist()->getCursor()->getSystem(),
                                    filtersChangedFunc);
    else
        ggf = new GuiGamelistFilter(mWindow, mSystem, filtersChangedFunc);

    mWindow->pushGui(ggf);
}

void GuiGamelistOptions::startEditMode()
{
    std::string editingSystem = mSystem->getName();
    // Need to check if we're editing the collections bundle,
    // as we will want to edit the selected collection within.
    if (editingSystem == CollectionSystemsManager::get()->getCustomCollectionsBundle()->getName()) {
        FileData* file = getGamelist()->getCursor();
        // Do we have the cursor on a specific collection?.
        if (file->getType() == FOLDER)
            editingSystem = file->getName();
        else
            // We are inside a specific collection. We want to edit that one.
            editingSystem = file->getSystem()->getName();
    }
    CollectionSystemsManager::get()->setEditMode(editingSystem);

    // Display the indication icons which show what games are part of the custom collection
    // currently being edited. This is done cheaply using onFileChanged() which will trigger
    // populateList().
    for (auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); ++it) {
        ViewController::get()->getGameListView((*it))->onFileChanged(
            ViewController::get()->getGameListView((*it))->getCursor(), false);
    }

    if (mSystem->getRootFolder()->getChildren().size() == 0)
        NavigationSounds::getInstance().playThemeNavigationSound(SCROLLSOUND);
    delete this;
}

void GuiGamelistOptions::exitEditMode()
{
    CollectionSystemsManager::get()->exitEditMode();
    if (mSystem->getRootFolder()->getChildren().size() == 0)
        NavigationSounds::getInstance().playThemeNavigationSound(SCROLLSOUND);
    delete this;
}

void GuiGamelistOptions::openMetaDataEd()
{
    // Open metadata editor.
    // Get the FileData that holds the original metadata.
    FileData* file = getGamelist()->getCursor()->getSourceFileData();
    ScraperSearchParams p;
    p.game = file;
    p.system = file->getSystem();

    std::function<void()> clearGameBtnFunc;
    std::function<void()> deleteGameBtnFunc;

    clearGameBtnFunc = [this, file] {
        if (file->getType() == FOLDER) {
            LOG(LogInfo) << "Deleting the media files and gamelist.xml entry for the folder \""
                         << file->getFullPath() << "\"";
        }
        else if (file->getType() == GAME && Utils::FileSystem::isDirectory(file->getFullPath())) {
            LOG(LogInfo) << "Deleting the media files and gamelist.xml entry for the "
                            "file-interpreted folder \""
                         << file->getFullPath() << "\"";
        }
        else {
            LOG(LogInfo) << "Deleting the media files and gamelist.xml entry for the file \""
                         << file->getFullPath() << "\"";
        }
        ViewController::get()->getGameListView(file->getSystem()).get()->removeMedia(file);

        // Manually reset all the metadata values, set the name to the actual file/folder name.
        const std::vector<MetaDataDecl>& mdd = file->metadata.getMDD();
        for (auto it = mdd.cbegin(); it != mdd.cend(); ++it) {
            if (it->key == "name") {
                if (file->isArcadeGame()) {
                    // If it's a MAME or Neo Geo game, expand the game name accordingly.
                    file->metadata.set(it->key,
                                       MameNames::getInstance().getCleanName(file->getCleanName()));
                }
                else {
                    file->metadata.set(it->key, file->getDisplayName());
                }
                continue;
            }
            file->metadata.set(it->key, it->defaultValue);
        }

        // For the special case where a directory has a supported file extension and is therefore
        // interpreted as a file, don't include the extension in the metadata name.
        if (file->getType() == GAME && Utils::FileSystem::isDirectory(file->getFullPath()))
            file->metadata.set("name", Utils::FileSystem::getStem(file->metadata.get("name")));

        // Update all collections where the game is present.
        if (file->getType() == GAME)
            CollectionSystemsManager::get()->refreshCollectionSystems(file, true);

        file->getSystem()->sortSystem();
        // This delay reduces the likelyhood that the SVG rasterizer which is running in a
        // separate thread is not done until the cached background is invalidated. Without
        // this delay there's a high chance that some theme elements are not rendered in
        // time and thus not getting included in the regenerated cached background.
        // This is just a hack though and a better mechanism is needed to handle this.
        SDL_Delay(100);
        mWindow->invalidateCachedBackground();

        // Remove the folder entry from the gamelist.xml file.
        file->setDeletionFlag(true);
        file->getParent()->getSystem()->writeMetaData();
        file->setDeletionFlag(false);
    };

    deleteGameBtnFunc = [this, file] {
        LOG(LogInfo) << "Deleting the game file \"" << file->getFullPath()
                     << "\", all its media files and its gamelist.xml entry.";
        CollectionSystemsManager::get()->deleteCollectionFiles(file);
        ViewController::get()->getGameListView(file->getSystem()).get()->removeMedia(file);
        ViewController::get()->getGameListView(file->getSystem()).get()->remove(file, true);
        mSystem->getRootFolder()->sort(*mListSort->getSelected(), mFavoritesSorting);
        ViewController::get()->reloadGameListView(mSystem);

        mWindow->invalidateCachedBackground();
    };

    if (file->getType() == FOLDER) {
        mWindow->pushGui(new GuiMetaDataEd(
            mWindow, &file->metadata, file->metadata.getMDD(FOLDER_METADATA), p,
            Utils::FileSystem::getFileName(file->getPath()),
            std::bind(&IGameListView::onFileChanged,
                      ViewController::get()->getGameListView(file->getSystem()).get(), file, true),
            clearGameBtnFunc, deleteGameBtnFunc));
    }
    else {
        mWindow->pushGui(new GuiMetaDataEd(
            mWindow, &file->metadata, file->metadata.getMDD(GAME_METADATA), p,
            Utils::FileSystem::getFileName(file->getPath()),
            std::bind(&IGameListView::onFileChanged,
                      ViewController::get()->getGameListView(file->getSystem()).get(), file, true),
            clearGameBtnFunc, deleteGameBtnFunc));
    }
}

void GuiGamelistOptions::jumpToLetter()
{
    std::string letter{Utils::String::getFirstCharacter(mJumpToLetterList->getSelected())};

    // Get the gamelist.
    const std::vector<FileData*>& files{
        getGamelist()->getCursor()->getParent()->getChildrenListToDisplay()};

    for (unsigned int i = 0; i < files.size(); ++i) {
        if (mFavoritesSorting && (mFirstLetterIndex.front() == ViewController::FAVORITE_CHAR ||
                                  mFirstLetterIndex.front() == ViewController::FOLDER_CHAR)) {
            if (Utils::String::getFirstCharacter(files.at(i)->getSortName()) == letter &&
                !files.at(i)->getFavorite()) {
                if (!mOnlyHasFolders && mFoldersOnTop && files.at(i)->getType() == FOLDER) {
                    continue;
                }
                else {
                    getGamelist()->setCursor(files.at(i));
                    break;
                }
            }
        }
        else {
            if (Utils::String::getFirstCharacter(files.at(i)->getSortName()) == letter) {
                if (!mOnlyHasFolders && mFoldersOnTop && files.at(i)->getType() == FOLDER) {
                    continue;
                }
                else {
                    getGamelist()->setCursor(files.at(i));
                    break;
                }
            }
        }
    }
}

void GuiGamelistOptions::jumpToFirstRow()
{
    if (mFoldersOnTop && mJumpToLetterList->getSelected() == ViewController::FAVORITE_CHAR) {
        // Get the gamelist.
        const std::vector<FileData*>& files =
            getGamelist()->getCursor()->getParent()->getChildrenListToDisplay();
        // Select the first game that is not a folder, unless it's a folder-only list in
        // which case the first line overall is selected.
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            if (!mOnlyHasFolders && mFoldersOnTop && (*it)->getType() == FOLDER) {
                continue;
            }
            else {
                getGamelist()->setCursor(*it);
                break;
            }
        }
    }
    else {
        // Get first row of the gamelist.
        getGamelist()->setCursor(getGamelist()->getFirstEntry());
    }
}

bool GuiGamelistOptions::input(InputConfig* config, Input input)
{
    if (input.value != 0 && config->isMappedTo("back", input))
        mCancelled = true;

    if (input.value != 0 && (config->isMappedTo("b", input) || config->isMappedTo("back", input))) {
        delete this;
        return true;
    }

    return mMenu.input(config, input);
}

HelpStyle GuiGamelistOptions::getHelpStyle()
{
    HelpStyle style = HelpStyle();
    style.applyTheme(mSystem->getTheme(), "system");
    return style;
}

std::vector<HelpPrompt> GuiGamelistOptions::getHelpPrompts()
{
    auto prompts = mMenu.getHelpPrompts();
    if (mSystem->getRootFolder()->getChildren().size() > 0 || mIsCustomCollectionGroup ||
        mIsCustomCollection || CollectionSystemsManager::get()->isEditing())
        prompts.push_back(HelpPrompt("a", "select"));
    if (mSystem->getRootFolder()->getChildren().size() > 0 && mSystem->getName() != "recent") {
        prompts.push_back(HelpPrompt("b", "close (apply)"));
        prompts.push_back(HelpPrompt("back", "close (cancel)"));
    }
    else {
        prompts.push_back(HelpPrompt("b", "close"));
        prompts.push_back(HelpPrompt("back", "close"));
    }
    return prompts;
}

IGameListView* GuiGamelistOptions::getGamelist()
{
    return ViewController::get()->getGameListView(mSystem).get();
}
