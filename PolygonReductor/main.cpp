#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <Shader.h>
#include <Display.h>
#include <ProgInit.h>
#include <Render.h>
#include <Polygon.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const bool wireframe = true;
const bool dot = true;
const char* filepath = "test_graph.grp";

int main()
{
    Init();

    GLFWwindow* window = WindowStart(SCR_WIDTH, SCR_HEIGHT, "POLYGON REDUCTOR");
    if (window == NULL) {
        return -1;
    }
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
}

    // Building and compiling shader programs
    // --------------------------------------
    Shader shader = Shader();
    shader.LinkShaders("VertexShader.vs", "FragmentShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------


    Polygon polygon = Polygon(filepath);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // draw in wireframe polygons.
    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (dot) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(5);
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        Rendering(shader, window, polygon.VAO, 12);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    
    shader.ShaderDelete();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}