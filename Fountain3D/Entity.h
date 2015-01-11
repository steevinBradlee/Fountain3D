//
// Entity.h
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#ifndef __Fountain3D__Entity__
#define __Fountain3D__Entity__

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <maths_funcs.h>
#include <stdlib.h>
#include <vector>
#include <CollisionBox.h>
#include <Mesh.h>
#include <string>

using namespace std;

class Entity {

private:
	string _label;
	Mesh* _mesh;
	mat4 _mod;
	CollisionBox _collision;

public:
	Entity(string label, Mesh* mesh, mat4 mod, CollisionBox collision);
	Entity(string label, Mesh* mesh, mat4 mod);
	~Entity();

	void render(GLuint model_mat_location, GLuint texture, bool render_collision);
	void renderCollision();
	void setModelMat(mat4 new_mod);
	void move(vec3 movement);
	CollisionBox getCollisionBox() { return _collision; }
	string getLabel() { return _label; }
	mat4 getModelMatrix() { return _mod; }
};

#endif