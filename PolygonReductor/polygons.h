#pragma once

class Polygon
{
public:
	Polygon(float* vertices, unsigned int* indices, unsigned int vertex_count, unsigned int index_count);
	void Reshape();
private:
	float vertices[999];
	unsigned int indices[999];
	unsigned int VBO, VAO, EBO;
};