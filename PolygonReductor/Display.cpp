#include <glad/glad.h>
#include <Display.h>
#include <iostream>


// glfw: Window creation
// ---------------------
GLFWwindow* WindowStart(int ScreenWidth, int ScreenHeight, const char* WindowName) {
    GLFWwindow* window = glfwCreateWindow(ScreenWidth, ScreenHeight, WindowName, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowAspectRatio(window, ScreenWidth, ScreenHeight);
    return window;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}