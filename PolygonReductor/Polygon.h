#pragma once

class Polygon
{
public:
	//Vertex and index setup, as well as buffers
	Polygon(const char* filepath);
	//parsing the file for the vertices and indices
	void parse(const char* filepath);
	//deleting vertices
	void Reshape();
	//deleting buffers
	void DeleteBuffer();
	//the buffers
	unsigned int VBO, VAO, EBO;
private:
	float vertices[999];
	unsigned int indices[999];
	unsigned int vertex_count, index_count;
};