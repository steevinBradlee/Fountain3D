//
// Mesh.cpp
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#include <Mesh.h>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <maths_funcs.h>
#include <obj_parser.h>
#include <stb_image.h>
#include <obj_parser.h>
#include <stdlib.h>

Mesh::Mesh(const char* file_name) {
	loadMeshFromFile(file_name);
	createVertexObjects();
}

Mesh::Mesh(float* points) {
	_point_count = sizeof(points) / sizeof(float);
	_vp = points;
	createSimpleVertexObjects();
}

Mesh::~Mesh() {

}

void Mesh::loadMeshFromFile(const char* file_name) {
	int point_count = 0;
	GLfloat* vp = NULL;
	GLfloat* vn = NULL;
	GLfloat* vt = NULL;
	if (!load_obj_file(file_name, vp, vt, vn, point_count)) {
		fprintf(stderr, "ERROR: could not find mesh file...\n");
	}
	_point_count = point_count;
	_vp = vp;
	_vn = vn;
	_vt = vt;
}

void Mesh::createVertexObjects() {

	GLuint pos_vbo;
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof (float) * 3 * _point_count, _vp, GL_STATIC_DRAW);
	_vp_vbo = pos_vbo;

	GLuint normals_vbo;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof (float) * 3 * _point_count, _vn, GL_STATIC_DRAW);
	_vn_vbo = normals_vbo;

	GLuint tex_vbo;
	glGenBuffers(1, &tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof (float) * 2 * _point_count, _vt, GL_STATIC_DRAW);
	_vt_vbo = tex_vbo;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	_vao = vao;
}

void Mesh::createSimpleVertexObjects() {

	GLuint pos_vbo;
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof (float) * 18, _vp, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof (float)* 3 * _point_count, _vp, GL_STATIC_DRAW);
	_vp_vbo = pos_vbo;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	_vao = vao;
}

void Mesh::render(GLint model_mat_location, mat4 model_mat) {
	glBindVertexArray(_vao);
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, model_mat.m);
	glDrawArrays(GL_TRIANGLES, 0, _point_count);
}