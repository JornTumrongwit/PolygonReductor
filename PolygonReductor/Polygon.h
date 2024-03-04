#pragma once

class Polygon
{
public:
	Polygon(float* vertices, unsigned int* indices, unsigned int vertex_count, unsigned int index_count);
	void Reshape();
	unsigned int VBO, VAO, EBO;
private:
	float vertices[999];
	unsigned int indices[999];
};