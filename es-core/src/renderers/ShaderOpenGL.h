//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  ShaderOpenGL.h
//
//  OpenGL / OpenGL ES shader functions.
//

#ifndef ES_CORE_RENDERER_SHADER_OPENGL_H
#define ES_CORE_RENDERER_SHADER_OPENGL_H

#define GL_GLEXT_PROTOTYPES

#include "renderers/Renderer.h"
#include "utils/MathUtil.h"

#if defined(_WIN64)
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>
#if defined(USE_OPENGLES)
#include <GLES3/gl3.h>
#include <SDL2/SDL_opengles.h>
#else
#include <SDL2/SDL_opengl.h>
#endif
#include <array>
#include <string>
#include <vector>

#if !defined(NDEBUG)
#define GL_CHECK_ERROR(Function) (Function, _GLCheckError(#Function))
static inline void _GLCheckError(const std::string& funcName)
{
    const GLenum errorCode = glGetError();

    if (errorCode != GL_NO_ERROR) {
#if defined(USE_OPENGLES)
        LOG(LogError) << "OpenGL ES error: " << funcName << " failed with error code: 0x"
                      << std::hex << errorCode;
#else
        LOG(LogError) << "OpenGL error: " << funcName << " failed with error code: 0x" << std::hex
                      << errorCode;
#endif
    }
}
#else
#define GL_CHECK_ERROR(Function) (Function)
#endif

class ShaderOpenGL
{
public:
    ShaderOpenGL();
    ~ShaderOpenGL();

    // Loads the shader source code only, no compilation done at this point.
    void loadShaderFile(const std::string& path, GLenum shaderType);
    // Compilation, shader attachment and linking.
    bool createProgram();
    // Only used for a clean shutdown.
    void deleteProgram(GLuint programID);
    // Get references to the variables inside the compiled shaders.
    void getVariableLocations(GLuint programID);
    // One-way communication with the compiled shaders.
    void setModelViewProjectionMatrix(glm::mat4 mvpMatrix);

    void setAttribPointers();
    void setTextureSize(std::array<GLfloat, 2> shaderVec2);
    void setOpacity(GLfloat opacity);
    void setSaturation(GLfloat saturation);
    void setDimming(GLfloat dimming);
    void setReflectionsFalloff(GLfloat falloff);
    void setFlags(GLuint flags);
    // Sets the shader program to use the loaded shaders.
    void activateShaders();
    // Sets the shader program to 0 which reverts to the fixed function pipeline.
    void deactivateShaders();
    // Returns the program ID that was generated by glCreateProgram().
    GLuint getProgramID() { return mProgramID; }
    // Only used for error logging if the shaders fail to compile or link.
    void printProgramInfoLog(GLuint programID);
    void printShaderInfoLog(GLuint shaderID, GLenum shaderType, bool error);

private:
    GLuint mProgramID;
    std::vector<std::tuple<std::string, std::string, GLenum>> mShaderVector;

    // Variables used for communication with the compiled shaders.
    GLint mShaderMVPMatrix;
    GLint mShaderPosition;
    GLint mShaderTextureCoord;
    GLint mShaderColor;
    GLint mShaderTextureSize;
    GLint mShaderOpacity;
    GLint mShaderSaturation;
    GLint mShaderDimming;
    GLint mShaderReflectionsFalloff;
    GLint mShaderFlags;
};

#endif // ES_CORE_RENDERER_SHADER_OPENGL_H
