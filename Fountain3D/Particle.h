//
// Particle.h
// Fountain3D
//
// Created by Stephen Bradley on 03/12/2014
//

#ifndef __Fountain3D__Particle__
#define __Fountain3D__Particle__

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <maths_funcs.h>
#include <Mesh.h>

using namespace std;

class Particle {

private:
	Mesh* _mesh;
	mat4 _mod;
	vec3 _position;
	int _lifespan;
	vec3 _speed;

public:
	static const GLuint model_mat_location;
	static const int FRAMES_TO_LIVE;
	static const float X_SPEED;
	static const float Z_SPEED;
	static const float Y_SPEED;
	static const float GRAVITY;

	Particle();
	Particle(Mesh* mesh, vec3 position);
	~Particle();

	int getLifespan() { return _lifespan; }
	vec3 getPosition() { return _position; }

	void update();
	void render(GLuint model_mat_location, GLuint texture);
	void revive(vec3 position);

};

#endif