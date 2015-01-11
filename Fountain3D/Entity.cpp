//
// Entity.cpp
// Fountain3D
//
// Created by Stephen Bradley on 18/11/2014
//

#include <stdlib.h>
#include <vector>
#include <Entity.h>

using namespace std;

// used for simple entities with only one collision box; trees, pillars.
Entity::Entity(string label, Mesh* mesh, mat4 mod, CollisionBox collision) {
	_label = label;
	_mesh = mesh;
	_mod = mod;
	_collision = collision;
	_collision.addModelMat(mod);
}

// used for complex entities with more than one collision box; walls.
Entity::Entity(string label, Mesh* mesh, mat4 mod) {
	_label = label;
	_mesh = mesh;
	_mod = mod;
}

Entity::~Entity() {

}

void Entity::render(GLuint model_mat_location, GLuint texture, bool render_collision) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(_mesh->getVao());
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, _mod.m);
	glDrawArrays(GL_TRIANGLES, 0, _mesh->getPointCount());
	if (render_collision) {
		renderCollision();
	}
}

void Entity::setModelMat(mat4 new_mod) {
	_mod = new_mod;
}

void Entity::renderCollision() {
	_collision.render();
}

void Entity::move(vec3 movement) {
	_mod = translate(_mod, movement);
}
