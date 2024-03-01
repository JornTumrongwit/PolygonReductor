#pragma once
#include <GLFW/glfw3.h>

// glfw: Window creation
// ---------------------
GLFWwindow* WindowStart(int ScreenWidth, int ScreenHeight, const char* WindowName);

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);