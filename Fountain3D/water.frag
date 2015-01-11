#version 400

in vec3 position_eye, normal_eye;
in vec2 texture_coordinates;

uniform mat4 view_mat;
uniform sampler2D basic_texture;

// ------- fixed point light properties
vec3 light_position_world  = vec3 (0.0, 0.0, 2.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// ------- surface reflectance
vec3 Ks = vec3 (0.2, 0.2, 0.2); // fully reflect specular light
vec3 Kd = vec3 (1.0, 1.0, 1.0); // white diffuse surface reflectance
//vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (0.2, 0.2, 0.2); // fully reflect ambient light
float specular_exponent = 2.0; // specular 'power'

out vec4 fragment_colour;

void main () {
	// ------- ambient intensity
	vec3 Ia = La * Ka;

	// ------- diffuse intensity
	// raise light position to eye space
	vec3 light_position_eye = vec3 (view_mat * vec4 (light_position_world, 1.0));
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	float dot_prod = dot (direction_to_light_eye, normal_eye);
	dot_prod = max (dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
	
	// ------- specular intensity
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	
	// ------- blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, normal_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	specular_factor = specular_factor; //* 0.00001;
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
	
	// ------- texture sampling
	vec4 texel = texture (basic_texture, texture_coordinates);

	// ------- distance fog
	vec3 fog_colour = vec3(0, 0, 0);
	// 3, 10
	float min_fog_radius = 1.0;
	float max_fog_radius = 14.0;

	float dist = length(-position_eye); // distance from camera to point
	// get a fog factor (thickness of fog) based on the distance
	float fog_fac = (dist - min_fog_radius) / (max_fog_radius - min_fog_radius);
	// constrain the fog factor between 0 and 1
	fog_fac = clamp (fog_fac, 0.0, 1.0);

	// ------- final fragment colour without fog
	fragment_colour = vec4 (Is + Id + Ia, 1.0) * texel;

	// ------- final fragment colour with fog
	fragment_colour.rgb = mix(fragment_colour.rgb, fog_colour, fog_fac);
	fragment_colour.w = 0.6;
}