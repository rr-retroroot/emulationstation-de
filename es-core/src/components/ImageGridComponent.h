//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  ImageGridComponent.cpp
//
//  X*Y image grid, used by GridGameListView.
//

#ifndef ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
#define ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H

#include "GridTileComponent.h"
#include "Log.h"
#include "animations/LambdaAnimation.h"
#include "components/IList.h"
#include "resources/TextureResource.h"

#define EXTRAITEMS 2

enum ScrollDirection {
    SCROLL_VERTICALLY, // Replace with AllowShortEnumsOnASingleLine: false (clang-format >=11.0).
    SCROLL_HORIZONTALLY
};

enum ImageSource {
    THUMBNAIL, // Replace with AllowShortEnumsOnASingleLine: false (clang-format >=11.0).
    IMAGE,
    MIXIMAGE,
    SCREENSHOT,
    COVER,
    MARQUEE,
    BOX3D
};

struct ImageGridData {
    std::string texturePath;
};

template <typename T> class ImageGridComponent : public IList<ImageGridData, T>
{
protected:
    using IList<ImageGridData, T>::mEntries;
    using IList<ImageGridData, T>::mScrollTier;
    using IList<ImageGridData, T>::listUpdate;
    using IList<ImageGridData, T>::listInput;
    using IList<ImageGridData, T>::listRenderTitleOverlay;
    using IList<ImageGridData, T>::getTransform;
    using IList<ImageGridData, T>::mSize;
    using IList<ImageGridData, T>::mCursor;
    using IList<ImageGridData, T>::mWindow;
    using IList<ImageGridData, T>::IList;

public:
    using IList<ImageGridData, T>::size;
    using IList<ImageGridData, T>::isScrolling;
    using IList<ImageGridData, T>::stopScrolling;

    ImageGridComponent(Window* window);

    void add(const std::string& name, const std::string& imagePath, const T& obj);

    bool input(InputConfig* config, Input input) override;
    void update(int deltaTime) override;
    void render(const glm::mat4& parentTrans) override;
    virtual void applyTheme(const std::shared_ptr<ThemeData>& theme,
                            const std::string& view,
                            const std::string& element,
                            unsigned int properties) override;

    void onSizeChanged() override;
    void setCursorChangedCallback(const std::function<void(CursorState state)>& func)
    {
        mCursorChangedCallback = func;
    }

    ImageSource getImageSource() { return mImageSource; }

protected:
    virtual void onCursorChanged(const CursorState& state) override;

private:
    // Tiles.
    void buildTiles();
    void updateTiles(bool ascending = true,
                     bool allowAnimation = true,
                     bool updateSelectedState = true);
    void updateTileAtPos(int tilePos, int imgPos, bool allowAnimation, bool updateSelectedState);
    void calcGridDimension();
    bool isScrollLoop();

    bool isVertical() { return mScrollDirection == SCROLL_VERTICALLY; }

    // Images and entries.
    bool mEntriesDirty;
    int mLastCursor;
    std::string mDefaultGameTexture;
    std::string mDefaultFolderTexture;

    // Tiles.
    bool mLastRowPartial;
    glm::vec2 mAutoLayout;
    float mAutoLayoutZoom;

    glm::vec4 mPadding;
    glm::vec2 mMargin;
    glm::vec2 mTileSize;
    Vector2i mGridDimension;
    std::shared_ptr<ThemeData> mTheme;
    std::vector<std::shared_ptr<GridTileComponent>> mTiles;

    int mStartPosition;

    float mCamera;
    float mCameraDirection;

    // Miscellaneous.
    bool mAnimate;
    bool mCenterSelection;
    bool mScrollLoop;
    ScrollDirection mScrollDirection;
    ImageSource mImageSource;
    std::function<void(CursorState state)> mCursorChangedCallback;
};

template <typename T>
ImageGridComponent<T>::ImageGridComponent(Window* window)
    : IList<ImageGridData, T>(window)
{
    glm::vec2 screen(static_cast<float>(Renderer::getScreenWidth()),
                     static_cast<float>(Renderer::getScreenHeight()));

    mCamera = 0.0f;
    mCameraDirection = 1.0f;

    mAutoLayout = glm::vec2({});
    mAutoLayoutZoom = 1.0f;

    mStartPosition = 0;

    mEntriesDirty = true;
    mLastCursor = 0;
    mDefaultGameTexture = ":/graphics/cartridge.svg";
    mDefaultFolderTexture = ":/graphics/folder.svg";

    mSize = screen * 0.80f;
    mMargin = screen * 0.07f;
    mPadding = {};
    mTileSize = GridTileComponent::getDefaultTileSize();

    mAnimate = true;
    mCenterSelection = false;
    mScrollLoop = false;
    mScrollDirection = SCROLL_VERTICALLY;
    mImageSource = THUMBNAIL;
}

template <typename T>
void ImageGridComponent<T>::add(const std::string& name, const std::string& imagePath, const T& obj)
{
    typename IList<ImageGridData, T>::Entry entry;
    entry.name = name;
    entry.object = obj;
    entry.data.texturePath = imagePath;

    static_cast<IList<ImageGridData, T>*>(this)->add(entry);
    mEntriesDirty = true;
}

template <typename T> bool ImageGridComponent<T>::input(InputConfig* config, Input input)
{
    if (input.value != 0) {
        int idx = isVertical() ? 0 : 1;

        Vector2i dir = Vector2i::Zero();
        if (config->isMappedLike("up", input))
            dir[1 ^ idx] = -1;
        else if (config->isMappedLike("down", input))
            dir[1 ^ idx] = 1;
        else if (config->isMappedLike("left", input))
            dir[0 ^ idx] = -1;
        else if (config->isMappedLike("right", input))
            dir[0 ^ idx] = 1;

        if (dir != Vector2i::Zero()) {
            if (isVertical())
                listInput(dir.x() + dir.y() * mGridDimension.x());
            else
                listInput(dir.x() + dir.y() * mGridDimension.y());
            return true;
        }
    }
    else {
        if (config->isMappedLike("up", input) || config->isMappedLike("down", input) ||
            config->isMappedLike("left", input) || config->isMappedLike("right", input)) {
            stopScrolling();
        }
    }

    return GuiComponent::input(config, input);
}

template <typename T> void ImageGridComponent<T>::update(int deltaTime)
{
    GuiComponent::update(deltaTime);
    listUpdate(deltaTime);

    for (auto it = mTiles.begin(); it != mTiles.end(); it++)
        (*it)->update(deltaTime);
}

template <typename T> void ImageGridComponent<T>::render(const glm::mat4& parentTrans)
{
    glm::mat4 trans = getTransform() * parentTrans;
    glm::mat4 tileTrans = trans;

    float offsetX = isVertical() ? 0.0f : mCamera * mCameraDirection * (mTileSize.x + mMargin.x);
    float offsetY = isVertical() ? mCamera * mCameraDirection * (mTileSize.y + mMargin.y) : 0.0f;

    tileTrans = glm::translate(tileTrans, glm::vec3(offsetX, offsetY, 0.0f));

    if (mEntriesDirty) {
        updateTiles();
        mEntriesDirty = false;
    }

    // Create a clipRect to hide tiles used to buffer texture loading.
    float scaleX = trans[0].x;
    float scaleY = trans[1].y;

    Vector2i pos(static_cast<int>(std::round(trans[3].x)),
                 static_cast<int>(std::round(trans[3].y)));
    Vector2i size(static_cast<int>(std::round(mSize.x * scaleX)),
                  static_cast<int>(std::round(mSize.y * scaleY)));

    Renderer::pushClipRect(pos, size);

    // Render all the tiles but the selected one.
    std::shared_ptr<GridTileComponent> selectedTile = nullptr;
    for (auto it = mTiles.begin(); it != mTiles.end(); it++) {
        std::shared_ptr<GridTileComponent> tile = (*it);
        // If it's the selected image, keep it for later, otherwise render it now.
        if (tile->isSelected())
            selectedTile = tile;
        else
            tile->render(tileTrans);
    }

    Renderer::popClipRect();

    // Render the selected image on top of the others.
    if (selectedTile != nullptr)
        selectedTile->render(tileTrans);

    listRenderTitleOverlay(trans);

    GuiComponent::renderChildren(trans);
}

template <typename T>
void ImageGridComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                       const std::string& view,
                                       const std::string& element,
                                       unsigned int properties)
{
    // Apply theme to GuiComponent but not the size property, which will be applied
    // at the end of this function.
    GuiComponent::applyTheme(theme, view, element, properties ^ ThemeFlags::SIZE);

    // Keep the theme pointer to apply it on the tiles later on.
    mTheme = theme;

    glm::vec2 screen(static_cast<float>(Renderer::getScreenWidth()),
                     static_cast<float>(Renderer::getScreenHeight()));

    const ThemeData::ThemeElement* elem = theme->getElement(view, element, "imagegrid");
    if (elem) {
        if (elem->has("margin"))
            mMargin = elem->get<glm::vec2>("margin") * screen;

        if (elem->has("padding"))
            mPadding =
                elem->get<glm::vec4>("padding") * glm::vec4(screen.x, screen.y, screen.x, screen.y);

        if (elem->has("autoLayout"))
            mAutoLayout = elem->get<glm::vec2>("autoLayout");

        if (elem->has("autoLayoutSelectedZoom"))
            mAutoLayoutZoom = elem->get<float>("autoLayoutSelectedZoom");

        if (elem->has("imageSource")) {
            auto direction = elem->get<std::string>("imageSource");
            if (direction == "image")
                mImageSource = IMAGE;
            else if (direction == "miximage")
                mImageSource = MIXIMAGE;
            else if (direction == "screenshot")
                mImageSource = SCREENSHOT;
            else if (direction == "cover")
                mImageSource = COVER;
            else if (direction == "marquee")
                mImageSource = MARQUEE;
            else if (direction == "3dbox")
                mImageSource = BOX3D;
            else
                mImageSource = THUMBNAIL;
        }
        else {
            mImageSource = THUMBNAIL;
        }

        if (elem->has("scrollDirection"))
            mScrollDirection =
                (ScrollDirection)(elem->get<std::string>("scrollDirection") == "horizontal");

        if (elem->has("centerSelection")) {
            mCenterSelection = (elem->get<bool>("centerSelection"));

            if (elem->has("scrollLoop"))
                mScrollLoop = (elem->get<bool>("scrollLoop"));
        }

        if (elem->has("animate"))
            mAnimate = (elem->get<bool>("animate"));
        else
            mAnimate = true;

        if (elem->has("gameImage")) {
            std::string path = elem->get<std::string>("gameImage");

            if (!ResourceManager::getInstance()->fileExists(path)) {
                LOG(LogWarning) << "Could not replace default game image, check path: " << path;
            }
            else {
                std::string oldDefaultGameTexture = mDefaultGameTexture;
                mDefaultGameTexture = path;

                // mEntries are already loaded at this point, so we need to update them with
                // the new game image texture.
                for (auto it = mEntries.begin(); it != mEntries.end(); it++) {
                    if ((*it).data.texturePath == oldDefaultGameTexture)
                        (*it).data.texturePath = mDefaultGameTexture;
                }
            }
        }

        if (elem->has("folderImage")) {
            std::string path = elem->get<std::string>("folderImage");

            if (!ResourceManager::getInstance()->fileExists(path)) {
                LOG(LogWarning) << "Could not replace default folder image, check path: " << path;
            }
            else {
                std::string oldDefaultFolderTexture = mDefaultFolderTexture;
                mDefaultFolderTexture = path;

                // mEntries are already loaded at this point, so we need to update them with
                // the new folder image texture.
                for (auto it = mEntries.begin(); it != mEntries.end(); it++) {
                    if ((*it).data.texturePath == oldDefaultFolderTexture)
                        (*it).data.texturePath = mDefaultFolderTexture;
                }
            }
        }
    }

    // We still need to manually get the grid tile size here, so we can recalculate the new
    // grid dimension, and then (re)build the tiles.
    elem = theme->getElement(view, "default", "gridtile");

    mTileSize = elem && elem->has("size") ? elem->get<glm::vec2>("size") * screen :
                                            GridTileComponent::getDefaultTileSize();

    // Apply size property which will trigger a call to onSizeChanged() which will build the tiles.
    GuiComponent::applyTheme(theme, view, element, ThemeFlags::SIZE);

    // Trigger the call manually if the theme has no "imagegrid" element.
    if (!elem)
        buildTiles();
}

template <typename T> void ImageGridComponent<T>::onSizeChanged()
{
    buildTiles();
    updateTiles();
}

template <typename T> void ImageGridComponent<T>::onCursorChanged(const CursorState& state)
{
    if (mLastCursor == mCursor) {
        if (state == CURSOR_STOPPED && mCursorChangedCallback)
            mCursorChangedCallback(state);
        return;
    }

    bool direction = mCursor >= mLastCursor;
    int diff = direction ? mCursor - mLastCursor : mLastCursor - mCursor;
    if (isScrollLoop() && diff == mEntries.size() - 1)
        direction = !direction;

    int oldStart = mStartPosition;

    int dimScrollable = (isVertical() ? mGridDimension.y() : mGridDimension.x()) - 2 * EXTRAITEMS;
    int dimOpposite = isVertical() ? mGridDimension.x() : mGridDimension.y();

    int centralCol = static_cast<int>((static_cast<float>(dimScrollable) - 0.5f) / 2.0f);
    int maxCentralCol = dimScrollable / 2;

    int oldCol = (mLastCursor / dimOpposite);
    int col = (mCursor / dimOpposite);

    int lastCol = static_cast<int>((mEntries.size() - 1) / dimOpposite);

    int lastScroll = std::max(0, (lastCol + 1 - dimScrollable));

    float startPos = 0;
    float endPos = 1;

    if (reinterpret_cast<GuiComponent*>(this)->isAnimationPlaying(2)) {
        startPos = 0;
        reinterpret_cast<GuiComponent*>(this)->cancelAnimation(2);
        updateTiles(direction, false, false);
    }

    if (mAnimate) {
        std::shared_ptr<GridTileComponent> oldTile = nullptr;
        std::shared_ptr<GridTileComponent> newTile = nullptr;

        int oldIdx = mLastCursor - mStartPosition + (dimOpposite * EXTRAITEMS);
        if (oldIdx >= 0 && oldIdx < mTiles.size())
            oldTile = mTiles[oldIdx];

        int newIdx = mCursor - mStartPosition + (dimOpposite * EXTRAITEMS);
        if (isScrollLoop()) {
            if (newIdx < 0)
                newIdx += static_cast<int>(mEntries.size());
            else if (newIdx >= mTiles.size())
                newIdx -= static_cast<int>(mEntries.size());
        }

        if (newIdx >= 0 && newIdx < mTiles.size())
            newTile = mTiles[newIdx];

        for (auto it = mTiles.begin(); it != mTiles.end(); it++) {
            if ((*it)->isSelected() && *it != oldTile && *it != newTile) {
                startPos = 0;
                (*it)->setSelected(false, false, nullptr);
            }
        }

        glm::vec3 oldPos {};

        if (oldTile != nullptr && oldTile != newTile) {
            oldPos = oldTile->getBackgroundPosition();
            oldTile->setSelected(false, true, nullptr, true);
        }

        if (newTile != nullptr)
            newTile->setSelected(true, true, oldPos == glm::vec3({}) ? nullptr : &oldPos, true);
    }

    int firstVisibleCol = mStartPosition / dimOpposite;

    if ((col < centralCol || (col == 0 && col == centralCol)) && !mCenterSelection) {
        mStartPosition = 0;
    }
    else if ((col - centralCol) > lastScroll && !mCenterSelection && !isScrollLoop()) {
        mStartPosition = lastScroll * dimOpposite;
    }
    else if ((maxCentralCol != centralCol && col == firstVisibleCol + maxCentralCol) ||
             col == firstVisibleCol + centralCol) {
        if (col == firstVisibleCol + maxCentralCol)
            mStartPosition = (col - maxCentralCol) * dimOpposite;
        else
            mStartPosition = (col - centralCol) * dimOpposite;
    }
    else {
        if (oldCol == firstVisibleCol + maxCentralCol)
            mStartPosition = (col - maxCentralCol) * dimOpposite;
        else
            mStartPosition = (col - centralCol) * dimOpposite;
    }

    auto lastCursor = mLastCursor;
    mLastCursor = mCursor;

    mCameraDirection = direction ? -1.0f : 1.0f;
    mCamera = 0;

    if (lastCursor < 0 || !mAnimate) {
        updateTiles(direction, mAnimate && (lastCursor >= 0 || isScrollLoop()));

        if (mCursorChangedCallback)
            mCursorChangedCallback(state);

        return;
    }

    if (mCursorChangedCallback)
        mCursorChangedCallback(state);

    bool moveCamera = (oldStart != mStartPosition);

    auto func = [this, startPos, endPos, moveCamera](float t) {
        if (!moveCamera)
            return;

        t -= 1.0f;
        float pct = Math::lerp(0, 1.0f, t * t * t + 1.0f);
        t = startPos * (1.0f - pct) + endPos * pct;
        mCamera = t;
    };

    reinterpret_cast<GuiComponent*>(this)->setAnimation(
        new LambdaAnimation(func, 250), 0,
        [this, direction] {
            mCamera = 0;
            updateTiles(direction, false);
        },
        false, 2);
}

// Create and position tiles (mTiles).
template <typename T> void ImageGridComponent<T>::buildTiles()
{
    mStartPosition = 0;
    mTiles.clear();

    calcGridDimension();

    if (mCenterSelection) {
        int dimScrollable =
            (isVertical() ? mGridDimension.y() : mGridDimension.x()) - 2 * EXTRAITEMS;
        mStartPosition -= static_cast<int>(floorf(dimScrollable / 2.0f));
    }

    glm::vec2 tileDistance = mTileSize + mMargin;

    if (mAutoLayout.x != 0.0f && mAutoLayout.y != 0.0f) {
        auto x = (mSize.x - (mMargin.x * (mAutoLayout.x - 1.0f)) - mPadding.x - mPadding.z) /
                 static_cast<int>(mAutoLayout.x);
        auto y = (mSize.y - (mMargin.y * (mAutoLayout.y - 1.0f)) - mPadding.y - mPadding.w) /
                 static_cast<int>(mAutoLayout.y);

        mTileSize = glm::vec2(x, y);
        tileDistance = mTileSize + mMargin;
    }

    bool vert = isVertical();
    glm::vec2 startPosition = mTileSize / 2.0f;
    startPosition.x += mPadding.x;
    startPosition.y += mPadding.y;

    int X;
    int Y;

    // Layout tile size and position.
    for (int y = 0; y < (vert ? mGridDimension.y() : mGridDimension.x()); y++) {
        for (int x = 0; x < (vert ? mGridDimension.x() : mGridDimension.y()); x++) {
            // Create tiles.
            auto tile = std::make_shared<GridTileComponent>(mWindow);

            // In Vertical mode, tiles are ordered from left to right, then from top to bottom.
            // In Horizontal mode, tiles are ordered from top to bottom, then from left to right.
            X = vert ? x : y - EXTRAITEMS;
            Y = vert ? y - EXTRAITEMS : x;

            tile->setPosition(X * tileDistance.x + startPosition.x,
                              Y * tileDistance.y + startPosition.y);
            tile->setOrigin(0.5f, 0.5f);
            tile->setImage("");

            if (mTheme)
                tile->applyTheme(mTheme, "grid", "gridtile", ThemeFlags::ALL);

            if (mAutoLayout.x != 0 && mAutoLayout.y != 0.0f)
                tile->forceSize(mTileSize, mAutoLayoutZoom);

            mTiles.push_back(tile);
        }
    }
}

template <typename T>
void ImageGridComponent<T>::updateTiles(bool ascending,
                                        bool allowAnimation,
                                        bool updateSelectedState)
{
    if (!mTiles.size())
        return;

    // Stop updating the tiles at highest scroll speed.
    if (mScrollTier == 3) {
        for (int ti = 0; ti < static_cast<int>(mTiles.size()); ti++) {
            std::shared_ptr<GridTileComponent> tile = mTiles.at(ti);

            tile->setSelected(false);
            tile->setImage(mDefaultGameTexture);
            tile->setVisible(false);
        }
        return;
    }

    // Temporarily store the previous textures so that they can't be unloaded.
    std::vector<std::shared_ptr<TextureResource>> previousTextures;
    for (int ti = 0; ti < static_cast<int>(mTiles.size()); ti++) {
        std::shared_ptr<GridTileComponent> tile = mTiles.at(ti);
        previousTextures.push_back(tile->getTexture());
    }

    // If going down, update from top to bottom.
    // If going up, update from bottom to top.
    int scrollDirection = ascending ? 1 : -1;
    int ti = ascending ? 0 : static_cast<int>(mTiles.size()) - 1;
    int end = ascending ? static_cast<int>(mTiles.size()) : -1;
    int img = mStartPosition + ti;

    img -= EXTRAITEMS * (isVertical() ? mGridDimension.x() : mGridDimension.y());

    // Update the tiles.
    while (ti != end) {
        updateTileAtPos(ti, img, allowAnimation, updateSelectedState);

        ti += scrollDirection;
        img += scrollDirection;
    }

    if (updateSelectedState)
        mLastCursor = mCursor;

    mLastCursor = mCursor;
}

template <typename T>
void ImageGridComponent<T>::updateTileAtPos(int tilePos,
                                            int imgPos,
                                            bool allowAnimation,
                                            bool updateSelectedState)
{
    std::shared_ptr<GridTileComponent> tile = mTiles.at(tilePos);

    if (isScrollLoop()) {
        if (imgPos < 0)
            imgPos += static_cast<int>(mEntries.size());
        else if (imgPos >= size())
            imgPos -= static_cast<int>(mEntries.size());
    }

    // If we have more tiles than we can display on screen, hide them.
    // Same for tiles out of the buffer.
    if (imgPos < 0 || imgPos >= size() || tilePos < 0 ||
        tilePos >= static_cast<int>(mTiles.size())) {
        if (updateSelectedState)
            tile->setSelected(false, allowAnimation);
        tile->reset();
        tile->setVisible(false);
    }
    else {
        tile->setVisible(true);

        std::string imagePath = mEntries.at(imgPos).data.texturePath;

        if (ResourceManager::getInstance()->fileExists(imagePath))
            tile->setImage(imagePath);
        else if (mEntries.at(imgPos).object->getType() == 2)
            tile->setImage(mDefaultFolderTexture);
        else
            tile->setImage(mDefaultGameTexture);

        if (updateSelectedState) {
            if (imgPos == mCursor && mCursor != mLastCursor) {
                int dif = mCursor - tilePos;
                int idx = mLastCursor - dif;

                if (idx < 0 || idx >= mTiles.size())
                    idx = 0;

                glm::vec3 pos = mTiles.at(idx)->getBackgroundPosition();
                tile->setSelected(true, allowAnimation, &pos);
            }
            else {
                tile->setSelected(imgPos == mCursor, allowAnimation);
            }
        }
    }
}

// Calculate how many tiles of size mTileSize we can fit in a grid of size mSize using
// a margin size of mMargin.
template <typename T> void ImageGridComponent<T>::calcGridDimension()
{
    // grid_size = columns * tile_size + (columns - 1) * margin
    // <=> columns = (grid_size + margin) / (tile_size + margin)
    glm::vec2 gridDimension = (mSize + mMargin) / (mTileSize + mMargin);

    if (mAutoLayout.x != 0.0f && mAutoLayout.y != 0.0f)
        gridDimension = mAutoLayout;

    mLastRowPartial = floorf(gridDimension.y) != gridDimension.y;

    // Ceil y dim so we can display partial last row.
    mGridDimension = Vector2i(static_cast<const int>(gridDimension.x),
                              static_cast<const int>(ceilf(gridDimension.y)));

    // Grid dimension validation.
    if (mGridDimension.x() < 1) {
        LOG(LogError) << "Theme defined grid X dimension below 1";
    }
    if (mGridDimension.y() < 1) {
        LOG(LogError) << "Theme defined grid Y dimension below 1";
    }

    // Add extra tiles to both sides.
    if (isVertical())
        mGridDimension.y() += 2 * EXTRAITEMS;
    else
        mGridDimension.x() += 2 * EXTRAITEMS;
}

template <typename T> bool ImageGridComponent<T>::isScrollLoop()
{
    if (!mScrollLoop)
        return false;
    if (isVertical())
        return (mGridDimension.x() * (mGridDimension.y() - 2 * EXTRAITEMS)) <= mEntries.size();
    return (mGridDimension.y() * (mGridDimension.x() - 2 * EXTRAITEMS)) <= mEntries.size();
}

#endif // ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
