#version 400

layout (location = 0) in vec3 vp;
layout (location = 1) in vec3 vn;
layout (location = 2) in vec2 vt; 

uniform mat4 model, view, proj;

out vec3 position_eye, normal_eye;
out vec2 texture_coordinates;

void main() {
	position_eye = vec3(view * model * vec4(vp, 1.0));
	normal_eye = vec3(view * model * vec4(vn, 0.0));
	gl_Position = proj * view * model * vec4 (vp, 1.0);
	texture_coordinates = vt;
}