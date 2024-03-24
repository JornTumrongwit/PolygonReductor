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
	int eng = twin(edge);
	//move all the edges to become twins of each other
	if(twin(prev(edge)) > 0) d_edge[twin(prev(edge)) * 2 + 1] = twin(next(edge));
	if(twin(next(edge)) > 0) d_edge[twin(next(edge)) * 2 + 1] = twin(prev(edge));
	if (eng != -1) {
		if (twin(prev(eng)) > 0) d_edge[twin(prev(eng)) * 2 + 1] = twin(next(eng));
		if (twin(next(eng)) > 0) d_edge[twin(next(eng)) * 2 + 1] = twin(prev(eng));
	}
	//changing the vertex
	unsigned int newvtx = d_edge[edge * 2];
	//spin clockwise until the start is found
	int the = edge;
	if (d_edge[the * 2 + 1] != -1) {
		the = next(twin(the));
		while (twin(next(twin(the))) != -1 || next(twin(the)) != edge) {
			the = next(twin(the));
		}
	}
	//counterclockwise spin while adjusting the vertex
	int starter = the;
	d_edge[twin(the) * 2] = newvtx;
	the = twin(prev(the));
	while (the != starter || the != -1) {
		d_edge[the * 2] = newvtx;
		the = twin(prev(the));
	}
	construct();
	refresh();
}

void Polygon::Split() {
	if (this->contracts.size() <= 0) return;
	bool p2 = this->contracts.top();
	this->contracts.pop();
	bool p1 = this->contracts.top();
	this->contracts.pop();
	unsigned int head1 = this->contracts.top();
	this->contracts.pop();
	unsigned int head2 = this->contracts.top();
	this->contracts.pop();
	unsigned int v1 = this->contracts.top();
	this->contracts.pop(); 
	unsigned int v2 = this->contracts.top();
	this->contracts.pop();
	//Idea: check the heads
	perimeters[v2] = p2;
	perimeters[v1] = p1;
	// if head1 is 0, and head2 is 0, blank collapse, skip
	// if head2 = v1, then it is a single edge
	if (head2 == v1) {
		// connect v1 to v2
		indices.push_back(v1);
		indices.push_back(v2);
		edges[v2].insert(v1);
		edges[v1].insert(v2);
		index_count += 2;
		perim_split();
		refresh();
		return;
	}
	if (head1 == 0) {
		if (head2 == 0) {
			perim_split();
			return;
		}
		// if head1 is 0, head2 is non-zero, split generates only one triangle
		else {
			// y = mx + b
			// m = rise/run
			float m = (vertices[v2 * 3 + 1] - vertices[head2 * 3 + 1]) / (vertices[v2 * 3] - vertices[head2 * 3]);
			// b = y-mx
			float b = vertices[v2 * 3 + 1] - m * vertices[v2 * 3];
			// connect v2 to head2
			indices.push_back(v2);
			indices.push_back(head2); 
			edges[v2].insert(head2);
			edges[head2].insert(v2);
			// connect v1 to v2
			indices.push_back(v1);
			indices.push_back(v2);
			edges[v2].insert(v1);
			edges[v1].insert(v2);
			splitter(v1, v2, head1, head2, m, b);
			index_count += 4;
		}
	}
	// else, have both heads be the end vertices of a line
	// get the line equation, check which side the vertices are on, then generate the edges or move according to that
	else {
		// line will be made from head1 and head2
		// y = mx + b
		// m = rise/run
		float m = (vertices[head1 * 3 + 1] - vertices[head2 * 3 + 1]) / (vertices[head1 * 3] - vertices[head2 * 3]);
		// b = y-mx
		float b = vertices[head2 * 3 + 1] - m * vertices[head2 * 3];
		splitter(v1, v2, head1, head2, m, b);
		// connect v2 to head1
		indices.push_back(v2);
		indices.push_back(head1);
		edges[v2].insert(head1);
		edges[head1].insert(v2);

		// connect v2 to head2
		indices.push_back(v2);
		indices.push_back(head2);
		edges[head2].insert(v2);
		edges[v2].insert(head2);
		// connect v1 to v2
		indices.push_back(v1);
		indices.push_back(v2);
		edges[v2].insert(v1);
		edges[v1].insert(v2);
		index_count += 6;
	}
	perim_split();
	refresh();
}

void Polygon::perim_split() {
	//read the perimeter stack
	while (normal_mod.top() != 0) {
		unsigned int mod1 = normal_mod.top();
		normal_mod.pop();
		unsigned int mod2 = normal_mod.top();
		normal_mod.pop();
		if (outer[mod1].count(mod2)) {
			outer[mod1].erase(mod2);
			outer[mod2].erase(mod1);
		}
	}
	normal_mod.pop();
}

bool Polygon::is_above(unsigned int index, float m, float b) {
	return m * vertices[index * 3] + b >= vertices[index * 3 + 1];
}

void Polygon::splitter(unsigned int v1, unsigned int v2, unsigned int head1, unsigned int head2, float m, float b){
	bool v1_more = is_above(v1, m, b);
	// check if any of the vertex that connects to v1 should move to connect to v2
	for (int i = 0; i < index_count; i += 2) {
		int mod = -1;
		if (indices[i] == v1) {
			mod = 0;
		}
		else if (indices[i + 1] == v1) {
			mod = 1;
		}
		//v1 is in i + mod, head2 is in i + (1-mod)
		if (mod >= 0) {
			//if the pair is the head itself, ignore all of this
			if (indices[i + 1 - mod] == head2 || indices[i + 1 - mod] == head1) continue;
			//check if its pair is not on the same side of the line
			//if not, move to v2
			if (is_above(indices[i + 1 - mod], m, b) != v1_more) {
				edges[v2].insert(indices[i + 1 - mod]);
				edges[indices[i + 1 - mod]].erase(v1);
				edges[indices[i + 1 - mod]].insert(v2);
				edges[v1].erase(indices[i + 1 - mod]);
				indices[i + mod] = v2;
			}
		}
	}
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