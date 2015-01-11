#version 400

in vec3 position_eye, normal_eye;
in vec2 texture_coordinates;

uniform mat4 view_mat;
uniform sampler2D basic_texture;
uniform float opacity;

out vec4 fragment_colour;

void main () {
	
	// ------- texture sampling
	vec4 texel = texture (basic_texture, texture_coordinates);


	// ------- final fragment 
	fragment_colour = texel;
	fragment_colour.w = opacity;

}