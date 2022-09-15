//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  ImageComponent.cpp
//
//  Handles images: loading, resizing, cropping, color shifting etc.
//

#include "components/ImageComponent.h"

#include "Log.h"
#include "Settings.h"
#include "ThemeData.h"
#include "Window.h"
#include "resources/TextureResource.h"
#include "utils/CImgUtil.h"
#include "utils/StringUtil.h"

ImageComponent::ImageComponent(bool forceLoad, bool dynamic)
    : mRenderer {Renderer::getInstance()}
    , mTargetSize {0, 0}
    , mFlipX {false}
    , mFlipY {false}
    , mTargetIsMax {false}
    , mScalableNonAspect {false}
    , mTileWidth {0.0f}
    , mTileHeight {0.0f}
    , mColorShift {0xFFFFFFFF}
    , mColorShiftEnd {0xFFFFFFFF}
    , mColorGradientHorizontal {true}
    , mFadeOpacity {0.0f}
    , mReflectionsFalloff {0.0f}
    , mFading {false}
    , mForceLoad {forceLoad}
    , mDynamic {dynamic}
    , mRotateByTargetSize {false}
    , mLinearInterpolation {false}
    , mMipmapping {false}
    , mTileHorizontalAlignment {ALIGN_LEFT}
    , mTileVerticalAlignment {ALIGN_BOTTOM}
    , mTopLeftCrop {0.0f, 0.0f}
    , mBottomRightCrop {1.0f, 1.0f}
    , mClipRegion {0.0f, 0.0f, 0.0f, 0.0f}
{
    updateColors();
}

void ImageComponent::setImage(const std::string& path, bool tile)
{
    // Always load bundled graphic resources statically, unless mForceLoad has been set.
    // This eliminates annoying texture pop-in problems that would otherwise occur.
    if (!mForceLoad && (path[0] == ':') && (path[1] == '/')) {
        mDynamic = false;
    }
    else if (Utils::FileSystem::isDirectory(path)) {
        LOG(LogError) << "ImageComponent: Path is a directory and not a file: \"" << path << "\"";
        return;
    }

    const bool isScalable {
        (path != "" && path.length() > 4) ?
            Utils::String::toLower(path.substr(path.size() - 4, std::string::npos)) == ".svg" :
            false};

    // Create an initial blank texture if needed.
    if (path.empty() || !ResourceManager::getInstance().fileExists(path)) {
        if (mDefaultPath.empty() || !ResourceManager::getInstance().fileExists(mDefaultPath))
            mTexture.reset();
        else
            mTexture = TextureResource::get(mDefaultPath, tile, mForceLoad, mDynamic,
                                            mLinearInterpolation, mMipmapping);
        resize(true);
    }
    else {
        // For raster images we just load and resize but for SVG images we first need to resize
        // without rasterizing in order to calculate the correct image size. Then we delete and
        // reload the texture at the requested size in order to add a valid cache entry. Finally
        // we perform the actual rasterization to have the cache entry updated with the proper
        // texture. For SVG images this requires that every call to setImage is made only after
        // a call to setResize or setMaxSize (so the requested size is known upfront).
        if (isScalable) {
            mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic, mLinearInterpolation,
                                            mMipmapping, 0, 0, 0.0f, 0.0f);
            if (tile && (mTileWidth == 0.0f || mTileHeight == 0.0f))
                setTileAxes();
            resize(false);
            mTexture.reset();
            mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic, mLinearInterpolation,
                                            mMipmapping, static_cast<size_t>(mSize.x),
                                            static_cast<size_t>(mSize.y), mTileWidth, mTileHeight);
            mTexture->setScalableNonAspect(mScalableNonAspect);
            mTexture->rasterizeAt(mSize.x, mSize.y);
            onSizeChanged();
        }
        else {
            mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic, mLinearInterpolation,
                                            mMipmapping, 0, 0, mTileWidth, mTileHeight);
            if (tile && (mTileWidth == 0.0f || mTileHeight == 0.0f))
                setTileAxes();
            resize(true);
        }
    }
}

void ImageComponent::setImage(const char* data, size_t length, bool tile)
{
    mTexture.reset();

    mTexture = TextureResource::get("", tile);
    mTexture->initFromMemory(data, length);

    resize();
}

void ImageComponent::setImage(const std::shared_ptr<TextureResource>& texture, bool resizeTexture)
{
    mTexture = texture;
    if (resizeTexture)
        resize();
}

void ImageComponent::setResize(float width, float height)
{
    mTargetSize = glm::vec2 {width, height};
    mTargetIsMax = false;
    resize();
}

void ImageComponent::setResize(float width, float height, bool rasterize)
{
    mTargetSize = glm::vec2 {width, height};
    mTargetIsMax = false;
    resize(rasterize);
}

void ImageComponent::setMaxSize(const float width, const float height)
{
    mTargetSize = glm::vec2 {width, height};
    mTargetIsMax = true;
    resize();
}

void ImageComponent::cropLeft(const float percent)
{
    assert(percent >= 0.0f && percent <= 1.0f);
    mTopLeftCrop.x = percent;
}

void ImageComponent::cropTop(const float percent)
{
    assert(percent >= 0.0f && percent <= 1.0f);
    mTopLeftCrop.y = percent;
}

void ImageComponent::cropRight(const float percent)
{
    assert(percent >= 0.0f && percent <= 1.0f);
    mBottomRightCrop.x = 1.0f - percent;
}

void ImageComponent::cropBot(const float percent)
{
    assert(percent >= 0.0f && percent <= 1.0f);
    mBottomRightCrop.y = 1.0f - percent;
}

void ImageComponent::crop(const float left, const float top, const float right, const float bot)
{
    cropLeft(left);
    cropTop(top);
    cropRight(right);
    cropBot(bot);
}

void ImageComponent::uncrop()
{
    // Remove any applied crop.
    crop(0.0f, 0.0f, 0.0f, 0.0f);
}

void ImageComponent::cropTransparentPadding(const float maxSizeX, const float maxSizeY)
{
    if (mSize == glm::vec2 {0.0f, 0.0f})
        return;

    std::vector<unsigned char> imageRGBA {mTexture.get()->getRawRGBAData()};

    if (imageRGBA.size() == 0)
        return;

    glm::ivec2 imageSize {mTexture.get()->getSize()};
    cimg_library::CImg<unsigned char> imageCImg(imageSize.x, imageSize.y, 1, 4, 0);

    int paddingCoords[4] {0, 0, 0, 0};

    // We need to convert our RGBA data to the CImg internal format as CImg does not interleave
    // the pixels (as in RGBARGBARGBA).
    Utils::CImg::convertRGBAToCImg(imageRGBA, imageCImg);

    // This will give us the coordinates for the fully transparent areas.
    Utils::CImg::getTransparentPaddingCoords(imageCImg, paddingCoords);

    glm::vec2 originalSize {mSize};

    float cropLeft {static_cast<float>(paddingCoords[0]) / static_cast<float>(imageSize.x)};
    float cropTop {static_cast<float>(paddingCoords[1]) / static_cast<float>(imageSize.y)};
    float cropRight {static_cast<float>(paddingCoords[2]) / static_cast<float>(imageSize.x)};
    float cropBottom {static_cast<float>(paddingCoords[3]) / static_cast<float>(imageSize.y)};

    crop(cropLeft, cropTop, cropRight, cropBottom);

    // Cropping the image obviously leads to a reduction in size, so we need to determine
    // how much to scale up after cropping to keep within the max size restrictions that
    // were passed as arguments.
    mSize.x -= mSize.x * (cropLeft + cropRight);
    mSize.y -= mSize.y * (cropTop + cropBottom);

    float scaleFactor {originalSize.y / mSize.y};

    if (scaleFactor * mSize.x < maxSizeX)
        scaleFactor = maxSizeX / mSize.x;

    if (scaleFactor * mSize.y < maxSizeY)
        scaleFactor = maxSizeY / mSize.y;

    if (scaleFactor * mSize.x > maxSizeX)
        scaleFactor = maxSizeX / mSize.x;

    if (scaleFactor * mSize.y > maxSizeY)
        scaleFactor = maxSizeY / mSize.y;

    setResize(mSize.x * scaleFactor, mSize.y * scaleFactor);
    updateVertices();
}

void ImageComponent::setColorShift(unsigned int color)
{
    mColorShift = color;
    mColorShiftEnd = color;
    updateColors();
}

void ImageComponent::setColorShiftEnd(unsigned int color)
{
    mColorShiftEnd = color;
    updateColors();
}

void ImageComponent::setColorGradientHorizontal(bool horizontal)
{
    mColorGradientHorizontal = horizontal;
    updateColors();
}

void ImageComponent::setOpacity(float opacity)
{
    mOpacity = opacity;
    updateColors();
}

void ImageComponent::setSaturation(float saturation)
{
    mSaturation = saturation;
    updateColors();
}

void ImageComponent::setDimming(float dimming)
{
    // Set dimming value.
    mDimming = dimming;
}

void ImageComponent::setClipRegion(const glm::vec4& clipRegion)
{
    if (mVertices[0].clipregion == clipRegion)
        return;

    mClipRegion = clipRegion;

    if (mClipRegion == glm::vec4 {0.0f, 0.0f, 0.0f, 0.0f}) {
        if (mVertices[0].shaderFlags & Renderer::ShaderFlags::CLIPPING) {
            mVertices[0].shaderFlags ^= Renderer::ShaderFlags::CLIPPING;
            mVertices[1].shaderFlags ^= Renderer::ShaderFlags::CLIPPING;
            mVertices[2].shaderFlags ^= Renderer::ShaderFlags::CLIPPING;
            mVertices[3].shaderFlags ^= Renderer::ShaderFlags::CLIPPING;
        }
    }
    else {
        mVertices[0].shaderFlags |= Renderer::ShaderFlags::CLIPPING;
        mVertices[1].shaderFlags |= Renderer::ShaderFlags::CLIPPING;
        mVertices[2].shaderFlags |= Renderer::ShaderFlags::CLIPPING;
        mVertices[3].shaderFlags |= Renderer::ShaderFlags::CLIPPING;
    }

    mVertices[0].clipregion = clipRegion;
    mVertices[1].clipregion = clipRegion;
    mVertices[2].clipregion = clipRegion;
    mVertices[3].clipregion = clipRegion;
}

void ImageComponent::setFlipX(bool state)
{
    mFlipX = state;
    updateVertices();
}

void ImageComponent::setFlipY(bool state)
{
    mFlipY = state;
    updateVertices();
}

glm::ivec2 ImageComponent::getTextureSize() const
{
    if (mTexture)
        return mTexture->getSize();
    else
        return glm::ivec2 {0, 0};
}

glm::vec2 ImageComponent::getSize() const
{
    return GuiComponent::getSize() * (mBottomRightCrop - mTopLeftCrop);
}

void ImageComponent::render(const glm::mat4& parentTrans)
{
    if (!isVisible() || mThemeOpacity == 0.0f || mTexture == nullptr ||
        mTargetSize == glm::vec2 {0.0f, 0.0f} || mSize == glm::vec2 {0.0f, 0.0f})
        return;

    glm::mat4 trans {parentTrans * getTransform()};
    mRenderer->setMatrix(trans);

    if (mTexture && mOpacity > 0.0f) {
        if (Settings::getInstance()->getBool("DebugImage")) {
            glm::vec2 targetSizePos {
                glm::round((mTargetSize - mSize) * mOrigin * glm::vec2 {-1.0f})};
            mRenderer->drawRect(targetSizePos.x, targetSizePos.y, mTargetSize.x, mTargetSize.y,
                                0xFF000033, 0xFF000033);
            if (mClipRegion == glm::vec4 {0.0f, 0.0f, 0.0f, 0.0f})
                mRenderer->drawRect(0.0f, 0.0f, std::round(mSize.x), std::round(mSize.y),
                                    0xFF000033, 0xFF000033);
            else
                mRenderer->drawRect(mClipRegion.x, mClipRegion.y, mClipRegion.z - mClipRegion.x,
                                    mClipRegion.w - mClipRegion.y, 0xFF000033, 0xFF000033);
        }
        // An image with zero size would normally indicate a corrupt image file.
        if (mTexture->getSize() != glm::ivec2 {}) {
            // Actually draw the image.
            // The bind() function returns false if the texture is not currently loaded. A blank
            // texture is bound in this case but we want to handle a fade so it doesn't just
            // 'jump' in when it finally loads. The exception is if the cached background is
            // getting invalidated, in which case we want to make sure to not get a partially
            // faded texture rendered onto the new background.
            if (mWindow->isInvalidatingCachedBackground())
                mTexture->bind();
            else
                fadeIn(mTexture->bind());

            mVertices->saturation = mSaturation * mThemeSaturation;
            mVertices->opacity = mThemeOpacity;
            mVertices->dimming = mDimming;
            mVertices->reflectionsFalloff = mReflectionsFalloff;

            mRenderer->drawTriangleStrips(&mVertices[0], 4);
        }
        else {
            if (!mTexture) {
                LOG(LogError) << "Image texture is not initialized";
            }
            else {
                std::string textureFilePath = mTexture->getTextureFilePath();
                if (textureFilePath != "") {
                    LOG(LogError) << "Image texture for file \"" << textureFilePath
                                  << "\" has zero size";
                }
                else {
                    LOG(LogError) << "Image texture has zero size";
                }
            }
            mTexture.reset();
        }
    }

    GuiComponent::renderChildren(trans);
}

void ImageComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                const std::string& view,
                                const std::string& element,
                                unsigned int properties)
{
    using namespace ThemeFlags;
    GuiComponent::applyTheme(theme, view, element,
                             (properties ^ ThemeFlags::SIZE) |
                                 ((properties & (ThemeFlags::SIZE | POSITION)) ? ORIGIN : 0));

    const ThemeData::ThemeElement* elem {theme->getElement(view, element, "image")};
    if (!elem)
        return;

    glm::vec2 scale {getParent() ?
                         getParent()->getSize() :
                         glm::vec2(Renderer::getScreenWidth(), Renderer::getScreenHeight())};

    bool noMax {false};

    if (properties & ThemeFlags::SIZE) {
        if (elem->has("size")) {
            glm::vec2 imageSize {elem->get<glm::vec2>("size")};
            if (imageSize == glm::vec2 {0.0f, 0.0f}) {
                LOG(LogWarning) << "ImageComponent: Invalid theme configuration, property \"size\" "
                                   "for element \""
                                << element.substr(6) << "\" is set to zero";
                imageSize = {0.001f, 0.001f};
            }
            if (imageSize.x > 0.0f)
                imageSize.x = glm::clamp(imageSize.x, 0.001f, 3.0f);
            if (imageSize.y > 0.0f)
                imageSize.y = glm::clamp(imageSize.y, 0.001f, 3.0f);
            setResize(imageSize * scale);
            if (imageSize.x != 0.0f && imageSize.y != 0.0f)
                noMax = true;
        }
        else if (elem->has("maxSize")) {
            glm::vec2 imageMaxSize {elem->get<glm::vec2>("maxSize")};
            imageMaxSize.x = glm::clamp(imageMaxSize.x, 0.001f, 3.0f);
            imageMaxSize.y = glm::clamp(imageMaxSize.y, 0.001f, 3.0f);
            setMaxSize(imageMaxSize * scale);
        }
    }

    if (elem->has("interpolation")) {
        const std::string interpolation {elem->get<std::string>("interpolation")};
        if (interpolation == "linear") {
            mLinearInterpolation = true;
        }
        else if (interpolation == "nearest") {
            mLinearInterpolation = false;
        }
        else {
            mLinearInterpolation = false;
            LOG(LogWarning) << "ImageComponent: Invalid theme configuration, property "
                               "\"interpolation\" for element \""
                            << element.substr(6) << "\" defined as \"" << interpolation << "\"";
        }
    }

    if (elem->has("default"))
        setDefaultImage(elem->get<std::string>("default"));

    bool tile {elem->has("tile") && elem->get<bool>("tile")};

    if (properties & PATH && elem->has("path")) {
        const std::string path {elem->get<std::string>("path")};

        if (!tile && !theme->isLegacyTheme() && noMax && path.length() > 4 &&
            Utils::String::toLower(path.substr(path.size() - 4, std::string::npos)) == ".svg") {
            mScalableNonAspect = true;
        }

        if (tile && elem->has("tileSize")) {
            glm::vec2 tileSize {elem->get<glm::vec2>("tileSize")};
            if (tileSize.x == 0.0f && tileSize.y == 0.0f) {
                LOG(LogWarning)
                    << "ImageComponent: Invalid theme configuration, property \"tileSize\" "
                       "for element \""
                    << element.substr(6) << "\" is set to zero";
                tile = false;
            }
            else {
                tileSize.x = glm::clamp(tileSize.x, 0.0f, 1.0f);
                tileSize.y = glm::clamp(tileSize.y, 0.0f, 1.0f);
                mTileWidth = tileSize.x * scale.x;
                mTileHeight = tileSize.y * scale.y;
                if (mTileWidth != 0.0f && mTileHeight != 0.0f)
                    mScalableNonAspect = true;
            }
        }

        setImage(path, tile);
    }

    bool updateAlignment {false};

    if (elem->has("tileHorizontalAlignment")) {
        const std::string alignment {elem->get<std::string>("tileHorizontalAlignment")};
        updateAlignment = true;
        if (alignment == "left") {
            mTileHorizontalAlignment = ALIGN_LEFT;
        }
        else if (alignment == "right") {
            mTileHorizontalAlignment = ALIGN_RIGHT;
        }
        else {
            LOG(LogWarning) << "ImageComponent: Invalid theme configuration, property "
                               "\"tileHorizontalAlignment\" for element \""
                            << element.substr(6) << "\" defined as \"" << alignment << "\"";
            mTileHorizontalAlignment = ALIGN_LEFT;
        }
    }

    if (elem->has("tileVerticalAlignment")) {
        const std::string alignment {elem->get<std::string>("tileVerticalAlignment")};
        updateAlignment = true;
        if (alignment == "top") {
            mTileVerticalAlignment = ALIGN_TOP;
        }
        else if (alignment == "bottom") {
            mTileVerticalAlignment = ALIGN_BOTTOM;
        }
        else {
            LOG(LogWarning) << "ImageComponent: Invalid theme configuration, property "
                               "\"tileVerticalAlignment\" for element \""
                            << element.substr(6) << "\" defined as \"" << alignment << "\"";
            mTileVerticalAlignment = ALIGN_TOP;
        }
    }

    if (tile && updateAlignment)
        updateVertices();

    if (properties && elem->has("imageType")) {
        std::string imageTypes {elem->get<std::string>("imageType")};
        for (auto& character : imageTypes) {
            if (std::isspace(character))
                character = ',';
        }
        imageTypes = Utils::String::replace(imageTypes, ",,", ",");
        mThemeImageTypes = Utils::String::delimitedStringToVector(imageTypes, ",");

        if (mThemeImageTypes.empty()) {
            LOG(LogError) << "ImageComponent: Invalid theme configuration, property \"imageType\" "
                             "for element \""
                          << element.substr(6) << "\" contains no values";
        }

        for (std::string& type : mThemeImageTypes) {
            if (std::find(supportedImageTypes.cbegin(), supportedImageTypes.cend(), type) ==
                supportedImageTypes.cend()) {
                LOG(LogError)
                    << "ImageComponent: Invalid theme configuration, property \"imageType\" "
                       "for element \""
                    << element.substr(6) << "\" defined as \"" << type << "\"";
                mThemeImageTypes.clear();
                break;
            }
        }

        std::vector<std::string> sortedTypes {mThemeImageTypes};
        std::stable_sort(sortedTypes.begin(), sortedTypes.end());

        if (std::adjacent_find(sortedTypes.begin(), sortedTypes.end()) != sortedTypes.end()) {
            LOG(LogError) << "ImageComponent: Invalid theme configuration, property \"imageType\" "
                             "for element \""
                          << element.substr(6) << "\" contains duplicate values";
            mThemeImageTypes.clear();
        }
    }

    if (elem->has("metadataElement") && elem->get<bool>("metadataElement"))
        mComponentThemeFlags |= ComponentThemeFlags::METADATA_ELEMENT;

    if (properties & COLOR) {
        if (elem->has("color"))
            setColorShift(elem->get<unsigned int>("color"));
        if (elem->has("colorEnd"))
            setColorShiftEnd(elem->get<unsigned int>("colorEnd"));
        if (elem->has("gradientType")) {
            const std::string gradientType {elem->get<std::string>("gradientType")};
            if (gradientType == "horizontal") {
                setColorGradientHorizontal(true);
            }
            else if (gradientType == "vertical") {
                setColorGradientHorizontal(false);
            }
            else {
                setColorGradientHorizontal(true);
                LOG(LogWarning) << "ImageComponent: Invalid theme configuration, property "
                                   "\"gradientType\" for element \""
                                << element.substr(6) << "\" defined as \"" << gradientType << "\"";
            }
        }
    }

    if (elem->has("scrollFadeIn") && elem->get<bool>("scrollFadeIn"))
        mComponentThemeFlags |= ComponentThemeFlags::SCROLL_FADE_IN;
}

std::vector<HelpPrompt> ImageComponent::getHelpPrompts()
{
    std::vector<HelpPrompt> ret;
    ret.push_back(HelpPrompt("a", "select"));
    return ret;
}

void ImageComponent::resize(bool rasterize)
{
    if (!mTexture)
        return;

    const glm::vec2 textureSize {mTexture->getSourceImageSize()};
    if (textureSize == glm::vec2 {0.0f, 0.0f})
        return;

    if (mTexture->isTiled()) {
        mSize = mTargetSize;
    }
    else {
        if (mTargetIsMax) {
            // Maintain image aspect ratio.
            mSize = textureSize;
            glm::vec2 resizeScale {mTargetSize.x / mSize.x, mTargetSize.y / mSize.y};

            if (resizeScale.x < resizeScale.y) {
                // SVG rasterization is determined by height and rasterization is done in terms of
                // pixels. If rounding is off enough in the rasterization step (for images with
                // extreme aspect ratios), it can cause cutoff when the aspect ratio breaks.
                // So we always make sure to round accordingly to avoid such issues.
                mSize.x *= resizeScale.x;
                mSize.y = floorf(std::min(mSize.y * resizeScale.x, mTargetSize.y));
            }
            else {
                // This will be mTargetSize.y(). We can't exceed it.
                mSize.y *= resizeScale.y;
                mSize.x = std::min((mSize.y / textureSize.y) * textureSize.x, mTargetSize.x);
            }
        }
        else {
            // If both axes are set we just stretch or squash, if no axes are set we do nothing.
            mSize = mTargetSize == glm::vec2 {0.0f, 0.0f} ? textureSize : mTargetSize;

            // If only one axis is set, we resize in a way that maintains aspect ratio.
            if (!mTargetSize.x && mTargetSize.y) {
                mSize.y = mTargetSize.y;
                mSize.x = (mSize.y / textureSize.y) * textureSize.x;
            }
            else if (mTargetSize.x && !mTargetSize.y) {
                mSize.y = (mTargetSize.x / textureSize.x) * textureSize.y;
                mSize.x = (mSize.y / textureSize.y) * textureSize.x;
            }
        }
    }

    // Make sure sub-pixel values are not rounded to zero and that the size is not unreasonably
    // large (which may be caused by a mistake in the theme configuration).
    mSize.x = glm::clamp(mSize.x, 1.0f, mRenderer->getScreenWidth() * 3.0f);
    mSize.y = glm::clamp(mSize.y, 1.0f, mRenderer->getScreenHeight() * 3.0f);

    if (rasterize) {
        mTexture->rasterizeAt(mSize.x, mSize.y);
        onSizeChanged();
    }
}

void ImageComponent::setTileAxes()
{
    if (mTileWidth == 0.0f && mTileHeight == 0.0f) {
        mTileWidth = static_cast<float>(mTexture->getSize().x);
        mTileHeight = static_cast<float>(mTexture->getSize().y);
        return;
    }

    const float ratio {mTexture->getSourceImageSize().x / mTexture->getSourceImageSize().y};
    if (mTileWidth == 0.0f)
        mTileWidth = std::round(mTileHeight * ratio);
    else if (mTileHeight == 0.0f)
        mTileHeight = std::round(mTileWidth / ratio);
}

void ImageComponent::updateVertices()
{
    if (!mTexture)
        return;

    const glm::vec2 topLeft {0.0f, 0.0f};
    const glm::vec2 bottomRight {mSize};
    const float px {mTexture->isTiled() ? mSize.x / getTextureSize().x : 1.0f};
    const float py {mTexture->isTiled() ? mSize.y / getTextureSize().y : 1.0f};

    if (mTileHeight == 0.0f) {
        // clang-format off
        mVertices[0] = {{topLeft.x,     topLeft.y    }, {mTopLeftCrop.x,          py   - mTopLeftCrop.y    }, 0};
        mVertices[1] = {{topLeft.x,     bottomRight.y}, {mTopLeftCrop.x,          1.0f - mBottomRightCrop.y}, 0};
        mVertices[2] = {{bottomRight.x, topLeft.y    }, {mBottomRightCrop.x * px, py   - mTopLeftCrop.y    }, 0};
        mVertices[3] = {{bottomRight.x, bottomRight.y}, {mBottomRightCrop.x * px, 1.0f - mBottomRightCrop.y}, 0};
        // clang-format on
    }
    else {
        // Resize and align tiled textures.
        glm::vec2 topLeftAlign {mTopLeftCrop};
        glm::vec2 bottomRightAlign {mBottomRightCrop};
        const float pxA {mSize.x / mTileWidth};
        const float pyA {mSize.y / mTileHeight};

        if (mTileHorizontalAlignment == Alignment::ALIGN_RIGHT) {
            float offsetX {pxA - std::floor(pxA)};
            if (offsetX != 0.0f) {
                const float moveX {(mTileWidth * offsetX) / mSize.x};
                topLeftAlign.x -= moveX * pxA;
                bottomRightAlign.x -= moveX;
            }
        }

        if (mTileVerticalAlignment == Alignment::ALIGN_TOP) {
            float offsetY {pyA - std::floor(pyA)};
            if (offsetY != 0.0f) {
                const float moveY {(mTileHeight * offsetY) / mSize.y};
                topLeftAlign.y += moveY * pyA;
                bottomRightAlign.y += moveY * pyA;
            }
        }

        // clang-format off
        mVertices[0] = {{topLeft.x,     topLeft.y    }, {topLeftAlign.x,           pyA  - topLeftAlign.y    }, 0};
        mVertices[1] = {{topLeft.x,     bottomRight.y}, {topLeftAlign.x,           1.0f - bottomRightAlign.y}, 0};
        mVertices[2] = {{bottomRight.x, topLeft.y    }, {bottomRightAlign.x * pxA, pyA  - topLeftAlign.y    }, 0};
        mVertices[3] = {{bottomRight.x, bottomRight.y}, {bottomRightAlign.x * pxA, 1.0f - bottomRightAlign.y}, 0};
        // clang-format on
    }

    updateColors();

    // We round the vertices already here in this component as we may otherwise end up with edge
    // cases at sizes near 0.5.
    for (int i = 0; i < 4; ++i)
        mVertices[i].position = glm::round(mVertices[i].position);

    if (mFlipX) {
        for (int i = 0; i < 4; ++i)
            mVertices[i].texcoord[0] = px - mVertices[i].texcoord[0];
    }

    if (mFlipY) {
        for (int i = 0; i < 4; ++i)
            mVertices[i].texcoord[1] = py - mVertices[i].texcoord[1];
    }

    setClipRegion(mClipRegion);
}

void ImageComponent::updateColors()
{
    const float opacity {mOpacity * (mFading ? mFadeOpacity : 1.0f)};
    const unsigned int color {(mColorShift & 0xFFFFFF00) |
                              static_cast<unsigned char>((mColorShift & 0xFF) * opacity)};
    const unsigned int colorEnd {(mColorShiftEnd & 0xFFFFFF00) |
                                 static_cast<unsigned char>((mColorShiftEnd & 0xFF) * opacity)};

    mVertices[0].color = color;
    mVertices[1].color = mColorGradientHorizontal ? color : colorEnd;
    mVertices[2].color = mColorGradientHorizontal ? colorEnd : color;
    mVertices[3].color = colorEnd;
}

void ImageComponent::fadeIn(bool textureLoaded)
{
    if (!mForceLoad) {
        if (!textureLoaded) {
            // Start the fade if this is the first time we've encountered the unloaded texture.
            if (!mFading) {
                // Start with a zero opacity and flag it as fading.
                mFadeOpacity = 0.0f;
                mFading = true;
                updateColors();
            }
        }
        else if (mFading) {
            // The texture is loaded and we need to fade it in. The fade is based on the frame
            // rate and is 1/4 second if running at 60 frames per second although the actual
            // value is not that important.
            float opacity {mFadeOpacity + 1.0f / 15.0f};
            // See if we've finished fading.
            if (opacity >= 1.0f) {
                mFadeOpacity = 1.0f;
                mFading = false;
            }
            else {
                mFadeOpacity = opacity;
            }
            updateColors();
        }
    }
}
