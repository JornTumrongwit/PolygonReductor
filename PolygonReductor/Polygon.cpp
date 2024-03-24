#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Polygon.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stack>

Polygon::Polygon() {
	//haha get blank'd on
}

// OBJ file parser function. Used for loading the graph file.
void Polygon::parse(const char* filepath) {
	FILE* fp;
	int c1, c2;
	float x, y, z;
	unsigned int ix, iy, iz, t1, ng1, t2, ng2;
	std::unordered_map<int, std::unordered_map<int, int>> edgemap;

	fopen_s(&fp, filepath, "rb");

	if (fp == NULL) {
		std::cerr << "Error loading file: " << filepath << std::endl;
		std::getchar();
		exit(-1);
	}

	while (!feof(fp)) {
		c1 = fgetc(fp);
		while (!(c1 == 'v' || c1 == 'i')) {
			c1 = fgetc(fp);
			if (feof(fp))
				break;
		}
		c2 = fgetc(fp);

		if ((c1 == 'v') && (c2 == ' ')) {
			//strong vertices
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			vertices.push_back(x / 1000.0);
			vertices.push_back(y / 1000.0); 
			vertices.push_back(z / 1000.0);
			vertex_count += 3;
		}
		else if ((c1 == 'i') && (c2 == ' ')) {
			//storing the halfedges
			fscanf_s(fp, "%d %d %d", &ix, &iy, &iz);
			//store in the edgemap
			edgemap[iy][ix] = d_edge.size()/2;
			edgemap[iz][iy] = d_edge.size()/2 + 1;
			edgemap[ix][iz] = d_edge.size()/2 + 2;
			//store the vertex, and twin
			d_edge.push_back(iy);
			if (edgemap.find(ix) != edgemap.end()) {
				if (edgemap[ix].find(iy) != edgemap[ix].end()) {
					d_edge[edgemap[ix][iy]*2 + 1] = (d_edge.size() - 1)/2;
					d_edge.push_back(edgemap[ix][iy]);
				}
				else d_edge.push_back(-1);
			} else d_edge.push_back(-1);

			d_edge.push_back(iz);
			if (edgemap.find(iy) != edgemap.end()) {
				if (edgemap[iy].find(iz) != edgemap[iy].end()) {
					d_edge[edgemap[iy][iz]*2 + 1] = (d_edge.size() - 1)/2;
					d_edge.push_back(edgemap[iy][iz]);
				}
				else d_edge.push_back(-1);
			}
			else d_edge.push_back(-1);

			d_edge.push_back(ix);
			if (edgemap.find(iz) != edgemap.end()) {
				if (edgemap[iz].find(ix) != edgemap[iz].end()) {
					d_edge[edgemap[iz][ix]*2 + 1] = (d_edge.size() - 1)/2;
					d_edge.push_back(edgemap[iz][ix]);
				}
				else d_edge.push_back(-1);
			}
			else d_edge.push_back(-1);
		}
	}
	construct();
	fclose(fp); // Finished parsing
}

Polygon::Polygon(const char* filepath) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	this->vertex_count = 0;
	this->index_count = 0;
	parse(filepath);
}

void Polygon::construct() {
	indices.clear();
	index_count = 0;
	added_flags.clear();
	construct_help(starter);
}

void Polygon::construct_help(unsigned int start) {
	//A single halfedge will mean that this triangle is already used in a manifold graph
	if (added_flags.count(start)) return;
	unsigned int prev_ind = prev(start);
	unsigned int next_ind = next(start);
	//push all the triangle indices
	indices.push_back(d_edge[prev_ind*2]);
	indices.push_back(d_edge[start*2]);
	indices.push_back(d_edge[next_ind*2]);
	//flag that this triangle is added
	added_flags.insert(start);
	added_flags.insert(prev_ind);
	added_flags.insert(next_ind);
	index_count += 3;
	//construct the other triangles next to it
	if(d_edge[prev_ind*2 + 1] != -1) construct_help(d_edge[prev_ind*2 + 1]);
	if (d_edge[next_ind*2 + 1] != -1) construct_help(d_edge[next_ind*2 + 1]);
	if (d_edge[start*2 + 1] != -1) construct_help(d_edge[start*2 + 1]);
}

int Polygon::prev(unsigned int ind) {
	return (ind % 3 == 0) ? (ind + 2) : (ind - 1);
}

int Polygon::next(unsigned int ind) {
	return (ind % 3 == 2) ? (ind - 2) : (ind + 1);
}

int Polygon::twin(unsigned int ind) {
	return d_edge[ind* 2 + 1];
}

void Polygon::Init() {
	for (auto vtx : d_edge) std::cout << vtx << "\n";
	std::cout << "\n";
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_count, &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_count, &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

//checking whether an edge is a perimeter
bool Polygon::is_perim(unsigned int v1, unsigned int v2) {
	return perimeters[v1] && perimeters[v2] && bool(outer[v1].count(v2));
}

// Contract the specified edge
void Polygon::collapse(unsigned int edge){
	//moving the starting edge (where it moves doesn't matter as long as it doesn't start at the deleted triangle
	//cannot use: this edge, its twin, this prev, this next
	if (starter == edge || starter == twin(edge) || starter == prev(edge) || starter == next(edge)){
		starter = twin(prev(edge));
		if (starter == -1) {
			twin(next(edge));
			if (starter == -1) {
				//just use something from the other triangle and pray for the best
				if (twin(edge) == -1) {
					//giveup and cry
					std::cout << "DON'T COLLAPSE THIS I DON'T WANNA DIIIIIIIIIIIIIIIIIIIE!";
					return;
				}
				starter = prev(twin(edge));
			}
		}
	}
	contracts.push(edge);
	int eng = twin(edge);
	//move all the edges to become twins of each other
	if(twin(prev(edge)) >= 0) d_edge[twin(prev(edge)) * 2 + 1] = twin(next(edge));
	if(twin(next(edge)) >= 0) d_edge[twin(next(edge)) * 2 + 1] = twin(prev(edge));
	if (eng != -1) {
		if (twin(prev(eng)) >= 0) d_edge[twin(prev(eng)) * 2 + 1] = twin(next(eng));
		if (twin(next(eng)) >= 0) d_edge[twin(next(eng)) * 2 + 1] = twin(prev(eng));
	}
	//changing the vertex
	unsigned int newvtx = d_edge[edge * 2];
	std::cout <<"NEW VERTEX: "<< d_edge[edge * 2] << "\n\n";
	//spin clockwise until the start is found
	int the = edge;
	if (d_edge[the * 2 + 1] != -1) {
		the = next(twin(the));
		while (twin(the) != -1 && twin(the) != edge) {
			std::cout << "CURRENT THE: " << the << "\n";
			the = next(twin(the));
		}
		std::cout << "STOPPED BECAUSE: " << twin(the) << " " << twin(the) << "\n";
	}
	std::cout << "STARTING MOVING VERTEX AT EDGE: " << the << "\n";
	//counterclockwise spin while adjusting the vertex
	int start = the;
	std::cout << "MODIFYING: " << prev(the) << " MOVING " << d_edge[prev(the) * 2] << " AS SIGNALED BY " << the << "\n";
	d_edge[prev(the) * 2] = newvtx;
	the = twin(prev(the));
	while (the != start && the >= 0) {
		if (the == edge) {
			the = twin(prev(the));
			continue;
		}
		std::cout << "MODIFYING: " << prev(the) << " MOVING " << d_edge[prev(the) * 2] << " AS SIGNALED BY " << the << "\n";
		d_edge[prev(the) * 2] = newvtx;
		the = twin(prev(the));
	}
	construct();
	refresh();
	for (auto vtx : d_edge) std::cout << vtx << "\n";
	std::cout << "\n";
}

bool Polygon::boundary(int edge) {
	return (twin(edge) == -1);
}

void Polygon::split() {
	int edge = contracts.top();
	contracts.pop();

	construct();
	refresh();
}


void Polygon::refresh() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->index_count, &indices[0], GL_STATIC_DRAW);
	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
}

//Deleting the buffers
void Polygon::DeleteBuffer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}