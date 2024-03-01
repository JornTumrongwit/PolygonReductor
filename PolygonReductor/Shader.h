#pragma once

#include <GLFW/glfw3.h>
#include <string>

class Shader
{
public:
	// Shader Compilation: Helps with compiling vertex shaders into a buffer for OpenGL to use
	// --------------------------------------------------------------------------------------
	unsigned int CompileVertexShader(std::string file);

	// Shader Compilation: Helps with compiling fragment shaders into a buffer for OpenGL to use
	// ----------------------------------------------------------------------------------------
	unsigned int CompileFragmentShader(std::string file);

	// Shader Compilation: Link the shaders that we will use
	// -----------------------------------------------------
	void LinkShaders(std::string vs, std::string fs);

	// Shader Usage: Telling OGL to use this shader
	// --------------------------------------------
	void use() const;

	// Shader ID getter
	// ----------------
	unsigned int GetID() const;

	// Shader deletion
	void ShaderDelete() const;

	// utility uniform functions
	// -------------------------
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec2(const std::string& name, float value1, float value2) const;
	void setVec3(const std::string& name, float value1, float value2, float value3) const;
	void setVec4(const std::string& name, float value1, float value2, float value3, float value4) const;

private:
	unsigned int ShaderID;
};