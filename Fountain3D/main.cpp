#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_FAILURE_USERMSG
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <stdio.h>
#include <iostream>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"
#include <maths_funcs.h>
#include <stdlib.h>
#include <stb_image.h>
#include <obj_parser.h>
#include <vector>
#include <Mesh.h>
#include <CollisionBox.h>
#include <Entity.h>
#include <gl_utils.h>
#include <Particle.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <irrklang.h>
#include <sstream>

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

using namespace std;
using namespace irrklang;


//******************************************
// Globals
//******************************************

int g_gl_width = 1366;
int g_gl_height = 768;
GLFWwindow* g_window = NULL;

mat4 view_mat;
mat4 proj_mat;

vec3 cam_pos(0.0f, 2.0f, 5.0f);

vector<Entity> collidable_entities;

int number_of_crystals_found = 0;

//******************************************
// Entity positions
//******************************************
vec3 ground_position;
vec3 arm_position;
vec3 torch_position;
vec3 menu_position;
vec3 fountain_position;
vec3 particle_fountain_position;
vector<vec3> tree_positions;
vector<vec3> wall1_positions;
vector<vec3> wall2_positions;
vector<vec3> wall3_positions;
vector<vec3> pillar2_positions;
vector<vec3> crystal_positions;
vector<vec3> statue_positions;



//******************************************
// Additional functions
//******************************************

// function for flipping an image
void flip_image(unsigned char* image_data, int x, int y, int n) {
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
}

// function for loading a texture from a file
bool load_texture(const char* file_name, GLuint* tex) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// check to see if image is power of two
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
			);
	}
	flip_image(image_data, x, y, n);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
		);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	return true;
}

// function for splitting a string at whitespace into tokens
unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	unsigned int pos = txt.find(ch);
	unsigned int initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos + 1));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

// function for reading text file containing Entity positions arranged in Blender
int readSceneFile(const char* file_name) {
	string line;
	ifstream myfile(file_name);
	if (myfile.is_open()) {

		while (getline(myfile, line)) {
			vector<string> tokens;
			split(line, tokens, ' ');

			if (tokens.size() != 4) {
				cout << "WE GOT A PROBLEM";
			}

			else {

				float x = ::atof(tokens[1].c_str());
				// swap y and z values
				float z = ::atof(tokens[2].c_str());
				float y = ::atof(tokens[3].c_str());
				// pillar
				if (tokens[0].substr(0, 2) == "Co") {
					pillar2_positions.push_back(vec3(x, y, z));
				}
				// tree
				else if (tokens[0].substr(0, 2) == "Cy") {
					tree_positions.push_back(vec3(x, y, z));
				}
				// statue
				else if (tokens[0].substr(0, 2) == "St") {
					statue_positions.push_back(vec3(x, y, z));
				}
				// plane
				else if (tokens[0].substr(0, 2) == "Pl") {
					ground_position = vec3(x, y, z);
				}
				// crystal
				else if (tokens[0].substr(0, 2) == "Ic") {
					crystal_positions.push_back(vec3(x, y, z));
				}
				// wall2
				if (tokens[0].substr(0, 5) == "wall2") {
					wall2_positions.push_back(vec3(x, y, z));
				}
			}
		}
		myfile.close();
	}

	else {
		cout << "Unable to open file";
	}

	return 0;

}

int main() {

	//******************************************
	// Set up glfw window 
	//******************************************

	const GLubyte* renderer;
	const GLubyte* version;

	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	g_window = glfwCreateWindow(g_gl_width, g_gl_height, "fountain3D", NULL, NULL);
	//g_window = glfwCreateWindow(g_gl_width, g_gl_height, "fountain3D", glfwGetPrimaryMonitor(), NULL);
	if (!g_window) {
		fprintf(stderr, "ERROR: opening OS window\n");
		return 1;
	}

	glfwMakeContextCurrent(g_window);

	glewExperimental = GL_TRUE;
	glewInit();

	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);



	//******************************************
	// Set up virtual camera matrices
	//******************************************

#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0

	// input variables
	float near = 0.1f; // near clipping plane
	float far = 100.0f; // far clipping plane
	float fovy = 67.0f; // field of view along y plane
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio

	// projection matrix
	proj_mat = perspective(fovy, aspect, near, far);

	float cam_speed = 4.0f; // 1 unit per second
	float cam_heading_speed = 50.0f; // 30 degrees per second
	float cam_heading = 0.0f; // y-rotation in degrees

	// camera translation matrix
	mat4 T = translate(
		identity_mat4(), vec3(-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2])
		);

	//camera rotation matrix
	mat4 R;
	versor Q = quat_from_axis_deg(-cam_heading, 0.0f, 1.0f, 0.0f);
	R = quat_to_mat4(Q);

	// camera view matrix
	view_mat = R * T;

	// directional vectors for camera
	vec4 fwd(0.0f, 0.0f, -1.0f, 0.0f);
	vec4 rgt(1.0f, 0.0f, 0.0f, 0.0f);
	vec4 up(0.0f, 1.0f, 0.0f, 0.0f);

	// CAMERA CONTROL WITH MOUSE
	double mouse_x = g_gl_width / 2;
	double mouse_y = g_gl_height / 2;
	glfwSetCursorPos(g_window, mouse_x, mouse_y);

	// camera collision box
	float camera_collision_size = 0.5f;
	float x = cam_pos.v[0];
	float z = cam_pos.v[2];
	vec3 cam_col_position = vec3(x, 0.0f, z);
	CollisionBox camera_collision = CollisionBox(cam_col_position, camera_collision_size, camera_collision_size);

	// static view matrix for arm and torch
	mat4 static_V = look_at(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f),
		vec3(0.0f, 1.0f, 0.0f));



	//******************************************
	// Set up shaders
	//******************************************

	// general purpose shader with lighting
	GLuint shader_programme1 = create_programme_from_files(
		"shader.vert", "shader.frag");

	// shader used for collision
	GLuint shader_programme2 = create_programme_from_files(
		"collision.vert", "collision.frag");
	CollisionBox::addShader(shader_programme2);

	// torch shader
	GLuint shader_programme_torch = create_programme_from_files(
		"torch.vert", "torch.frag");

	// particle shader
	GLuint shader_programme_particle = create_programme_from_files(
		"particle.vert", "particle.frag");

	// water shader
	GLuint shader_programme_water = create_programme_from_files(
		"water.vert", "water.frag");

	// black fade shader
	GLuint shader_programme_black = create_programme_from_files(
		"black.vert", "black.frag");

	// white arm shader
	GLuint shader_programme_white = create_programme_from_files(
		"white_arm.vert", "white_arm.frag");

	// ------- shader for static entities, with lighting and fog
	GLint model_mat_location_entity = glGetUniformLocation(shader_programme1, "model");
	GLint view_mat_location_entity = glGetUniformLocation(shader_programme1, "view");
	GLint proj_mat_location_entity = glGetUniformLocation(shader_programme1, "proj");

	glUseProgram(shader_programme1);
	glUniformMatrix4fv(view_mat_location_entity, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(proj_mat_location_entity, 1, GL_FALSE, proj_mat.m);

	// ------- shader for water in fountain
	GLint model_mat_location_water = glGetUniformLocation(shader_programme_water, "model");
	GLint view_mat_location_water = glGetUniformLocation(shader_programme_water, "view");
	GLint proj_mat_location_water = glGetUniformLocation(shader_programme_water, "proj");

	glUseProgram(shader_programme_water);
	glUniformMatrix4fv(view_mat_location_water, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(proj_mat_location_water, 1, GL_FALSE, proj_mat.m);

	// ------- shader for particles
	GLint model_mat_location_particle = glGetUniformLocation(shader_programme_particle, "model");
	GLint view_mat_location_particle = glGetUniformLocation(shader_programme_particle, "view");
	GLint proj_mat_location_particle = glGetUniformLocation(shader_programme_particle, "proj");

	glUseProgram(shader_programme_particle);
	glUniformMatrix4fv(view_mat_location_particle, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(proj_mat_location_particle, 1, GL_FALSE, proj_mat.m);

	// ------- shader for torch and arm
	GLint model_mat_location_torch = glGetUniformLocation(shader_programme_torch, "model");
	GLint view_mat_location_torch = glGetUniformLocation(shader_programme_torch, "view");
	GLint proj_mat_location_torch = glGetUniformLocation(shader_programme_torch, "proj");

	glUseProgram(shader_programme_torch);
	glUniformMatrix4fv(view_mat_location_torch, 1, GL_FALSE, static_V.m);
	glUniformMatrix4fv(proj_mat_location_torch, 1, GL_FALSE, proj_mat.m);

	// ------- shader for black fade from and to menu
	float black_fade_opacity = 1.0f;
	GLint model_mat_location_black = glGetUniformLocation(shader_programme_black, "model");
	GLint view_mat_location_black = glGetUniformLocation(shader_programme_black, "view");
	GLint proj_mat_location_black = glGetUniformLocation(shader_programme_black, "proj");
	GLint opacity_int_location_black = glGetUniformLocation(shader_programme_black, "opacity");

	glUseProgram(shader_programme_black);
	glUniformMatrix4fv(view_mat_location_black, 1, GL_FALSE, static_V.m);
	glUniformMatrix4fv(proj_mat_location_black, 1, GL_FALSE, proj_mat.m);
	glUniform1f(opacity_int_location_black, black_fade_opacity);

	// ------- shader for white arm which fades as player health runs out
	float arm_opacity = 1.0f;
	GLint model_mat_location_white = glGetUniformLocation(shader_programme_white, "model");
	GLint view_mat_location_white = glGetUniformLocation(shader_programme_white, "view");
	GLint proj_mat_location_white = glGetUniformLocation(shader_programme_white, "proj");
	GLint opacity_int_location_white = glGetUniformLocation(shader_programme_white, "opacity");

	glUseProgram(shader_programme_white);
	glUniformMatrix4fv(view_mat_location_white, 1, GL_FALSE, static_V.m);
	glUniformMatrix4fv(proj_mat_location_white, 1, GL_FALSE, proj_mat.m);
	glUniform1f(opacity_int_location_white, arm_opacity);



	//******************************************
	// Load meshes from object file
	//******************************************

	Mesh ground_mesh = Mesh("ground.obj");
	Mesh tree_mesh = Mesh("tree_tall.obj");
	Mesh wall2_mesh = Mesh("wall2.obj");
	Mesh pillar2_mesh = Mesh("pillar2.obj");
	Mesh crystal_mesh = Mesh("crystal.obj");
	Mesh arm_mesh = Mesh("arm5.obj");
	Mesh particle_mesh = Mesh("particle1.obj");
	Mesh fountain_mesh = Mesh("fountain3.obj");
	Mesh statue_mesh = Mesh("statue.obj");
	Mesh torch_mesh = Mesh("torch.obj");
	Mesh water_mesh = Mesh("water.obj");
	Mesh menu_mesh = Mesh("menu.obj");



	//******************************************
	// Load textures
	//******************************************

	GLuint ground_tex, tree_tex, wall1_tex, wall2_tex,
		pillar2_tex, white_crystal_tex, particle_tex, marble_tex,
		torch_tex, water_tex, menu1_tex, menu2_tex, menu_black_tex,
		credits1_tex, credits2_tex, black_crystal_tex;

	glActiveTexture(GL_TEXTURE0);
	assert(load_texture("grass_texture.png", &ground_tex));
	assert(load_texture("tree_texture.png", &tree_tex));
	assert(load_texture("wall2_texture.png", &wall2_tex));
	assert(load_texture("pillar2_texture.png", &pillar2_tex));
	assert(load_texture("white_crystal_texture.png", &white_crystal_tex));
	assert(load_texture("black_crystal_texture.png", &black_crystal_tex));
	assert(load_texture("blue_texture.png", &particle_tex));
	assert(load_texture("marble_texture.png", &marble_tex));
	assert(load_texture("torch_texture.png", &torch_tex));
	assert(load_texture("water_texture.png", &water_tex));
	assert(load_texture("menu1_texture.png", &menu1_tex));
	assert(load_texture("menu2_texture.png", &menu2_tex));
	assert(load_texture("credits1_texture.png", &credits1_tex));
	assert(load_texture("credits2_texture.png", &credits2_tex));
	assert(load_texture("menu_black_texture.png", &menu_black_tex));



	//******************************************
	// Create Entities
	//******************************************

	readSceneFile("scene_layout2.txt");

	// ------- ground
	ground_position = vec3(0.0f, 0.0f, 0.0f);
	vec3 g_pos;
	vector<mat4> ground_mod_mats;
	for (int i = -4; i < 5; i++) {
		for (int j = -4; j < 5; j++) {
			float x = 10 * i;
			float y = 0.0f;
			float z = 10 * j;
			g_pos = vec3(x, y, z);
			ground_mod_mats.push_back(translate(identity_mat4(), g_pos));
			print(g_pos);
		}
	}
	vector<Entity> ground_tiles;
	for (int i = 0; i < ground_mod_mats.size(); i++) {
		ground_tiles.push_back(Entity("ground", &ground_mesh, ground_mod_mats[i]));
	}

	// ------- trees
	vector<mat4> tree_mod_mats;
	for (int i = 0; i < tree_positions.size(); i++) {
		tree_mod_mats.push_back(translate(identity_mat4(), tree_positions[i]));
		tree_mod_mats[i] = translate(tree_mod_mats[i], vec3(0.0f, 0.5f, 0.0f));
		tree_mod_mats[i] = local_rotate_y(tree_mod_mats[i], i * 20);
	}
	vector<Entity> trees;
	float tree_collision_size = 1.5f;
	for (int i = 0; i < tree_positions.size(); i++) {
		float x = tree_positions[i].v[0];
		float z = tree_positions[i].v[2];
		vec3 col_position = vec3(x, 0.0f, z);
		CollisionBox c = CollisionBox(col_position, tree_collision_size, tree_collision_size);
		c.addModelMat(translate(identity_mat4(), col_position));
		Entity t = Entity("tree", &tree_mesh, tree_mod_mats[i], c);
		collidable_entities.push_back(t);
		trees.push_back(t);
	}

	// ------- fountain
	fountain_position = vec3(0.0f, 1.0f, 0.0f);
	float fountain_collision_size = 3.7f;
	mat4 fountain_mod_mat = translate(identity_mat4(), fountain_position);
	CollisionBox fountain_collision = CollisionBox(fountain_position, fountain_collision_size, fountain_collision_size);
	Entity fountain = Entity("fountain", &fountain_mesh, fountain_mod_mat, fountain_collision);
	collidable_entities.push_back(fountain);

	// ------- water
	vec3 water_position1(0.0f, 1.5f, 0.0f);
	mat4 water_mod_mat1 = translate(identity_mat4(), water_position1);
	Entity water1 = Entity("water", &water_mesh, water_mod_mat1);

	vec3 water_position2(0.0f, 1.4f, 0.0f);
	mat4 water_mod_mat2 = translate(identity_mat4(), water_position2);
	water_mod_mat2 = local_rotate_y(water_mod_mat2, 90);
	Entity water2 = Entity("water", &water_mesh, water_mod_mat2);

	// ------- crystals
	vector<Entity> removed_crystals;
	vector<mat4> crystal_mod_mats;
	for (int i = 0; i < crystal_positions.size(); i++) {
		crystal_mod_mats.push_back(translate(identity_mat4(), crystal_positions[i]));
		crystal_mod_mats[i] = local_rotate_y(crystal_mod_mats[i], 90);

	}
	vector<Entity> crystals;
	float crystal_collision_size = 1.0f;
	for (int i = 0; i < crystal_positions.size(); i++) {
		float x = crystal_positions[i].v[0];
		float z = crystal_positions[i].v[2];
		vec3 col_position = vec3(x, 0.0f, z);
		CollisionBox c = CollisionBox(col_position, crystal_collision_size, crystal_collision_size);
		c.addModelMat(translate(identity_mat4(), col_position));
		char* crystal_label = "crystal";
		stringstream ss;
		ss << crystal_label << i;
		Entity cr = Entity(ss.str(), &crystal_mesh, crystal_mod_mats[i], c);
		collidable_entities.push_back(cr);
		crystals.push_back(cr);
	}

	// ------- wall2s
	vector<int> ns_walls = { 3, 4, 6, 7, 8, 9, 10, 13, 17, 18, 19, 24, 27, 28, 29, 30, 31, 32, 33, 34, 38, 39, 40, 41 };
	vector<mat4> wall2_mod_mats;
	for (int i = 0; i < wall2_positions.size(); i++) {
		wall2_mod_mats.push_back(translate(identity_mat4(), wall2_positions[i]));
		wall2_mod_mats[i] = translate(wall2_mod_mats[i], vec3(0.0f, 2.4f, 0.0f));
		if (find(ns_walls.begin(), ns_walls.end(), i) == ns_walls.end()) {
			wall2_mod_mats[i] = local_rotate_y(wall2_mod_mats[i], 90);
		}
	}
	vector<Entity> wall2s;
	float wall2_collision_size_a = 1.0f;
	float wall2_collision_size_b = 7.0f;
	CollisionBox c;
	for (int i = 0; i < wall2_positions.size(); i++) {
		float x = wall2_positions[i].v[0];
		float z = wall2_positions[i].v[2];
		vec3 col_position = vec3(x, 0.0f, z);
		if (find(ns_walls.begin(), ns_walls.end(), i) != ns_walls.end()) {
			c = CollisionBox(col_position, wall2_collision_size_a, wall2_collision_size_b);
		}
		else {
			c = CollisionBox(col_position, wall2_collision_size_b, wall2_collision_size_a);
		}
		c.addModelMat(translate(identity_mat4(), col_position));
		Entity w = Entity("wall2", &wall2_mesh, wall2_mod_mats[i], c);
		collidable_entities.push_back(w);
		wall2s.push_back(w);
	}


	// ------- pillar2s
	vector<mat4> pillar2_mod_mats;
	for (int i = 0; i < pillar2_positions.size(); i++) {
		pillar2_mod_mats.push_back(translate(identity_mat4(), pillar2_positions[i]));
		pillar2_mod_mats[i] = local_rotate_y(pillar2_mod_mats[i], -i * 90);
	}
	vector<Entity> pillar2s;
	float pillar2_collision_size = 1.2f;
	for (int i = 0; i < pillar2_positions.size(); i++) {
		float x = pillar2_positions[i].v[0];
		float z = pillar2_positions[i].v[2];
		vec3 col_position = vec3(x, 0.0f, z);
		CollisionBox c = CollisionBox(col_position, pillar2_collision_size, pillar2_collision_size);
		c.addModelMat(translate(identity_mat4(), col_position));
		Entity p = Entity("pillar2", &pillar2_mesh, pillar2_mod_mats[i], c);
		collidable_entities.push_back(p);
		pillar2s.push_back(p);
	}

	// ------- statues
	vector<mat4> statue_mod_mats;
	for (int i = 0; i < statue_positions.size(); i++) {
		statue_mod_mats.push_back(translate(identity_mat4(), statue_positions[i]));
		statue_mod_mats[i] = local_rotate_y(statue_mod_mats[i], -i * 90);
	}
	vector<Entity> statues;
	float statue_collision_size = 1.2f;
	for (int i = 0; i < statue_positions.size(); i++) {
		float x = statue_positions[i].v[0];
		float z = statue_positions[i].v[2];
		vec3 col_position = vec3(x, 0.0f, z);
		CollisionBox c = CollisionBox(col_position, statue_collision_size, statue_collision_size);
		c.addModelMat(translate(identity_mat4(), col_position));
		Entity s = Entity("statue", &statue_mesh, statue_mod_mats[i], c);
		collidable_entities.push_back(s);
		statues.push_back(s);
	}

	// ------- arms
	arm_position = vec3(1.5f, -0.8f, 1.6f);
	mat4 arm_mod_mat = translate(identity_mat4(), arm_position);
	arm_mod_mat = scale(arm_mod_mat, vec3(0.1, 0.1, 0.1));
	Entity outer_arm = Entity("arm1", &arm_mesh, arm_mod_mat);
	Entity inner_arm = Entity("arm2", &arm_mesh, scale(arm_mod_mat, vec3(0.97f, 0.97f, 0.97f)));

	// ------- torch
	torch_position = vec3(1.5f, -0.4f, 1.67f);
	mat4 torch_mod_mat = translate(identity_mat4(), torch_position);
	torch_mod_mat = scale(torch_mod_mat, vec3(0.1, 0.1, 0.1));
	Entity torch = Entity("torch", &torch_mesh, torch_mod_mat);

	// ------- menu
	menu_position = vec3(0.0f, 0.0f, 0.75f);
	mat4 menu_mod_mat = translate(identity_mat4(), menu_position);
	menu_mod_mat = scale(menu_mod_mat, vec3(6.0f, 6.0f, 6.0f));
	menu_mod_mat = local_rotate_y(menu_mod_mat, 180);
	Entity menu = Entity("menu", &menu_mesh, menu_mod_mat);

	// ------- particle fountain
	particle_fountain_position = vec3(0.75f, 3.4f, -0.90f);
	int max_particles = 700;
	vector<Particle> particles;
	while (particles.size() < max_particles){
		Particle p = Particle(&particle_mesh, particle_fountain_position);
		particles.push_back(p);
		for (int i = 0; i < particles.size(); i++) {
			if (particles[i].getLifespan() > 0) {
				particles[i].update();
			}
			else {
				particles[i].revive(particle_fountain_position);
			}
		}
	}

	// ------- collision box
	float square_collision_points[] = {
		-0.25f, 0.1f, 0.25f,
		0.25f, 0.1f, 0.25f,
		-0.25f, 0.1f, -0.25f,
		0.25f, 0.1f, 0.25f,
		0.25f, 0.1f, -0.25f,
		-0.25f, 0.1f, -0.25f
	};
	Mesh square_collision_box = Mesh(square_collision_points);
	CollisionBox::addMesh(&square_collision_box);



	//******************************************
	// Set up sound effects and music
	//******************************************

	ISoundEngine* engine = createIrrKlangDevice();
	if (!engine) {
		return 0; // error starting up the engine
	}
	ISound* footsteps = engine->play2D("footsteps_grass.mp3", true, true, true);
	ISound* fountain_sound = engine->play2D("fountain.wav", true, false, true);
	ISound* gasp = engine->play2D("gasp.mp3", false, true, true);
	ISound* gasp2 = engine->play2D("gasp2.wav", false, true, true);
	ISound* echo = engine->play2D("echo.mp3", false, true, true);
	ISoundSource* song = engine->addSoundSourceFromFile("oneohtrix-point-never-replica.mp3");
	song->setDefaultVolume(0.6f);



	//******************************************
	// Set rendering defaults
	//******************************************

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


	//********************************************************************************************************************
	// MAIN PROGRAM LOOP
	//********************************************************************************************************************
	while (!glfwWindowShouldClose(g_window)) {

		bool menu_tex_flag = false;
		bool menu_input_flag = false;
		bool credits_flag = false;
		bool exit = false;
		float current_time_flicker = (float)glfwGetTime();
		float previous_time_flicker = (float)glfwGetTime();
		while (particles.size() < max_particles) {
			Particle p = Particle(&particle_mesh, particle_fountain_position);
			particles.push_back(p);
			for (int i = 0; i < particles.size(); i++) {
				if (particles[i].getLifespan() > 0) {
					particles[i].update();
				}
				else {
					particles[i].revive(particle_fountain_position);
				}
			}
		}
		cam_pos = vec3(0.0f, 2.0f, 5.0f);
		menu_position = vec3(0.0f, 0.0f, 0.75f);
		menu_mod_mat = translate(identity_mat4(), menu_position);
		menu_mod_mat = scale(menu_mod_mat, vec3(6.0f, 6.0f, 6.0f));
		menu_mod_mat = local_rotate_y(menu_mod_mat, 180);
		menu.setModelMat(menu_mod_mat);

		//******************************************
		// Menu loop
		//******************************************
		while (true) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, g_gl_width, g_gl_height);
			glActiveTexture(GL_TEXTURE0);
			// set up particle fountain while waiting
			if (particles.size() < max_particles){
				Particle p = Particle(&particle_mesh, particle_fountain_position);
				particles.push_back(p);
			}
			for (int i = 0; i < particles.size(); i++) {
				if (particles[i].getLifespan() > 0) {
					particles[i].update();
				}
				else {
					particles[i].revive(particle_fountain_position);
				}
			}
			// user pressed enter, fade to black, go to play state
			if (glfwGetKey(g_window, GLFW_KEY_ENTER) && menu_input_flag == false) {
				menu_input_flag = true;
			}
			if (menu_input_flag == true) {
				menu.render(model_mat_location_black, menu_black_tex, false);
				break;
			}
			if (glfwGetKey(g_window, GLFW_KEY_C) && credits_flag != true) {
				credits_flag = true;
			}
			if (glfwGetKey(g_window, GLFW_KEY_M) && credits_flag == true) {
				credits_flag = false;
			}
			if (credits_flag == false) {
				current_time_flicker = (float)glfwGetTime();
				glUseProgram(shader_programme_black);
				if ((current_time_flicker - previous_time_flicker) > 0.8) {
					previous_time_flicker = current_time_flicker;
					menu_tex_flag = !menu_tex_flag;
				}
				if (menu_tex_flag == true) {
					menu.render(model_mat_location_black, menu1_tex, false);
				}
				else {
					menu.render(model_mat_location_black, menu2_tex, false);
				}
			}
			else {
				current_time_flicker = (float)glfwGetTime();
				glUseProgram(shader_programme_black);
				if ((current_time_flicker - previous_time_flicker) > 0.8) {
					previous_time_flicker = current_time_flicker;
					menu_tex_flag = !menu_tex_flag;
				}
				if (menu_tex_flag == true) {
					menu.render(model_mat_location_black, credits1_tex, false);
				}
				else {
					menu.render(model_mat_location_black, credits2_tex, false);
				}
			}
			if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
				exit = true;
				break;
			}
			glfwPollEvents();
			glfwSwapBuffers(g_window);
		}

		if (exit == true) {
			break;
		}

		bool cutscene = true;
		bool end_cutscene = false;
		float current_time_intro = (float)glfwGetTime();
		float previous_time_intro = current_time_intro;
		mat4 menu_model = menu.getModelMatrix();
		menu_model = translate(menu_model, vec3(0.0f, 0.0f, -4.37f));
		menu.setModelMat(menu_model);

		// player stats
		int current_life_state;
		int current_life;
		int life0 = 60;
		int life1 = 30;
		int life2 = 20;
		int life3 = 10;
		vector<int> lives = { life0, life1, life2, life3 };
		float current_time_life = (float)glfwGetTime();
		float previous_time_life = current_time_life;
		current_life_state = 0;
		current_life = life0;

		bool fountain_cooldown_flag = false;
		float current_time_cooldown = (float)glfwGetTime();
		float previous_time_cooldown = current_time_cooldown;

		float current_time_outro = (float)glfwGetTime();
		float previous_time_outro = current_time_outro;

		float black_fade_opacity = 1.0f;
		float arm_opacity = 1.0f;

		// reset crystals and entities
		if (crystals.size() < 4) {
			crystals.insert(crystals.end(), removed_crystals.begin(), removed_crystals.end());
			removed_crystals.clear();
		}
		number_of_crystals_found = 0;
		collidable_entities.clear();
		collidable_entities.insert(collidable_entities.end(), trees.begin(), trees.end());
		collidable_entities.insert(collidable_entities.end(), crystals.begin(), crystals.end());
		collidable_entities.insert(collidable_entities.end(), wall2s.begin(), wall2s.end());
		collidable_entities.insert(collidable_entities.end(), pillar2s.begin(), pillar2s.end());
		collidable_entities.insert(collidable_entities.end(), statues.begin(), statues.end());
		collidable_entities.push_back(fountain);


		// reset camera
		mat4 R;
		versor Q = quat_from_axis_deg(-cam_heading, 0.0f, 1.0f, 0.0f);
		R = quat_to_mat4(Q);
		view_mat = R * T;

		//******************************************
		// Game loop
		//******************************************		
		while (true) {
			cout << "num crystals: " << removed_crystals.size() << '\n';
			cout << "num entities: " << collidable_entities.size() << '\n';
			if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
				glfwSetWindowShouldClose(g_window, 1);
				break;
			}

			if (current_life <= 0 && end_cutscene == false) {
				gasp2 = engine->play2D("gasp2.wav", false, false, true);
				end_cutscene = true;
				current_time_outro = (float)glfwGetTime();
				previous_time_outro = current_time_outro;
			}

			// manage intro cutscene
			current_time_intro = (float)glfwGetTime();
			if (cutscene == true && (current_time_intro - previous_time_intro) > 12) {
				cutscene = false;
				black_fade_opacity = 0.0f;
			}
			if (cutscene == true) {
				black_fade_opacity = black_fade_opacity - 0.0019f;
			}

			// manage end cutscene
			if (end_cutscene == true) {
				black_fade_opacity = black_fade_opacity + 0.0019f;
				current_time_outro = (float)glfwGetTime();

			}
			if (end_cutscene == true && (current_time_outro - previous_time_outro) > 12) {
				end_cutscene = false;
				black_fade_opacity = 1.0f;
				break;
			}

			// manage player life
			if (cutscene == false) {
				current_time_life = (float)glfwGetTime();
				if ((current_time_life - previous_time_life) >= 1) {
					current_life -= 1;
					if (current_life_state == 0) {
						arm_opacity -= 0.0166f;
					}
					else if (current_life_state == 1) {
						arm_opacity -= 0.0333f;
					}
					else if (current_life_state == 2) {
						arm_opacity -= 0.05f;
					}
					else {
						arm_opacity -= 0.1f;
					}
					previous_time_life = current_time_life;
				}
			}
			current_time_cooldown = (float)glfwGetTime();
			if ((current_time_cooldown - previous_time_cooldown) >= 3 && fountain_cooldown_flag == true) {
				fountain_cooldown_flag = false;
				previous_time_cooldown = current_time_cooldown;
			}

			if (engine->isCurrentlyPlaying(song) != true) {
				engine->play2D(song, true);
			}

			float sine_time = sinf((float)glfwGetTime());

			static double previous_seconds = glfwGetTime();
			double current_seconds = glfwGetTime();
			double elapsed_seconds = current_seconds - previous_seconds;
			previous_seconds = current_seconds;



			//******************************************
			// Draw
			//******************************************

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, g_gl_width, g_gl_height);

			glUseProgram(shader_programme1);
			glActiveTexture(GL_TEXTURE0);

			// ------- draw ground
			glUseProgram(shader_programme1);
			for (int i = 0; i < ground_tiles.size(); i++) {
				ground_tiles[i].render(model_mat_location_entity, ground_tex, false);
			}

			// ------- draw trees
			glUseProgram(shader_programme1);
			for (int i = 0; i < trees.size(); i++) {
				trees[i].render(model_mat_location_entity, tree_tex, false);
			}

			// ------- draw wall2s
			glUseProgram(shader_programme1);
			for (int i = 0; i < wall2s.size(); i++) {
				glUseProgram(shader_programme1);
				wall2s[i].render(model_mat_location_entity, wall2_tex, true);
			}

			// ------- draw pillar2s
			glUseProgram(shader_programme1);
			for (int i = 0; i < pillar2s.size(); i++) {
				pillar2s[i].render(model_mat_location_entity, pillar2_tex, false);
			}

			// ------- draw statues
			glUseProgram(shader_programme1);
			for (int i = 0; i < statues.size(); i++) {
				statues[i].render(model_mat_location_entity, black_crystal_tex, false);
			}

			// ------- draw crystal
			glUseProgram(shader_programme1);
			for (int i = 0; i < crystals.size(); i++) {
				mat4 crystal_mod_mat = crystals[i].getModelMatrix();
				crystal_mod_mat = translate(crystal_mod_mat, vec3(0.0f, sine_time * 0.0001, 0.0f));
				crystal_mod_mat = local_rotate_y(crystal_mod_mat, 0.1);
				crystals[i].setModelMat(crystal_mod_mat);
				crystals[i].render(model_mat_location_entity, white_crystal_tex, false);
			}

			// -------- draw fountain
			glUseProgram(shader_programme1);
			fountain.render(model_mat_location_entity, marble_tex, false);

			// --------draw water
			glUseProgram(shader_programme_water);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			water_mod_mat1 = local_rotate_y(water_mod_mat1, sine_time * 0.08);
			water1.setModelMat(water_mod_mat1);
			water1.render(model_mat_location_water, water_tex, false);
			water_mod_mat2 = local_rotate_y(water_mod_mat2, sine_time * 0.08);
			water2.setModelMat(water_mod_mat2);
			water2.render(model_mat_location_water, water_tex, false);
			glDisable(GL_BLEND);

			// -------- draw particle fountain
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glUseProgram(shader_programme_particle);
			if (particles.size() < max_particles){
				Particle p = Particle(&particle_mesh, particle_fountain_position);
				particles.push_back(p);
			}
			for (int i = 0; i < particles.size(); i++) {
				if (particles[i].getLifespan() > 0) {
					particles[i].render(model_mat_location_particle, particle_tex);
					particles[i].update();
				}
				else {
					particles[i].revive(particle_fountain_position);
				}
			}
			glDisable(GL_BLEND);

			// -------- draw arms
			vec3 movement = vec3(sine_time*0.00008, sine_time*0.00008, 0.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUseProgram(shader_programme_torch);
			if (end_cutscene == false) {
				inner_arm.move(movement);
			}
			inner_arm.render(model_mat_location_torch, black_crystal_tex, false);

			glUseProgram(shader_programme_white);
			glDisable(GL_DEPTH_TEST);
			if (end_cutscene == false) {
				outer_arm.move(movement);
			}
			outer_arm.render(model_mat_location_white, white_crystal_tex, false);
			glUseProgram(shader_programme_white);
			glUniform1f(opacity_int_location_white, arm_opacity);
			glEnable(GL_DEPTH_TEST);

			// -------- draw torch
			glUseProgram(shader_programme_torch);
			if (end_cutscene == false) {
				torch.move(movement);
			}
			torch.render(model_mat_location_torch, torch_tex, false);
			glDisable(GL_BLEND);


			// ------- draw menu for cutscene
			if (cutscene == true) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glUseProgram(shader_programme_black);
				menu.render(model_mat_location_black, menu_black_tex, false);
				glDisable(GL_BLEND);
				glUseProgram(shader_programme_black);
				glUniform1f(opacity_int_location_black, black_fade_opacity);
			}

			if (end_cutscene == true) {
				cout << "opacity: " << black_fade_opacity << '\n';
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glUseProgram(shader_programme_black);
				menu.render(model_mat_location_black, menu_black_tex, false);
				glDisable(GL_BLEND);
				glUseProgram(shader_programme_black);
				glUniform1f(opacity_int_location_black, black_fade_opacity);
			}

			glfwPollEvents();

			glUseProgram(shader_programme1);

			// ------- camera defaults
			bool cam_moved = false;
			vec3 move(0.0, 0.0, 0.0);
			float cam_yaw = 0.0f;
			float cam_pitch = 0.0f;
			float cam_roll = 0.0;
			bool feet_moved = false;

			//******************************************
			// Handle input
			//******************************************
			if (cutscene == false && end_cutscene == false) {
				if (glfwGetKey(g_window, GLFW_KEY_A)) {
					move.v[0] -= cam_speed * elapsed_seconds;
					cam_moved = true;
					feet_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_D)) {
					move.v[0] += cam_speed * elapsed_seconds;
					cam_moved = true;
					feet_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_W)) {
					move.v[2] -= cam_speed * elapsed_seconds;
					cam_moved = true;
					feet_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_S)) {
					move.v[2] += cam_speed * elapsed_seconds;
					cam_moved = true;
					feet_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_LEFT)) {
					cam_yaw += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_yaw = quat_from_axis_deg(cam_yaw, up.v[0], up.v[1], up.v[2]);
					Q = q_yaw * Q;
				}
				if (glfwGetKey(g_window, GLFW_KEY_RIGHT)) {
					cam_yaw -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_yaw = quat_from_axis_deg(cam_yaw, up.v[0], up.v[1], up.v[2]);
					Q = q_yaw * Q;
				}
				if (glfwGetKey(g_window, GLFW_KEY_UP)) {
					cam_pitch += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_pitch = quat_from_axis_deg(cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					if ((q_pitch * Q).q[1] < 0.5) {
						Q = q_pitch * Q;
					}
				}
				if (glfwGetKey(g_window, GLFW_KEY_DOWN)) {
					cam_pitch -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_pitch = quat_from_axis_deg(cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					if ((q_pitch * Q).q[1] > -0.5) {
						Q = q_pitch * Q;
					}
				}
			}

			//******************************************
			// Update camera
			//******************************************			
			if (cam_moved) {
				if (feet_moved == true) {
					if (footsteps->getIsPaused() == true) {
						footsteps = engine->play2D("footsteps_grass.mp3", true, false, true);
					}
				}
				// re-calculate local axes so can move fwd in dir cam is pointing
				R = quat_to_mat4(Q);
				fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
				rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
				up = vec4(0.0, 1.0, 0.0, 0.0);

				vec3 fwd_y_constrained = vec3(fwd);
				fwd_y_constrained.v[1] = 0;

				vector<Entity> nearest_collidable;
				for (int i = 0; i < collidable_entities.size(); i++) {
					float dist = sqrt(get_squared_dist(camera_collision.getPos(), collidable_entities[i].getCollisionBox().getPos()));
					if (dist < 10.0f) {
						nearest_collidable.push_back(collidable_entities[i]);
					}
				}

				// new camera position variables
				vec3 new_cam_pos_x;
				vec3 new_cam_pos_z;
				vec3 new_cam_pos = vec3(cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]);

				// check new z posistion for possible collision
				new_cam_pos_z = new_cam_pos + fwd_y_constrained * -move.v[2];
				CollisionBox new_cam_col_z = CollisionBox(new_cam_pos_z, camera_collision_size, camera_collision_size);
				bool collision_z = false;
				int position_crystal_to_remove = -1;
				for (int i = 0; i < nearest_collidable.size(); i++) {
					if (new_cam_col_z.collides(nearest_collidable[i].getCollisionBox()) == true) {
						cout << "z_collision\n";
						collision_z = true;
						if (nearest_collidable[i].getLabel().substr(0, 7) == "crystal") {
							echo = engine->play2D("echo.mp3", false, false, true);
							string crystal_to_remove = nearest_collidable[i].getLabel();
							for (int j = 0; j < crystals.size(); j++) {
								if (crystals[j].getLabel() == crystal_to_remove) {
									position_crystal_to_remove = j;
								}
							}
						}
						// touched fountain, recover health
						if (nearest_collidable[i].getLabel() == "fountain") {
							if (current_life_state == 3 && current_life > 0 && fountain_cooldown_flag == false) {
								current_life = 10;
								fountain_cooldown_flag = true;
								arm_opacity = 1.0f;
								gasp = engine->play2D("gasp.mp3", false, false, true);
								gasp->setVolume(0.2f);
							}
							if (current_life_state < 3 && current_life < lives[current_life_state + 1]) {
								current_life_state++;
								current_life = lives[current_life_state];
								arm_opacity = 1.0f;
								gasp = engine->play2D("gasp.mp3", false, false, true);
								gasp->setVolume(0.2f);
							}

						}
					}
				}
				if (position_crystal_to_remove != -1) {
					string removed_crystal_label = crystals[position_crystal_to_remove].getLabel();
					removed_crystals.push_back(crystals[position_crystal_to_remove]);
					crystals.erase(crystals.begin() + position_crystal_to_remove);
					int remove_crystal_pos = -1;
					for (int i = 0; i < collidable_entities.size(); i++) {
						if (collidable_entities[i].getLabel() == removed_crystal_label) {
							remove_crystal_pos = i;
						}
					}
					if (remove_crystal_pos != -1) {
						collidable_entities.erase(collidable_entities.begin() + remove_crystal_pos);
					}
					number_of_crystals_found++;
				}

				if (!collision_z) {
					new_cam_pos += fwd_y_constrained * -move.v[2];
				}
				else {
					new_cam_pos = new_cam_pos - (fwd_y_constrained * -move.v[2]);
				}

				// check new x position for possible collision
				new_cam_pos_x = new_cam_pos + vec3(rgt) * move.v[0];
				CollisionBox new_cam_col_x = CollisionBox(new_cam_pos_x, camera_collision_size, camera_collision_size);
				bool collision_x = false;
				position_crystal_to_remove = -1;
				for (int i = 0; i < nearest_collidable.size(); i++) {
					if (new_cam_col_x.collides(nearest_collidable[i].getCollisionBox()) == true) {
						cout << "x_collision\n";
						collision_x = true;
						if (nearest_collidable[i].getLabel().substr(0, 7) == "crystal") {
							echo = engine->play2D("echo.mp3", false, false, true);
							string crystal_to_remove = nearest_collidable[i].getLabel();
							for (int j = 0; j < crystals.size(); j++) {
								if (crystals[j].getLabel() == crystal_to_remove) {
									position_crystal_to_remove = j;
								}
							}
						}
						// touched fountain, recover health
						if (nearest_collidable[i].getLabel() == "fountain") {
							if (current_life_state == 4 && current_life > 0 && fountain_cooldown_flag == false) {
								current_life = 10;
								fountain_cooldown_flag = true;
								arm_opacity = 1.0f;
								gasp = engine->play2D("gasp.mp3", false, false, true);
								gasp->setVolume(0.2f);
							}
							else if (current_life < lives[current_life_state + 1]) {
								current_life_state++;
								current_life = lives[current_life_state];
								arm_opacity = 1.0f;
								gasp = engine->play2D("gasp.mp3", false, false, true);
								gasp->setVolume(0.2f);
							}

						}
					}
				}
				if (position_crystal_to_remove != -1) {
					string removed_crystal_label = crystals[position_crystal_to_remove].getLabel();
					removed_crystals.push_back(crystals[position_crystal_to_remove]);
					crystals.erase(crystals.begin() + position_crystal_to_remove);
					int remove_crystal_pos = -1;
					for (int i = 0; i < collidable_entities.size(); i++) {
						if (collidable_entities[i].getLabel() == removed_crystal_label) {
							remove_crystal_pos = i;
						}
					}
					if (remove_crystal_pos != -1) {
						collidable_entities.erase(collidable_entities.begin() + remove_crystal_pos);
					}
					number_of_crystals_found++;
				}

				if (!collision_x) {
					new_cam_pos += vec3(rgt) * move.v[0];
				}
				else {
					new_cam_pos = new_cam_pos - (vec3(rgt) * move.v[0]);
				}

				// move camera to new position
				cam_pos = vec3(new_cam_pos.v[0], new_cam_pos.v[1], new_cam_pos.v[2]);
				camera_collision.move(cam_pos);

				// create new view matrix and send to shader
				mat4 T = translate(identity_mat4(), vec3(new_cam_pos));
				view_mat = inverse(R) * inverse(T);


			}
			else {
				if (footsteps->getIsPaused() != true) {
					footsteps->setIsPaused();
				}
			}

			glUseProgram(shader_programme1);
			glUniformMatrix4fv(view_mat_location_entity, 1, GL_FALSE, view_mat.m);
			glUseProgram(shader_programme_water);
			glUniformMatrix4fv(view_mat_location_water, 1, GL_FALSE, view_mat.m);
			glUseProgram(shader_programme_particle);
			glUniformMatrix4fv(view_mat_location_particle, 1, GL_FALSE, view_mat.m);
			glUseProgram(shader_programme_white);
			glUniform1f(opacity_int_location_white, arm_opacity);
			glfwSwapBuffers(g_window);
		}
	}
	glfwTerminate();
	return 0;
}