//
//  Shader_GL21.h
//
//  OpenGL 2.1 GLSL shader functions.
//

#ifndef ES_CORE_RENDERER_SHADER_GL21_H
#define ES_CORE_RENDERER_SHADER_GL21_H

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <array>
#include <string>
#include <vector>

namespace Renderer
{
    class Shader
    {
    public:
        enum shaderNames {
            Desaturate
        };

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
        void setVariable(GLfloat shaderFloat, int index = 0);
        void setVariable(std::array<GLfloat, 4> shaderVec4, int index = 0);
        // Sets the shader program to use the loaded shaders.
        void activateShaders();
        // Sets the shader program to 0 which reverts to the fixed function pipeline.
        void deactivateShaders();
        // Returns the program ID that was generated by glCreateProgram().
        GLuint getProgramID();
        // Only used for error logging if the shaders fail to compile or link.
        void printProgramInfoLog(GLuint programID);
        void printShaderInfoLog(GLuint shaderID);

    private:
        GLuint mProgramID;
        std::vector<std::tuple<std::string, std::string, GLenum>> shaderVector;

        // Variables used for communication with the compiled shaders.
        GLint shaderFloat_0;
        GLint shaderFloat_1;
        GLint shaderFloat_2;
        GLint shaderVec4_0;
        GLint shaderVec4_1;
        GLint shaderVec4_2;
    };
} // Renderer

#endif // ES_CORE_RENDERER_SHADER_GL21_H
