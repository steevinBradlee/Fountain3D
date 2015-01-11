//
// Particle.cpp
// Fountain3D
//
// Created by Stephen Bradley on 03/12/2014
//

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <maths_funcs.h>
#include <Particle.h>
#include <math.h>
#include <random>

using namespace std;

const int Particle::FRAMES_TO_LIVE = 300;
const float Particle::X_SPEED = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.01;
const float Particle::Z_SPEED = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.01;
const float Particle::Y_SPEED = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.01;
const float Particle::GRAVITY = 0.0002f;

Particle::Particle() {

}

Particle::Particle(Mesh* mesh, vec3 position) {
	_mesh = mesh;
	_position = position;
	_lifespan = FRAMES_TO_LIVE;
	_mod = translate(identity_mat4(), position);
	_speed = vec3(X_SPEED, Y_SPEED, Z_SPEED);
}

Particle::~Particle() {

}

void Particle::update() {

	// Calculate the new Y speed of the particle
	_speed.v[1] = _speed.v[1] - GRAVITY;

	// Update the position of the particle by the speed it's moving at
	_position += _speed;
	_mod = translate(identity_mat4(), _position);

	// Decrease the frames the particle will live for by 1
	_lifespan = _lifespan - 1; 
	
}

void Particle::render(GLuint model_mat_location, GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(_mesh->getVao());
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, _mod.m);
	glDrawArrays(GL_TRIANGLES, 0, _mesh->getPointCount());
}

void Particle::revive(vec3 position) {
	bool neg = rand() % 2 == 1;
	float r_x, r_z;
	if (neg == true) {
		r_x = -static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.009;
		r_z = -static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.009;
	}
	else {
		r_x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.009;
		r_z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.009;
	}
	float r_y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)* 0.01;
	//float time = sinf((float)glfwGetTime());
	_position = position;
	_lifespan = FRAMES_TO_LIVE;
	float x_speed = r_x;
	float y_speed = r_y;
	float z_speed = r_z;
	_speed = vec3(x_speed, y_speed, z_speed);
}