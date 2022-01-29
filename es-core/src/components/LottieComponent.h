//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  LottieComponent.h
//
//  Component to play Lottie animations using the rlottie library.
//

#ifndef ES_CORE_COMPONENTS_LOTTIE_COMPONENT_H
#define ES_CORE_COMPONENTS_LOTTIE_COMPONENT_H

#include "GuiComponent.h"
#include "renderers/Renderer.h"
#include "resources/TextureResource.h"
#include "utils/MathUtil.h"

#include "rlottie.h"

#include <chrono>
#include <future>
#include <unordered_map>

class LottieComponent : public GuiComponent
{
public:
    LottieComponent();
    ~LottieComponent();

    void setAnimation(const std::string& path);
    void setKeepAspectRatio(bool value) { mKeepAspectRatio = value; }
    void setFrameCaching(bool value) { mCacheFrames = value; }
    void setMaxCacheSize(int value)
    {
        mMaxCacheSize = static_cast<size_t>(glm::clamp(value, 0, 1024) * 1024 * 1024);
    }

    void resetFileAnimation() override;
    void onSizeChanged() override;

    virtual void applyTheme(const std::shared_ptr<ThemeData>& theme,
                            const std::string& view,
                            const std::string& element,
                            unsigned int properties) override;

private:
    void update(int deltaTime) override;
    void render(const glm::mat4& parentTrans) override;

    std::shared_ptr<TextureResource> mTexture;
    std::vector<uint8_t> mPictureRGBA;
    std::unordered_map<size_t, std::vector<uint8_t>> mFrameCache;
    // Set a 1024 MiB total Lottie animation cache as default.
    static inline size_t mMaxTotalFrameCache = 1024 * 1024 * 1024;
    static inline size_t mTotalFrameCache;
    bool mCacheFrames;
    size_t mMaxCacheSize;
    size_t mCacheSize;
    size_t mFrameSize;

    std::chrono::time_point<std::chrono::system_clock> mAnimationStartTime;
    std::unique_ptr<rlottie::Animation> mAnimation;
    std::unique_ptr<rlottie::Surface> mSurface;
    std::future<rlottie::Surface> mFuture;
    std::string mPath;
    std::string mStartDirection;
    std::string mDirection;
    size_t mTotalFrames;
    size_t mFrameNum;

    double mFrameRate;
    float mSpeedModifier;
    int mTargetPacing;
    int mTimeAccumulator;
    int mLastRenderedFrame;
    int mSkippedFrames;

    bool mHoldFrame;
    bool mPause;
    bool mAlternate;
    bool mKeepAspectRatio;
};

#endif // ES_CORE_COMPONENTS_LOTTIE_COMPONENT_H
