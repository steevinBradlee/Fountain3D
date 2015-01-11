//
// CollisionBox.h
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#ifndef __Fountain3D__CollisionBox__
#define __Fountain3D__CollisionBox__

#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <gl_utils.h>
#include <maths_funcs.h>
#include <mesh.h>

class CollisionBox {

private:
	vec3 _pos;
	float _width;
	float _height;

	float _x1;
	float _z1;
	float _x2;
	float _z2;
	//float* _vp;

	mat4 _mod;
	GLuint _model_mat_location;

	// add mesh to collision box so that it can 
	// be rendered for debugging purposes

	static GLuint _collision_shader;
	static Mesh* _mesh;

	void createVp();

public:
	CollisionBox(vec3 pos, float width, float height);
	CollisionBox();
	~CollisionBox();

	void addModelMat(mat4 model_mat);

	bool collides(CollisionBox b2);

	vec3 getPos() { return _pos; }
	
	static void addShader(GLuint collision_shader);
	static GLuint getShader();

	static void addMesh(Mesh* mesh);
	static const float* getVp();
	void render();
	void calculateCorners();

	void move(vec3 new_position);
	void print();

	float getWidth() { return _width;  }
	float getHeight() { return _height; }
	// finish writing render method
	// need to add a model matrix in order 
	// to place collision box at same place as mesh
};

#endif