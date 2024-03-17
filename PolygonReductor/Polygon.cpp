#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Polygon.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <queue>

Polygon::Polygon() {
	//haha get blank'd on
}

// OBJ file parser function. Used for loading the graph file.
void Polygon::parse(const char* filepath) {
	FILE* fp;
	int c1, c2;
	float x, y, z;
	unsigned int ix, iy;

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
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			this->vertices[this->vertex_count] = x / 1000.0;
			this->vertex_count++;
			this->vertices[this->vertex_count] = y / 1000.0;
			this->vertex_count++;
			this->vertices[this->vertex_count] = z / 1000.0;
			this->vertex_count++;
		}
		else if ((c1 == 'i') && (c2 == ' ')) {
			fscanf_s(fp, "%d %d", &ix, &iy);
			this->indices[this->index_count] = ix;
			this->index_count++;
			this->indices[this->index_count] = iy;
			this->index_count++;
			edges[ix].insert(iy);
			edges[iy].insert(ix);
		}
	}

	fclose(fp); // Finished parsing
}

Polygon::Polygon(const char* filepath) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	this->vertex_count = 0;
	this->index_count = 0;
	parse(filepath);
}

void Polygon::Init() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->vertex_count, this->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->index_count, this->indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

// Contract the specified edge by moving v2 into v1
void Polygon::Contract(unsigned int v1, unsigned int v2) {
	//Idea: get a set of all the adjacents of v2, get the list of edges that need to be added
	//		replace the edges of v2 in the list with the edges in that list, then the rest
	//      of the edges will be set to 0,0, which is to be ignored
	this->contracts.push(v2);
	this->contracts.push(v1);
	unsigned int vertex_head = 0;
	std::set<unsigned int> adj = edges[v2];
	std::vector<unsigned int> to_add;
	for (auto vert: adj) {
		for (auto endvert : adj) {
			//check only one way
			//if not same vertex and not yet adjacent
			if (vert < endvert && (vert == v1 || endvert == v1)) {
				if (!edges[vert].count(endvert)) {
					to_add.push_back(vert);
					to_add.push_back(endvert);
					edges[endvert].insert(vert);
					edges[vert].insert(endvert);
				}
				else {
					//this is a triangle head
					std::cout << vert << endvert << "\n";
					this->contracts.push(endvert);
					vertex_head++;
				}
			}
		}
		//remove this vertex from adjacency
		edges[vert].erase(v2);
	}
	if (vertex_head < 2) this->contracts.push(0); //signify single triangle collapsed
	edges.erase(v2);
	unsigned int add_index = 0;
	for (unsigned int i = 0; i < this->index_count; i+=2) {
		if (indices[i] == v2 || indices[i+1] == v2) {
			if (add_index < to_add.size()) {
				indices[i] = to_add[add_index];
				indices[i + 1] = to_add[add_index + 1];
				add_index += 2;
			}
			else {
				indices[i] = 0;
				indices[i + 1] = 0;
			}
		}
		std::cout << indices[i] << " " << indices[i + 1] << "\n";
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->index_count, this->indices, GL_STATIC_DRAW);
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