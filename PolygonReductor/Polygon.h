#pragma once
#include <unordered_map>
#include <set>
#include <vector>
#include <queue>

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
	//queue structure (in order of pop)
	// first triangle head (also determines if it is a single-triangle op)
	// second triangle head
	// vertex you contract to
	// vertex that moved into the contract
	std::queue<unsigned int> contracts;
};