//
// Mesh.h
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#ifndef __Fountain3D__Mesh__
#define __Fountain3D__Mesh__

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <maths_funcs.h>

class Mesh {

private:
	int _point_count;
	GLfloat* _vp;
	GLfloat* _vn;
	GLfloat* _vt;
	GLuint _vp_vbo;
	GLuint _vt_vbo;
	GLuint _vn_vbo;
	GLuint _vao;

	void loadMeshFromFile(const char* file_name);
	void createVertexObjects();
	void createSimpleVertexObjects();

public:
	Mesh(const char* file_name);
	Mesh(float* points);
	~Mesh();

	GLuint getVao() { return _vao; }
	GLuint getPointCount() { return _point_count; }
	void render(GLint model_mat_location, mat4 model_mat);

};

#endif