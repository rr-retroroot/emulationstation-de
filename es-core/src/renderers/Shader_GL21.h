//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  Shader_GL21.h
//
//  OpenGL 2.1 GLSL shader functions.
//

#ifndef ES_CORE_RENDERER_SHADER_GL21_H
#define ES_CORE_RENDERER_SHADER_GL21_H

#define GL_GLEXT_PROTOTYPES

#include "utils/MathUtil.h"

#if defined(_WIN64)
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>
// Hack until shader support has been added for OpenGL ES.
#if defined(USE_OPENGL_21)
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles.h>
#endif
#include <array>
#include <string>
#include <vector>

namespace Renderer
{
    class Shader
    {
    public:
        Shader();
        ~Shader();

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

        void setTextureSize(std::array<GLfloat, 2> shaderVec2);
        void setTextureCoordinates(std::array<GLfloat, 4> shaderVec4);
        void setColor(std::array<GLfloat, 4> shaderVec4);
        void setSaturation(GLfloat saturation);
        void setOpacity(GLfloat opacity);
        void setDimValue(GLfloat dimValue);
        // Sets the shader program to use the loaded shaders.
        void activateShaders();
        // Sets the shader program to 0 which reverts to the fixed function pipeline.
        void deactivateShaders();
        // Returns the program ID that was generated by glCreateProgram().
        GLuint getProgramID() { return mProgramID; }
        // Only used for error logging if the shaders fail to compile or link.
        void printProgramInfoLog(GLuint programID);
        void printShaderInfoLog(GLuint shaderID, GLenum shaderType);

    private:
        GLuint mProgramID;
        std::vector<std::tuple<std::string, std::string, GLenum>> shaderVector;

        // Variables used for communication with the compiled shaders.
        GLint shaderMVPMatrix;
        GLint shaderTextureSize;
        GLint shaderTextureCoord;
        GLint shaderColor;
        GLint shaderSaturation;
        GLint shaderOpacity;
        GLint shaderDimValue;
    };

} // namespace Renderer

#endif // ES_CORE_RENDERER_SHADER_GL21_H
