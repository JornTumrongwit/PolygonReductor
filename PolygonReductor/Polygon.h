#pragma once
#include <unordered_map>
#include <set>
#include <vector>

class Polygon
{
public:
	//default constructor because debugger is whiny
	Polygon();
	//Vertex and index setup, as well as buffers
	Polygon(const char* filepath);
	//parsing the file for the vertices and indices
	void parse(const char* filepath);
	//initiating the buffers
	void Init();
	//deleting vertices
	void Contract(unsigned int v1, unsigned int v2);
	//deleting buffers
	void DeleteBuffer();
	//the buffers
	unsigned int VBO, VAO, EBO;
private:
	float vertices[999];
	unsigned int indices[999];
	unsigned int vertex_count, index_count;
	std::unordered_map<unsigned int, std::set<unsigned int>> edges;
	std::vector<unsigned int> contracts;
};