#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Polygon.h>

Polygon::Polygon(float* vertices, unsigned int* indices, unsigned int vertex_count, unsigned int index_count) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    for (unsigned int i = 0; i < vertex_count; i++) {
        this->vertices[i] = *vertices;
        vertices++;
    }
    for (unsigned int i = 0; i < index_count; i++) {
        this->indices[i] = *indices;
        indices++;
    }
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count, this->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count, this->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// TODO: make a polygon reshape function (This is likely where the graph reduction algorithm goes)
void Polygon::Reshape() {

}