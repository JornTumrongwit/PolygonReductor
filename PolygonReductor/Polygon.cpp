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
			vertices.push_back(x / 1000.0);
			vertices.push_back(y / 1000.0); 
			vertices.push_back(z / 1000.0);
			vertex_count += 3;
		}
		else if ((c1 == 'i') && (c2 == ' ')) {
			fscanf_s(fp, "%d %d", &ix, &iy);
			indices.push_back(ix);
			indices.push_back(iy);
			edges[ix].insert(iy);
			edges[iy].insert(ix);
			index_count += 2;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->vertex_count, &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->index_count, &indices[0], GL_STATIC_DRAW);

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
	std::vector<unsigned int> deletes;
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
					if (vert == v1) this->contracts.push(endvert);
					else this->contracts.push(vert);
					vertex_head++;
				}
			}
		}
		//remove this vertex from adjacency
		edges[vert].erase(v2);
	}
	while (vertex_head < 2) {
		this->contracts.push(0); //signify single triangle collapsed
		vertex_head++;
	}
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
				deletes.push_back(i);
				deletes.push_back(i+1);
			}
		}
	}
	unsigned int down = 0;
	for (unsigned int i = 0; i < deletes.size(); i++) {
		unsigned int index = deletes[i] - down;
		indices.erase(std::next(indices.begin(), index));
		down++;
		index_count--;
	}
	for (unsigned int i = 0; i < index_count; i++) {
		std::cout << indices[i] << "\n";
	}
	refresh();
}

void Polygon::Split() {
	if (this->contracts.size() <= 0) return;
	unsigned int head1 = this->contracts.top();
	this->contracts.pop();
	unsigned int head2 = this->contracts.top();
	this->contracts.pop();
	unsigned int v1 = this->contracts.top();
	this->contracts.pop(); 
	unsigned int v2 = this->contracts.top();
	this->contracts.pop();
	std::cout << "SPLITTING " << v2 << " OUT OF " << v1 << " WITH HEADS "<< head1 << " " << head2 << "\n";
	//Idea: check the heads
	// if head1 is 0, and head2 is 0, blank collapse, skip
	if (head1 == 0) {
		if (head2 == 0) return;
		// if head1 is 0, head2 is non-zero, split generates only one triangle
		else {
			bool addself = false;//small flag that its own edge was formed
			for (unsigned int i = 0; i < this->index_count; i += 2) {
				if (indices[i] == 0 || indices[i + 1] == 0) {
					
				}
			}
		}
	}
	// else, have both heads be the end vertices of a line
	// get the line equation, check which side the vertices are on, then generate the edges and move according to that
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