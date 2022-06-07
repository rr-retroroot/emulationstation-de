//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GamelistView.cpp
//
//  Main gamelist logic.
//

#include "views/GamelistView.h"
#include "views/GamelistLegacy.h"

#include "CollectionSystemsManager.h"
#include "UIModeController.h"
#include "animations/LambdaAnimation.h"

#define FADE_IN_START_OPACITY 0.5f
#define FADE_IN_TIME 325

GamelistView::GamelistView(FileData* root)
    : GamelistBase {root}
    , mRenderer {Renderer::getInstance()}
    , mViewStyle {ViewController::BASIC}
    , mLegacyMode {false}
{
    mViewStyle = ViewController::getInstance()->getState().viewstyle;

    if (mLegacyMode)
        return;
}

GamelistView::~GamelistView()
{
    // Remove theme extras.
    for (auto extra : mThemeExtras) {
        removeChild(extra);
        delete extra;
    }
    mThemeExtras.clear();
}

void GamelistView::onFileChanged(FileData* file, bool reloadGamelist)
{
    if (reloadGamelist) {
        // Might switch to a detailed view.
        ViewController::getInstance()->reloadGamelistView(this);
        return;
    }

    // We could be tricky here to be efficient;
    // but this shouldn't happen very often so we'll just always repopulate.
    FileData* cursor {getCursor()};
    if (!cursor->isPlaceHolder()) {
        populateList(cursor->getParent()->getChildrenListToDisplay(), cursor->getParent());
        setCursor(cursor);
    }
    else {
        populateList(mRoot->getChildrenListToDisplay(), mRoot);
        setCursor(cursor);
    }
}

void GamelistView::onShow()
{
    // Reset any GIF and Lottie animations.
    for (auto& animation : mLottieAnimComponents)
        animation->resetFileAnimation();

    for (auto& animation : mGIFAnimComponents)
        animation->resetFileAnimation();

    mLastUpdated = nullptr;
    GuiComponent::onShow();

    if (mLegacyMode)
        legacyUpdateInfoPanel(CursorState::CURSOR_STOPPED);
    else
        updateInfoPanel(CursorState::CURSOR_STOPPED);

    mPrimary->finishAnimation(0);
}

void GamelistView::onTransition()
{
    for (auto& animation : mLottieAnimComponents)
        animation->setPauseAnimation(true);

    for (auto& animation : mGIFAnimComponents)
        animation->setPauseAnimation(true);
}

void GamelistView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
    auto themeSets = ThemeData::getThemeSets();
    std::map<std::string, ThemeData::ThemeSet, ThemeData::StringComparator>::const_iterator
        selectedSet {themeSets.find(Settings::getInstance()->getString("ThemeSet"))};

    assert(selectedSet != themeSets.cend());
    mLegacyMode = selectedSet->second.capabilities.legacyTheme;

    if (mLegacyMode) {
        legacyOnThemeChanged(theme);
        return;
    }

    using namespace ThemeFlags;

    if (mTheme->hasView("gamelist")) {
        for (auto& element : mTheme->getViewElements("gamelist").elements) {
            if (element.second.type == "textlist" || element.second.type == "carousel") {
                if (element.second.type == "carousel" && mTextList != nullptr) {
                    LOG(LogWarning) << "SystemView::populate(): Multiple primary components "
                                    << "defined, skipping <carousel> configuration entry";
                    continue;
                }
                if (element.second.type == "textlist" && mCarousel != nullptr) {
                    LOG(LogWarning) << "SystemView::populate(): Multiple primary components "
                                    << "defined, skipping <textlist> configuration entry";
                    continue;
                }
            }
            if (element.second.type == "textlist") {
                if (mTextList == nullptr) {
                    mTextList = std::make_unique<TextListComponent<FileData*>>();
                    mPrimary = mTextList.get();
                }
                mPrimary->setPosition(0.2f, mSize.y * 0.2f);
                mPrimary->setSize(mSize.x * 0.7f, mSize.y * 0.6f);
                mPrimary->setAlignment(TextListComponent<FileData*>::PrimaryAlignment::ALIGN_LEFT);
                mPrimary->setCursorChangedCallback(
                    [&](const CursorState& state) { updateInfoPanel(state); });
                mPrimary->setDefaultZIndex(50.0f);
                mPrimary->setZIndex(50.0f);
                mPrimary->applyTheme(theme, "gamelist", element.first, ALL);
                addChild(mPrimary);
            }
            if (element.second.type == "carousel") {
                if (mCarousel == nullptr) {
                    mCarousel = std::make_unique<CarouselComponent<FileData*>>();
                    if (element.second.has("itemType")) {
                        const std::string itemType {element.second.get<std::string>("itemType")};
                        if (itemType == "marquee" || itemType == "cover" ||
                            itemType == "backcover" || itemType == "3dbox" ||
                            itemType == "physicalmedia" || itemType == "screenshot" ||
                            itemType == "titlescreen" || itemType == "miximage" ||
                            itemType == "fanart") {
                            mCarousel->setItemType(itemType);
                        }
                        else {
                            LOG(LogWarning)
                                << "GamelistView::onThemeChanged(): Invalid theme configuration, "
                                   "<itemType> property defined as \""
                                << itemType << "\"";
                            mCarousel->setItemType("marquee");
                        }
                    }
                    else {
                        mCarousel->setItemType("marquee");
                    }
                    if (element.second.has("defaultItem"))
                        mCarousel->setDefaultItem(element.second.get<std::string>("defaultItem"));
                    mPrimary = mCarousel.get();
                }
                mPrimary->setCursorChangedCallback(
                    [&](const CursorState& state) { updateInfoPanel(state); });
                mPrimary->setDefaultZIndex(50.0f);
                mPrimary->applyTheme(theme, "gamelist", element.first, ALL);
                addChild(mPrimary);
            }
            if (element.second.type == "image") {
                mImageComponents.push_back(std::make_unique<ImageComponent>());
                mImageComponents.back()->setDefaultZIndex(30.0f);
                mImageComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                if (mImageComponents.back()->getThemeImageTypes().size() != 0)
                    mImageComponents.back()->setScrollHide(true);
                addChild(mImageComponents.back().get());
            }
            else if (element.second.type == "video") {
                mVideoComponents.push_back(std::make_unique<VideoFFmpegComponent>());
                mVideoComponents.back()->setDefaultZIndex(30.0f);
                addChild(mVideoComponents.back().get());
                mVideoComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                if (mVideoComponents.back()->getThemeImageTypes().size() != 0)
                    mVideoComponents.back()->setScrollHide(true);
            }
            else if (element.second.type == "animation" && element.second.has("path")) {
                const std::string extension {
                    Utils::FileSystem::getExtension(element.second.get<std::string>("path"))};
                if (extension == ".json") {
                    mLottieAnimComponents.push_back(std::make_unique<LottieAnimComponent>());
                    mLottieAnimComponents.back()->setDefaultZIndex(35.0f);
                    mLottieAnimComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                    addChild(mLottieAnimComponents.back().get());
                }
                else if (extension == ".gif") {
                    mGIFAnimComponents.push_back(std::make_unique<GIFAnimComponent>());
                    mGIFAnimComponents.back()->setDefaultZIndex(35.0f);
                    mGIFAnimComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                    addChild(mGIFAnimComponents.back().get());
                }
                else if (extension == ".") {
                    LOG(LogWarning)
                        << "GamelistView::onThemeChanged(): Invalid theme configuration, "
                           "animation file extension is missing";
                }
                else {
                    LOG(LogWarning)
                        << "GamelistView::onThemeChanged(): Invalid theme configuration, "
                           "animation file extension defined as \""
                        << extension << "\"";
                }
            }
            else if (element.second.type == "badges") {
                mBadgeComponents.push_back(std::make_unique<BadgeComponent>());
                mBadgeComponents.back()->setDefaultZIndex(35.0f);
                mBadgeComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                mBadgeComponents.back()->setScrollHide(true);
                addChild(mBadgeComponents.back().get());
            }
            else if (element.second.type == "text") {
                if (element.second.has("container") && element.second.get<bool>("container")) {
                    mContainerComponents.push_back(std::make_unique<ScrollableContainer>());
                    mContainerComponents.back()->setDefaultZIndex(40.0f);
                    addChild(mContainerComponents.back().get());
                    mContainerTextComponents.push_back(std::make_unique<TextComponent>());
                    mContainerTextComponents.back()->setDefaultZIndex(40.0f);
                    mContainerComponents.back()->addChild(mContainerTextComponents.back().get());
                    mContainerComponents.back()->applyTheme(theme, "gamelist", element.first,
                                                            POSITION | ThemeFlags::SIZE | Z_INDEX |
                                                                VISIBLE);
                    mContainerComponents.back()->setAutoScroll(true);
                    mContainerTextComponents.back()->setSize(
                        mContainerComponents.back()->getSize().x, 0.0f);
                    mContainerTextComponents.back()->applyTheme(
                        theme, "gamelist", element.first,
                        ALL ^ POSITION ^ Z_INDEX ^ ThemeFlags::SIZE ^ VISIBLE ^ ROTATION);
                    mContainerComponents.back()->setScrollHide(true);
                }
                else {
                    mTextComponents.push_back(std::make_unique<TextComponent>());
                    mTextComponents.back()->setDefaultZIndex(40.0f);
                    mTextComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                    if (mTextComponents.back()->getThemeMetadata() != "")
                        mTextComponents.back()->setScrollHide(true);
                    addChild(mTextComponents.back().get());
                }
            }
            else if (element.second.type == "datetime") {
                mDateTimeComponents.push_back(std::make_unique<DateTimeComponent>());
                mDateTimeComponents.back()->setDefaultZIndex(40.0f);
                mDateTimeComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                if (mDateTimeComponents.back()->getThemeMetadata() != "")
                    mDateTimeComponents.back()->setScrollHide(true);
                addChild(mDateTimeComponents.back().get());
            }
            else if (element.second.type == "gamelistinfo") {
                mGamelistInfoComponents.push_back(std::make_unique<TextComponent>());
                mGamelistInfoComponents.back()->setDefaultZIndex(45.0f);
                mGamelistInfoComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                addChild(mGamelistInfoComponents.back().get());
            }
            else if (element.second.type == "rating") {
                mRatingComponents.push_back(std::make_unique<RatingComponent>());
                mRatingComponents.back()->setDefaultZIndex(45.0f);
                mRatingComponents.back()->applyTheme(theme, "gamelist", element.first, ALL);
                mRatingComponents.back()->setScrollHide(true);
                addChild(mRatingComponents.back().get());
            }
        }

        mHelpStyle.applyTheme(mTheme, "gamelist");
    }

    if (mPrimary == nullptr) {
        mTextList = std::make_unique<TextListComponent<FileData*>>();
        mPrimary = mTextList.get();
        mPrimary->setPosition(0.2f, mSize.y * 0.2f);
        mPrimary->setSize(mSize.x * 0.7f, mSize.y * 0.6f);
        mPrimary->setAlignment(TextListComponent<FileData*>::PrimaryAlignment::ALIGN_LEFT);
        mPrimary->setCursorChangedCallback(
            [&](const CursorState& state) { updateInfoPanel(state); });
        mPrimary->setDefaultZIndex(50.0f);
        mPrimary->setZIndex(50.0f);
        mPrimary->applyTheme(theme, "gamelist", "", ALL);
        addChild(mPrimary);
    }

    populateList(mRoot->getChildrenListToDisplay(), mRoot);

    // Disable quick system select if the primary component uses the left and right buttons.
    if (mCarousel != nullptr) {
        if (mCarousel->getType() == CarouselComponent<FileData*>::CarouselType::HORIZONTAL ||
            mCarousel->getType() == CarouselComponent<FileData*>::CarouselType::HORIZONTAL_WHEEL)
            mLeftRightAvailable = false;
    }

    sortChildren();
}

void GamelistView::update(int deltaTime)
{
    if (mLegacyMode) {
        legacyUpdate(deltaTime);
        return;
    }

    if (ViewController::getInstance()->getGameLaunchTriggered()) {
        for (auto& image : mImageComponents) {
            if (image->isAnimationPlaying(0))
                image->finishAnimation(0);
        }
    }

    updateChildren(deltaTime);
}

void GamelistView::render(const glm::mat4& parentTrans)
{
    glm::mat4 trans {parentTrans * getTransform()};

    float scaleX {trans[0].x};
    float scaleY {trans[1].y};

    glm::ivec2 pos {static_cast<int>(std::round(trans[3].x)),
                    static_cast<int>(std::round(trans[3].y))};
    glm::ivec2 size {static_cast<int>(std::round(mSize.x * scaleX)),
                     static_cast<int>(std::round(mSize.y * scaleY))};

    mRenderer->pushClipRect(pos, size);
    renderChildren(trans);
    mRenderer->popClipRect();
}

std::vector<HelpPrompt> GamelistView::getHelpPrompts()
{
    std::vector<HelpPrompt> prompts;

    if (Settings::getInstance()->getBool("QuickSystemSelect") &&
        SystemData::sSystemVector.size() > 1 && mLeftRightAvailable)
        prompts.push_back(HelpPrompt("left/right", "system"));

    if (mRoot->getSystem()->getThemeFolder() == "custom-collections" && mCursorStack.empty() &&
        ViewController::getInstance()->getState().viewing == ViewController::GAMELIST)
        prompts.push_back(HelpPrompt("a", "enter"));
    else
        prompts.push_back(HelpPrompt("a", "launch"));

    prompts.push_back(HelpPrompt("b", "back"));
    prompts.push_back(HelpPrompt("x", "view media"));

    if (!UIModeController::getInstance()->isUIModeKid())
        prompts.push_back(HelpPrompt("back", "options"));
    if (mRoot->getSystem()->isGameSystem() && Settings::getInstance()->getBool("RandomAddButton"))
        prompts.push_back(HelpPrompt("thumbstickclick", "random"));

    if (mRoot->getSystem()->getThemeFolder() == "custom-collections" &&
        !CollectionSystemsManager::getInstance()->isEditing() && mCursorStack.empty() &&
        ViewController::getInstance()->getState().viewing == ViewController::GAMELIST &&
        ViewController::getInstance()->getState().viewstyle != ViewController::BASIC) {
        prompts.push_back(HelpPrompt("y", "jump to game"));
    }
    else if (mRoot->getSystem()->isGameSystem() &&
             (mRoot->getSystem()->getThemeFolder() != "custom-collections" ||
              !mCursorStack.empty()) &&
             !UIModeController::getInstance()->isUIModeKid() &&
             !UIModeController::getInstance()->isUIModeKiosk() &&
             (Settings::getInstance()->getBool("FavoritesAddButton") ||
              CollectionSystemsManager::getInstance()->isEditing())) {
        std::string prompt = CollectionSystemsManager::getInstance()->getEditingCollection();
        prompts.push_back(HelpPrompt("y", prompt));
    }
    else if (mRoot->getSystem()->isGameSystem() &&
             mRoot->getSystem()->getThemeFolder() == "custom-collections" &&
             CollectionSystemsManager::getInstance()->isEditing()) {
        std::string prompt = CollectionSystemsManager::getInstance()->getEditingCollection();
        prompts.push_back(HelpPrompt("y", prompt));
    }
    return prompts;
}

void GamelistView::updateInfoPanel(const CursorState& state)
{
    if (mLegacyMode) {
        legacyUpdateInfoPanel(state);
        return;
    }

    FileData* file {(mPrimary->size() > 0 && state == CursorState::CURSOR_STOPPED) ?
                        mPrimary->getSelected() :
                        nullptr};

    if (mCarousel != nullptr)
        mCarousel->onDemandTextureLoad();

    // If the game data has already been rendered to the info panel, then skip it this time.
    if (file == mLastUpdated)
        return;

    if (state == CursorState::CURSOR_STOPPED)
        mLastUpdated = file;

    bool hideMetaDataFields {false};

    if (file) {
        // Always hide the metadata fields if browsing grouped custom collections.
        if (file->getSystem()->isCustomCollection() &&
            file->getPath() == file->getSystem()->getName())
            hideMetaDataFields = true;
        else
            hideMetaDataFields = (file->metadata.get("hidemetadata") == "true");

        // Always hide the metadata fields for placeholders as well.
        if (file->getType() == PLACEHOLDER) {
            hideMetaDataFields = true;
            mLastUpdated = nullptr;
        }
    }

    // If we're scrolling, hide the metadata fields if the last game had this options set,
    // or if we're in the grouped custom collection view.
    if (state == CursorState::CURSOR_SCROLLING) {
        if ((mLastUpdated && mLastUpdated->metadata.get("hidemetadata") == "true") ||
            (mLastUpdated->getSystem()->isCustomCollection() &&
             mLastUpdated->getPath() == mLastUpdated->getSystem()->getName()))
            hideMetaDataFields = true;
    }

    if (hideMetaDataFields) {
        for (auto& text : mTextComponents) {
            if (text->getThemeMetadata() != "")
                text->setVisible(false);
        }
        for (auto& date : mDateTimeComponents)
            date->setVisible(false);
        for (auto& badge : mBadgeComponents)
            badge->setVisible(false);
        for (auto& rating : mRatingComponents)
            rating->setVisible(false);
        for (auto& cText : mContainerTextComponents) {
            if (cText->getThemeMetadata() != "description")
                cText->setVisible(false);
        }
    }
    else {
        for (auto& text : mTextComponents) {
            if (text->getThemeMetadata() != "")
                text->setVisible(true);
        }
        for (auto& date : mDateTimeComponents)
            date->setVisible(true);
        for (auto& badge : mBadgeComponents)
            badge->setVisible(true);
        for (auto& rating : mRatingComponents)
            rating->setVisible(true);
        for (auto& cText : mContainerTextComponents) {
            if (cText->getThemeMetadata() != "description")
                cText->setVisible(true);
        }
    }

    bool fadingOut = false;
    if (file == nullptr) {
        if (mVideoPlaying) {
            for (auto& video : mVideoComponents) {
                video->stopVideoPlayer();
                video->setVideo("");
                if (!video->hasStartDelay())
                    video->setImage("");
            }
        }
        mVideoPlaying = false;
        fadingOut = true;
    }
    else {
        // If we're browsing a grouped custom collection, then update the folder metadata
        // which will generate a description of three random games and return a pointer to
        // the first of these so that we can display its game media.
        if (file->getSystem()->isCustomCollection() &&
            file->getPath() == file->getSystem()->getName()) {
            mRandomGame = CollectionSystemsManager::getInstance()->updateCollectionFolderMetadata(
                file->getSystem());
            if (mRandomGame) {
                for (auto& image : mImageComponents)
                    setGameImage(mRandomGame, image.get());

                for (auto& video : mVideoComponents) {
                    setGameImage(mRandomGame, video.get());

                    video->stopVideoPlayer();

                    if (video->hasStaticVideo())
                        video->setStaticVideo();
                    else if (!video->setVideo(mRandomGame->getVideoPath()))
                        video->setDefaultVideo();

                    video->startVideoPlayer();
                }
            }
            else {
                for (auto& image : mImageComponents) {
                    if (image->getThemeImageTypes().size() != 0)
                        image->setImage("");
                }

                for (auto& video : mVideoComponents) {
                    video->stopVideoPlayer();
                    video->setImage("");
                    video->setVideo("");
                    if (video->hasStaticVideo()) {
                        video->setStaticVideo();
                    }
                    else {
                        video->setDefaultVideo();
                    }
                }
            }
        }
        else {
            for (auto& image : mImageComponents) {
                setGameImage(file, image.get());
            }

            for (auto& video : mVideoComponents) {
                setGameImage(file, video.get());
                video->stopVideoPlayer();

                if (video->hasStaticVideo())
                    video->setStaticVideo();
                else if (!video->setVideo(file->getVideoPath()))
                    video->setDefaultVideo();

                video->startVideoPlayer();
            }
        }

        mVideoPlaying = true;

        // Populate the gamelistInfo field which shows an icon if a folder has been entered
        // as well as the game count for the entire system (total and favorites separately).
        // If a filter has been applied, then the number of filtered and total games replaces
        // the game counter.
        for (auto& gamelistInfo : mGamelistInfoComponents) {
            std::string gamelistInfoString;
            Alignment infoAlign = gamelistInfo->getHorizontalAlignment();

            if (mIsFolder && infoAlign == ALIGN_RIGHT)
                gamelistInfoString = ViewController::FOLDER_CHAR + "  ";

            if (mIsFiltered) {
                if (mFilteredGameCountAll == mFilteredGameCount)
                    gamelistInfoString += ViewController::FILTER_CHAR + " " +
                                          std::to_string(mFilteredGameCount) + " / " +
                                          std::to_string(mGameCount);
                else
                    gamelistInfoString +=
                        ViewController::FILTER_CHAR + " " + std::to_string(mFilteredGameCount) +
                        " + " + std::to_string(mFilteredGameCountAll - mFilteredGameCount) + " / " +
                        std::to_string(mGameCount);
            }
            else {
                gamelistInfoString +=
                    ViewController::CONTROLLER_CHAR + " " + std::to_string(mGameCount);
                if (!(file->getSystem()->isCollection() &&
                      file->getSystem()->getFullName() == "favorites"))
                    gamelistInfoString += "  " + ViewController::FAVORITE_CHAR + " " +
                                          std::to_string(mFavoritesGameCount);
            }

            if (mIsFolder && infoAlign != ALIGN_RIGHT)
                gamelistInfoString += "  " + ViewController::FOLDER_CHAR;

            gamelistInfo->setValue(gamelistInfoString);
        }

        // Fade in the game image.
        for (auto& image : mImageComponents) {
            if (image->getScrollFadeIn()) {
                auto func = [&image](float t) {
                    image->setOpacity(glm::mix(FADE_IN_START_OPACITY, 1.0f, t));
                };
                image->setAnimation(new LambdaAnimation(func, FADE_IN_TIME), 0, nullptr, false);
            }
        }

        // Fade in the static image.
        for (auto& video : mVideoComponents) {
            if (video->getScrollFadeIn()) {
                auto func = [&video](float t) {
                    video->setOpacity(glm::mix(FADE_IN_START_OPACITY, 1.0f, t));
                };
                video->setAnimation(new LambdaAnimation(func, FADE_IN_TIME), 0, nullptr, false);
            }
        }

        for (auto& container : mContainerComponents)
            container->reset();

        for (auto& rating : mRatingComponents)
            rating->setValue(file->metadata.get("rating"));

        // Populate the badge slots based on game metadata.
        std::vector<BadgeComponent::BadgeInfo> badgeSlots;
        for (auto& badgeComponent : mBadgeComponents) {
            for (auto& badge : badgeComponent->getBadgeTypes()) {
                BadgeComponent::BadgeInfo badgeInfo;
                badgeInfo.badgeType = badge;
                if (badge == "collection" && CollectionSystemsManager::getInstance()->isEditing()) {
                    if (CollectionSystemsManager::getInstance()->inCustomCollection(
                            CollectionSystemsManager::getInstance()->getEditingCollection(),
                            file)) {
                        badgeSlots.emplace_back(badgeInfo);
                    }
                }
                else if (badge == "folder") {
                    if (file->getType() == FOLDER) {
                        if (file->metadata.get("folderlink") != "")
                            badgeInfo.folderLink = true;
                        badgeSlots.emplace_back(badgeInfo);
                    }
                }
                else if (badge == "controller") {
                    if (file->metadata.get("controller") != "") {
                        badgeInfo.gameController = file->metadata.get("controller");
                        badgeSlots.emplace_back(badgeInfo);
                    }
                }
                else if (badge == "altemulator") {
                    if (file->metadata.get(badge) != "")
                        badgeSlots.emplace_back(badgeInfo);
                }
                else {
                    if (file->metadata.get(badge) == "true")
                        badgeSlots.emplace_back(badgeInfo);
                }
            }
            badgeComponent->setBadges(badgeSlots);
        }

        for (auto& text : mTextComponents) {
            if (text->getThemeMetadata() == "name")
                text->setText(file->metadata.get("name"));
        }

        if (file->getType() == GAME) {
            if (!hideMetaDataFields) {
                for (auto& date : mDateTimeComponents) {
                    if (date->getThemeMetadata() == "lastplayed")
                        date->setValue(file->metadata.get("lastplayed"));
                    else if (date->getThemeMetadata() == "playcount")
                        date->setValue(file->metadata.get("playcount"));
                }
            }
            else if (file->getType() == FOLDER) {
                if (!hideMetaDataFields) {
                    for (auto& date : mDateTimeComponents) {
                        if (date->getThemeMetadata() == "lastplayed") {
                            date->setValue(file->metadata.get("lastplayed"));
                            date->setVisible(false);
                            date->setVisible(false);
                        }
                    }
                }
            }
        }

        std::string metadata;

        auto getMetadataValue = [&file, &metadata]() -> std::string {
            if (metadata == "name")
                return file->metadata.get("name");
            else if (metadata == "description")
                return file->metadata.get("desc");
            else if (metadata == "developer")
                return file->metadata.get("developer");
            else if (metadata == "publisher")
                return file->metadata.get("publisher");
            else if (metadata == "genre")
                return file->metadata.get("genre");
            else if (metadata == "players")
                return file->metadata.get("players");
            else if (metadata == "favorite")
                return file->metadata.get("favorite") == "true" ? "yes" : "no";
            else if (metadata == "completed")
                return file->metadata.get("completed") == "true" ? "yes" : "no";
            else if (metadata == "kidgame")
                return file->metadata.get("kidgame") == "true" ? "yes" : "no";
            else if (metadata == "broken")
                return file->metadata.get("broken") == "true" ? "yes" : "no";
            else if (metadata == "playcount")
                return file->metadata.get("playcount");
            else if (metadata == "altemulator")
                return file->metadata.get("altemulator");
            else
                return metadata;
        };

        for (auto& text : mContainerTextComponents) {
            metadata = text->getThemeMetadata();
            if (metadata == "")
                continue;

            if (metadata == "rating") {
                text->setValue(RatingComponent::getRatingValue(file->metadata.get("rating")));
                continue;
            }
            else if (metadata == "controller") {
                std::string controller {
                    BadgeComponent::getDisplayName(file->metadata.get("controller"))};
                text->setValue(controller == "unknown" ? "" : controller);
                continue;
            }

            text->setValue(getMetadataValue());
        }

        for (auto& text : mTextComponents) {
            metadata = text->getThemeMetadata();
            if (metadata == "")
                continue;

            if (metadata == "rating") {
                text->setValue(RatingComponent::getRatingValue(file->metadata.get("rating")));
                continue;
            }
            else if (metadata == "controller") {
                std::string controller =
                    BadgeComponent::getDisplayName(file->metadata.get("controller"));
                text->setValue(controller == "unknown" ? "" : controller);
                continue;
            }

            text->setValue(getMetadataValue());
        }

        for (auto& date : mDateTimeComponents) {
            std::string metadata = date->getThemeMetadata();
            if (metadata == "")
                continue;

            if (metadata == "releasedate") {
                date->setValue(file->metadata.get("releasedate"));
            }
            else if (metadata == "lastplayed") {
                date->setValue(file->metadata.get("lastplayed"));
            }
            else {
                date->setValue("19700101T000000");
            }
        }
    }

    std::vector<GuiComponent*> comps;

    for (auto& text : mTextComponents) {
        if (text->getScrollHide())
            comps.emplace_back(text.get());
    }
    for (auto& date : mDateTimeComponents) {
        if (date->getScrollHide())
            comps.emplace_back(date.get());
    }
    for (auto& image : mImageComponents) {
        if (image->getScrollHide())
            comps.emplace_back(image.get());
    }
    for (auto& video : mVideoComponents) {
        if (video->getScrollHide())
            comps.emplace_back(video.get());
    }
    for (auto& badge : mBadgeComponents) {
        if (badge->getScrollHide())
            comps.emplace_back(badge.get());
    }
    for (auto& rating : mRatingComponents) {
        if (rating->getScrollHide())
            comps.emplace_back(rating.get());
    }
    for (auto& container : mContainerComponents) {
        if (container->getScrollHide())
            comps.emplace_back(container.get());
    }

    for (auto it = comps.cbegin(); it != comps.cend(); ++it) {
        GuiComponent* comp {*it};
        // An animation is playing, then animate if reverse != fadingOut.
        // An animation is not playing, then animate if opacity != our target opacity.
        if ((comp->isAnimationPlaying(0) && comp->isAnimationReversed(0) != fadingOut) ||
            (!comp->isAnimationPlaying(0) && comp->getOpacity() != (fadingOut ? 0.0f : 1.0f))) {
            auto func = [comp](float t) { comp->setOpacity(glm::mix(0.0f, 1.0f, t)); };
            comp->setAnimation(new LambdaAnimation(func, 150), 0, nullptr, fadingOut);
        }
    }

    if (state == CursorState::CURSOR_SCROLLING)
        mLastUpdated = nullptr;
}

void GamelistView::setGameImage(FileData* file, GuiComponent* comp)
{
    std::string path;
    for (auto& imageType : comp->getThemeImageTypes()) {
        if (imageType == "image") {
            path = file->getImagePath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "miximage") {
            path = file->getMiximagePath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "marquee") {
            path = file->getMarqueePath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "screenshot") {
            path = file->getScreenshotPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "titlescreen") {
            path = file->getTitleScreenPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "cover") {
            path = file->getCoverPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "backcover") {
            path = file->getBackCoverPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "3dbox") {
            path = file->get3DBoxPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "physicalmedia") {
            path = file->getPhysicalMediaPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
        else if (imageType == "fanart") {
            path = file->getFanArtPath();
            if (path != "") {
                comp->setImage(path);
                break;
            }
        }
    }
    // This is needed so the default image is set if no game media was found.
    if (path == "" && comp->getThemeImageTypes().size() > 0)
        comp->setImage("");
}
