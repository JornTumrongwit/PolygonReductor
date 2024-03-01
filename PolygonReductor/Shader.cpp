#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <Shader.h>
#include <fstream>
#include <streambuf>

// Shader Compilation: Helps with compiling shaders into a buffer for OpenGL to use
// --------------------------------------------------------------------------------
unsigned int Shader::CompileVertexShader(std::string file)
{
    std::ifstream ifs(file);
    std::string stringFile = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    const char* FileContent = stringFile.c_str();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &FileContent, NULL);
    glCompileShader(vertexShader);
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return vertexShader;
}

// Shader Compilation: Helps with compiling fragment shaders into a buffer for OpenGL to use
// ----------------------------------------------------------------------------------------
unsigned int Shader::CompileFragmentShader(std::string file)
{
    std::ifstream ifs(file);
    std::string stringFile = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    const char* FileContent = stringFile.c_str();
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &FileContent, NULL);
    glCompileShader(fragmentShader);
    int  success;
    char infoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return fragmentShader;
}

// Shader Compilation: Link the shaders that we will use
// -----------------------------------------------------
void Shader::LinkShaders(std::string vs, std::string fs)
{
    unsigned int vertexShader = CompileVertexShader(vs);
    unsigned int fragmentShader = CompileFragmentShader(fs);
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    ShaderID = shaderProgram;
}

// utility uniform functions
// -------------------------
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ShaderID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ShaderID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ShaderID, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, float value1, float value2) const {
    glUniform2f(glGetUniformLocation(ShaderID, name.c_str()), value1, value2);
}
void Shader::setVec3(const std::string& name, float value1, float value2, float value3) const {
    glUniform3f(glGetUniformLocation(ShaderID, name.c_str()), value1, value2, value3);
}
void Shader::setVec4(const std::string& name, float value1, float value2, float value3, float value4) const {
    glUniform4f(glGetUniformLocation(ShaderID, name.c_str()), value1, value2, value3, value4);
}

// Shader Usage: Telling OGL to use this shader
// --------------------------------------------
void Shader::use() const { glUseProgram(ShaderID); }

// Shader ID getter
// ----------------
unsigned int Shader::GetID() const { return ShaderID; }

// Shader deletion
// ---------------
void Shader::ShaderDelete() const { glDeleteProgram(ShaderID); }