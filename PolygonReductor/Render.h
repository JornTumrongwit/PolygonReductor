#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void processInput(struct GLFWwindow* window);
void Rendering(class Shader shader, struct GLFWwindow* window, unsigned int VAO, unsigned int lines);