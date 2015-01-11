#version 400

in vec3 vertex_position;

uniform mat4 model;

void main() {
	gl_Position = model * vec4(vertex_position, 1.0);
}
