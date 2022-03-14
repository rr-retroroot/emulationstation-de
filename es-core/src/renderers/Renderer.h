//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  Renderer.h
//
//  Generic rendering functions.
//

#ifndef ES_CORE_RENDERER_RENDERER_H
#define ES_CORE_RENDERER_RENDERER_H

#include "Log.h"
#include "utils/MathUtil.h"

#include <stack>
#include <string>
#include <vector>

struct SDL_Window;

class Renderer
{
public:
    enum class TextureType {
        RGBA, // Replace with AllowShortEnumsOnASingleLine: false (clang-format >=11.0).
        BGRA,
        RED
    };

    enum class BlendFactor {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA
    };

    // clang-format off
    enum Shader {
        CORE =            0x00000001,
        BLUR_HORIZONTAL = 0x00000002,
        BLUR_VERTICAL =   0x00000004,
        SCANLINES =       0x00000008
    };

    enum ShaderFlags {
        BGRA_TO_RGBA =    0x00000001,
        FONT_TEXTURE =    0x00000002,
        POST_PROCESSING = 0x00000004
    };
    // clang-format on

    struct Vertex {
        glm::vec2 position;
        glm::vec2 texture;
        unsigned int color;
        float opacity;
        float saturation;
        float dimming;
        unsigned int shaders;
        unsigned int shaderFlags;

        Vertex()
            : opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , shaders {0}
            , shaderFlags {0}
        {
        }

        Vertex(const glm::vec2& position, const glm::vec2& textureCoord, const unsigned int color)
            : position(position)
            , texture(textureCoord)
            , color(color)
            , opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , shaders {0}
            , shaderFlags {0}
        {
        }
    };

    struct postProcessingParams {
        float opacity;
        float saturation;
        float dimming;
        unsigned int blurPasses;
        unsigned int shaders;

        postProcessingParams()
            : opacity {1.0f}
            , saturation {1.0f}
            , dimming {1.0f}
            , blurPasses {1}
            , shaders {0}
        {
        }
    };

    struct Rect {
        Rect(const int xValue, const int yValue, const int wValue, const int hValue)
            : x(xValue)
            , y(yValue)
            , w(wValue)
            , h(hValue)
        {
        }
        int x;
        int y;
        int w;
        int h;
    };

    static Renderer* getInstance();

    void setIcon();
    bool createWindow();
    void destroyWindow();
    bool init();
    void deinit();

    virtual bool loadShaders() = 0;

    void pushClipRect(const glm::ivec2& pos, const glm::ivec2& size);
    void popClipRect();

    void drawRect(const float x,
                  const float y,
                  const float w,
                  const float h,
                  const unsigned int color,
                  const unsigned int colorEnd,
                  bool horizontalGradient = false,
                  const float opacity = 1.0,
                  const float dimming = 1.0,
                  const BlendFactor srcBlendFactor = BlendFactor::SRC_ALPHA,
                  const BlendFactor dstBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA);

    const glm::mat4& getProjectionMatrix()
    {
        if (mScreenRotated)
            return mProjectionMatrixRotated;
        else
            return mProjectionMatrix;
    }
    const glm::mat4& getProjectionMatrixNormal() { return mProjectionMatrix; }
    SDL_Window* getSDLWindow() { return mSDLWindow; }
    const bool getScreenRotated() { return mScreenRotated; }
    const float getWindowWidth() { return static_cast<float>(mWindowWidth); }
    const float getWindowHeight() { return static_cast<float>(mWindowHeight); }
    static const float getScreenWidth() { return static_cast<float>(sScreenWidth); }
    static const float getScreenHeight() { return static_cast<float>(sScreenHeight); }
    static const float getScreenWidthModifier() { return sScreenWidthModifier; }
    static const float getScreenHeightModifier() { return sScreenHeightModifier; }
    static const float getScreenAspectRatio() { return sScreenAspectRatio; }

    static constexpr glm::mat4 getIdentity() { return glm::mat4 {1.0f}; }
    glm::mat4 mTrans {getIdentity()};

    virtual void shaderPostprocessing(
        const unsigned int shaders,
        const Renderer::postProcessingParams& parameters = postProcessingParams(),
        unsigned char* textureRGBA = nullptr) = 0;

    virtual void setup() = 0;
    virtual bool createContext() = 0;
    virtual void destroyContext() = 0;
    virtual unsigned int createTexture(const TextureType type,
                                       const bool linearMinify,
                                       const bool linearMagnify,
                                       const bool repeat,
                                       const unsigned int width,
                                       const unsigned int height,
                                       void* data) = 0;
    virtual void destroyTexture(const unsigned int texture) = 0;
    virtual void updateTexture(const unsigned int texture,
                               const TextureType type,
                               const unsigned int x,
                               const unsigned int y,
                               const unsigned int width,
                               const unsigned int height,
                               void* data) = 0;
    virtual void bindTexture(const unsigned int texture) = 0;
    virtual void drawTriangleStrips(
        const Vertex* vertices,
        const unsigned int numVertices,
        const BlendFactor srcBlendFactor = BlendFactor::SRC_ALPHA,
        const BlendFactor dstBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA) = 0;
    virtual void setMatrix(const glm::mat4& matrix) = 0;
    virtual void setScissor(const Rect& scissor) = 0;
    virtual void setSwapInterval() = 0;
    virtual void swapBuffers() = 0;

private:
    std::stack<Rect> mClipStack;
    SDL_Window* mSDLWindow {nullptr};
    glm::mat4 mProjectionMatrix {};
    glm::mat4 mProjectionMatrixRotated {};
    int mWindowWidth {0};
    int mWindowHeight {0};
    static inline int sScreenWidth {0};
    static inline int sScreenHeight {0};
    int mScreenOffsetX {0};
    int mScreenOffsetY {0};
    bool mScreenRotated {0};
    bool mInitialCursorState {1};
    // Screen resolution modifiers relative to the 1920x1080 reference.
    static inline float sScreenHeightModifier {0.0f};
    static inline float sScreenWidthModifier {0.0f};
    static inline float sScreenAspectRatio {0.0f};
};

#endif // ES_CORE_RENDERER_RENDERER_H
