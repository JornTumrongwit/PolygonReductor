#pragma once
#include <unordered_map>
#include <set>
#include <vector>
#include <stack>

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
	//constructing the triangles
	void construct();
	//construction helper
	void construct_help(unsigned int start);
	//getting previous edge
	unsigned int prev(unsigned int ind);
	//geting the next edge
	unsigned int next(unsigned int ind);
	//collapsing edge
	void Contract(unsigned int v1, unsigned int v2);
	//splitting vertex from queue
	void Split();
	//deleting buffers
	void DeleteBuffer();
	//refresh buffers
	void refresh();
	//util for checking "side" of the line the point is in
	bool is_above(unsigned int index, float m, float b);
	//util for moving the edge in splitting
	void splitter(unsigned int v1, unsigned int v2, unsigned int head1, unsigned int head2, float m, float b);
	//check if an edge is a perimeter edge
	bool is_perim(unsigned int v1, unsigned int v2);
	//modification of perimeter after split
	void perim_split();
	//the buffers
	unsigned int VBO, VAO, EBO;
	unsigned int vertex_count, index_count;
private:
	unsigned int starter = 0;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	//Directed edges
	std::vector<int> d_edge;
	//construction flags
	std::set<int> added_flags;
	std::unordered_map<unsigned int, std::set<unsigned int>> edges;
	//queue structure (in order of pop)
	// first triangle head (also determines if it is a single-triangle op) (if it is a line collapse, head = v1)
	// second triangle head
	// vertex you contract to
	// vertex that moved into the contract
	// v1 normal
	// v2 normal
	std::stack<unsigned int> contracts;
	// changes in perimeter
	std::stack<unsigned int> normal_mod;
	// perimeter vectors
	std::vector<bool> perimeters;
	// outer edges
	std::unordered_map<unsigned int, std::set<unsigned int>> outer;
	std::set<unsigned int> discarded;
};