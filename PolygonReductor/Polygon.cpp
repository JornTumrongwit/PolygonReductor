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
	std::cout << "Start edge is: " << starter << "\n";
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
	printedge();
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

// Contract the specified edge
bool Polygon::collapse(unsigned int edge){
	//check if this edge is collapsed. If so, return false
	if (collapsed.count(edge)) {
		std::cout << "Edge: " << edge << " already collapsed.\n";
		return false;
	}
	else {
		//push all the related triangle edges
		collapsed.insert(edge);
		collapsed.insert(prev(edge));
		collapsed.insert(next(edge));
		int eng = twin(edge);
		if (eng != -1) {
			collapsed.insert(eng);
			collapsed.insert(prev(eng));
			collapsed.insert(next(eng));
		}
	}
	//moving the starting edge (where it moves doesn't matter as long as it doesn't start at the deleted triangle
	//cannot use: this edge, its twin, this prev, this next
	if (starter == edge || starter == prev(edge) || starter == next(edge)){
		starter = twin(prev(edge));
		if (starter == -1) {
			starter = twin(next(edge));
			if (starter == -1) {
				//just use something from the other triangle and pray for the best
				if (twin(edge) == -1) {
					//giveup and cry
					std::cout << "DON'T COLLAPSE THIS I DON'T WANNA DIIIIIIIIIIIIIIIIIIIE!";
					return false;
				}
				starter = twin(edge);
			}
		}
	}
	if (starter == twin(edge) || starter == prev(twin(edge)) || starter == next(twin(edge))) {
		starter = twin(prev(twin(edge)));
		if (starter == -1) {
			starter = twin(next(twin(edge)));
			if (starter == -1) {
				std::cout << "DON'T COLLAPSE THIS I DON'T WANNA DIIIIIIIIIIIIIIIIIIIE!";
				return false;
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
	//spin counter clockwise
	int start = prev(edge);
	int the = start;
	bool moved = false;
	std::cout << "GOING COUNTERCLOCKWISE\n";
	if (twin(the) != -1) {
		moved = true;
		start = prev(twin(start));
		the = start;
		std::cout << "STARTER IS: " << start << "\n";
		std::cout << "MODIFYING: " << the << " MOVING " << d_edge[the * 2] << " AS SIGNALED BY " << the << "\n";
		d_edge[the * 2] = newvtx;
		if (twin(the) != -1) the = prev(twin(the));
		while (the != start) {
			std::cout << "MODIFYING: " << the << " MOVING " << d_edge[the * 2] << " AS SIGNALED BY " << the << "\n";
			d_edge[the* 2] = newvtx;
			if (twin(the) != -1) the = prev(twin(the));
			else break;
		}
		std::cout << "STOPPED BECAUSE: " << (twin(the) != -1) << " " << (the != start) << "\n";
	}
	std::cout << "STARTING MOVING CLOCKWISE\n";
	//spin clockwise if not stopping because of start, and that edge can rotate the other way
	if ((the != start || !moved) && (twin(edge) != -1)) {
		start = next(twin(edge));
		the = start;
		std::cout << "STARTING AT: " << start << "\n";
		if (twin(the) != -1) {
			std::cout << "MODIFYING: " << twin(the) << " MOVING " << d_edge[twin(the) * 2] << " AS SIGNALED BY " << the << "\n";
			d_edge[twin(the) * 2] = newvtx;
			the = next(twin(edge));
		}
	}
	construct();
	refresh();
	printedge();
	return true;
}

bool Polygon::boundary(int edge) {
	return (twin(edge) == -1);
}

void Polygon::split() {
	if (contracts.empty()) return;
	int edge = contracts.top();
	//remove all the collapsed status
	collapsed.erase(edge);
	collapsed.erase(prev(edge));
	collapsed.erase(next(edge));
	int eng = twin(edge);
	if (eng != -1) {
		collapsed.erase(eng);
		collapsed.erase(prev(eng));
		collapsed.erase(next(eng));
	}
	contracts.pop();
	if (twin(prev(edge)) >= 0) d_edge[twin(prev(edge)) * 2 + 1] = prev(edge);
	if (twin(next(edge)) >= 0) d_edge[twin(next(edge)) * 2 + 1] = next(edge);
	if (eng != -1) {
		if (twin(prev(eng)) >= 0) d_edge[twin(prev(eng)) * 2 + 1] = prev(eng);
		if (twin(next(eng)) >= 0) d_edge[twin(next(eng)) * 2 + 1] = next(eng);
	}
	//changing the vertex
	unsigned int newvtx = d_edge[prev(edge) * 2];
	std::cout << "RETURN VERTEX: " << newvtx << "\n\n";
	//spin counter clockwise
	int start = prev(edge);
	int the = start;
	bool moved = false;
	std::cout << "GOING COUNTERCLOCKWISE\n";
	if (twin(the) != -1) {
		moved = true;
		start = prev(twin(start));
		the = start;
		std::cout << "STARTER IS: " << start << "\n";
		std::cout << "MODIFYING: " << the << " MOVING " << d_edge[the * 2] << " AS SIGNALED BY " << the << "\n";
		d_edge[the * 2] = newvtx;
		if (twin(the) != -1) the = prev(twin(the));
		while (the != start) {
			std::cout << "MODIFYING: " << the << " MOVING " << d_edge[the * 2] << " AS SIGNALED BY " << the << "\n";
			d_edge[the * 2] = newvtx;
			if (twin(the) != -1) the = prev(twin(the));
			else break;
		}
		std::cout << "STOPPED BECAUSE: " << (twin(the) != -1) << " " << (the != start) << "\n";
	}
	std::cout << "STARTING MOVING CLOCKWISE\n";
	//spin clockwise if not stopping because of start, and that edge can rotate the other way
	if ((the != start || !moved) && (twin(edge) != -1)) {
		start = next(twin(edge));
		the = start;
		std::cout << "STARTING AT: " << start << "\n";
		if (twin(the) != -1) {
			std::cout << "MODIFYING: " << twin(the) << " MOVING " << d_edge[twin(the) * 2] << " AS SIGNALED BY " << the << "\n";
			d_edge[twin(the) * 2] = newvtx;
			the = next(twin(edge));
		}
	}
	construct();
	refresh();
	printedge();
}

void Polygon::printedge() {
	for (int i = 0; i * 2 < d_edge.size(); i++) {
		std::cout << i << ": " << d_edge[i*2] << " " << d_edge[i*2+1] << "\n";
	}
	std::cout << "\n";
}

/**
* Get the normal of the vertex
*/
std::vector<float> Polygon::get_normal(unsigned int v1, unsigned int v2) {
	std::vector<float> normal(2);

	// y | -x (should I swap)
	normal[1] = -(vertices[v1 * 3] - vertices[v2 * 3]);
	normal[0] = vertices[v1 * 3 + 1] - vertices[v2 * 3 + 1];

	return normal;
}

/**
* Get the cost for collapsing v2 into v1 (v2 -> v1)
*/
unsigned int Polygon::get_cost_QEM(unsigned int v1, unsigned int v2) {
	/**
	* \Sigma^{m}_{i=1}(n_i \cdot v + d_i)^2
	* n = normal 
	* d = offset from origin
	* v = new vertex location
	*/
	unsigned int cost = 0;

	// get union of edges associated with v1,v2
	std::set<unsigned int> nbrs;
	nbrs.insert(edges.at(v1).begin(), edges.at(v1).end());
	nbrs.insert(edges.at(v2).begin(), edges.at(v2).end());
	nbrs.erase(v1);

	for (auto v : nbrs) {
		// Get normal of the plane 
		std::vector<float> n = get_normal(v1, v);

		// Get the offset from origin of the plane using midpoint
		int mp_x = (vertices[v1 * 3 + 1] + vertices[v * 3 + 1]) / 2;
		int mp_y = (vertices[v1 * 3] + vertices[v * 3]) / 2;
		unsigned int offset = std::sqrt(mp_x * mp_x + mp_y * mp_y);

		// Get n /cdot v
		// ignore this if the vertex is not on the boundary.
		int nv = (n[0]* vertices[v1 * 3] + n[1]* vertices[v1 * 3 + 1]);
		cost += std::pow((offset + nv), 2);
	}

	return cost;
}

/**
* Get the minimum cost pair to collapse in the next iteration.
*/
std::vector<unsigned int> Polygon::get_min_cost_QEM() {
	std::vector<unsigned int> min_pair(2);
	unsigned int min_cost = UINT_MAX;
	unsigned int current_cost;
	for (auto const& x: edges) {
		for (auto v2 : x.second ) {
			current_cost = get_cost_QEM(x.first, v2);
			if (current_cost < min_cost) {
				min_cost = current_cost;
				min_pair[0] = x.first;
				min_pair[1] = v2;
			}
		}	
	}
	return min_pair;
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