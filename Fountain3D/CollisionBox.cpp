//
// CollisionBox.cpp
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#include <CollisionBox.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <maths_funcs.h>


using namespace std;

// define static variable
GLuint CollisionBox::_collision_shader;
Mesh* CollisionBox::_mesh;

CollisionBox::CollisionBox(vec3 pos, float width, float height) {
	_pos = pos;
	_width = width;
	_height = height;
	calculateCorners();
	//print(_pos);
	//createVp();
}

CollisionBox::CollisionBox() {

}

CollisionBox::~CollisionBox() {

}

bool CollisionBox::collides(CollisionBox b2) {
	if (_x1 < b2._x2 && 
		_x2 > b2._x1 && 
		_z1 < b2._z2 && 
		_z2 > b2._z1)
	{
		return true;
	}
}

void CollisionBox::calculateCorners() {
	_x1 = _pos.v[0] - (_width/2);
	_z1 = _pos.v[2] - (_height/2);
	_x2 = _pos.v[0] + (_width / 2);
	_z2 = _pos.v[2] + (_height/2);
}

/*void CollisionBox::createVp() {
	float points[] = {
		_x1, 0.0f, _z1,
		_x2, 0.0f, _z1,
		_x1, 0.0f, _z2,
		_x2, 0.0f, _z1,
		_x2, 0.0f, _z2,
		_x1, 0.0f, _z2
	};
	_vp = points;
}*/

void CollisionBox::addShader(GLuint collision_shader) {
	_collision_shader = collision_shader;
}

GLuint CollisionBox::getShader() {
	return CollisionBox::_collision_shader;
}

void CollisionBox::addMesh(Mesh* mesh) {
	_mesh = mesh;
}

void CollisionBox::render() {
	glUseProgram(_collision_shader);
	glBindTexture(GL_TEXTURE_2D, _collision_shader);
	glBindVertexArray(_mesh->getVao());
	glUniformMatrix4fv(_model_mat_location, 1, GL_FALSE, _mod.m);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawArrays(GL_TRIANGLES, 0, _mesh->getPointCount());
}

void CollisionBox::addModelMat(mat4 model_mat) {
	_mod = model_mat;
	_model_mat_location = glGetUniformLocation(_collision_shader, "model");
}

void CollisionBox::move(vec3 new_position) {
	_pos = new_position;
	calculateCorners();
}

void CollisionBox::print() {
	printf("[x1: %.2f, z1: %.2f, x2: %.2f, z2: %.2f]\n", _x1, _z1, _x2, _z2);
}
