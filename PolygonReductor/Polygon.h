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
	int prev(unsigned int ind);
	//geting the next edge
	int next(unsigned int ind);
	//geting the twin edge
	int twin(unsigned int ind);
	//collapsing edge
	bool collapse(unsigned int edge);
	//check if something is a boundary
	bool boundary(int edge);
	//splitting vertex from queue
	void split();
	// better have a function to print out smth
	void printedge();
	//deleting buffers
	void DeleteBuffer();
	//refresh buffers
	void refresh();
	//the buffers
	unsigned int VBO, VAO, EBO;
	unsigned int vertex_count, index_count;

private:
	int starter = 0;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	//Directed edges
	std::vector<int> d_edge;
	//construction flags
	std::set<int> added_flags;
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
	//collapsed edges
	std::set<unsigned int> collapsed;

	// -------------------- Cost function Data Structures -------------------- //

	std::vector<float> vertex_cost;

	// --------------------- Cost function (and helpers) --------------------- //
	std::vector<unsigned int> get_incident_vert(unsigned int);
	//std::vector<float> get_normal(unsigned int);
	std::vector<float> get_normal(unsigned int, unsigned int);
	void init_QEM();
	std::vector<unsigned int> get_min_edge();
	//unsigned int calc_init_vertex_cost(unsigned int);
	void calc_init_vertex_cost(unsigned int, unsigned int);
	void update_collapse_cost(unsigned int);
};