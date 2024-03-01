#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void processInput(class GLFWwindow* window);
void Rendering(class Shader shader, class GLFWwindow* window, unsigned int VAO);