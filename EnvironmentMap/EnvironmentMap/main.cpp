/*
CS6610 Project 6 - Environment Mapping
AUTHOR: Varsha Alangar
Created on: 02/25/2017
*/

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/GL.h>
#include <cyCodeBase/cyMatrix.h>
#include <cyCodeBase/cyGL.h>
#include <cyCodeBase/cyTriMesh.h>
#include <imageLoader/lodepng.h>
#include <iostream>
#include <cmath>
#include <GL/vmath.h>
#include <GL/glm/glm.hpp>
#include <ctype.h>
#include <string.h>
#include <vector>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

std::vector<cy::Point3f> v_indices;
std::vector<cy::Point3f> cube_v_indices;
std::vector<cy::Point3f> n_indices;
std::vector<cy::Point3f> quad_normal;
std::vector<float> diffuse_Kd;
std::vector<float> specular_Ks;
std::vector<unsigned char> image;
std::vector<unsigned char> image1;
std::vector<unsigned char> image2;
std::vector<unsigned char> image3;
std::vector<unsigned char> image4;
std::vector<unsigned char> image5;
std::vector<unsigned char> image6;

unsigned img_width = 512;
unsigned img_height = 512;

int window_width = 600;
int window_height = 600;
int prevMouseX = 0;
int prevMouseY = 0;
int curMouseX = 0;
int curMouseY = 0;
int plane_prevMouseX = 0;
int plane_prevMouseY = 0;
int plane_curMouseX = 0;
int plane_curMouseY = 0;

float x_pos = 0.0f;
float y_pos = 10.0f;
float z_pos = 10.0f;
float scale_factor = 1.0f;
float rotation_angle;
float light_rotation_angle;
float plane_x_pos = 0.0f;
float plane_y_pos = 10.0f;
float plane_z_pos = 11.0f; //11
float plane_scale_factor = 25.0f; //25
float plane_rotation_angle;
float reflection_x_pos = 0.0f;
float reflection_y_pos = 10.0f;
float reflection_z_pos = 10.0f;
float reflection_rotation_angle;
float reflection_scale_factor = 1.0f;

bool zoom = false;
bool rotate = false;
bool flag = false;
bool ctrl = false;
bool ctrl_flag = false;
bool shift = false;
bool shift_flag = false;
bool plane_zoom = false;
bool plane_rotate = false;
bool plane_flag = false;
bool alt = false;

GLuint vao;
GLuint plane_vao;
GLuint cube_vao;
GLuint tex_obj1;
GLuint tex_obj2;
GLuint quad_buffer;
GLuint quad_n_buffer;
GLuint cube_buffer;
GLuint v_buffer;
GLuint n_buffer;
GLuint t_buffer;
GLuint vs_id;
GLuint fs_id;
GLuint program1;
GLuint program2;
GLuint cube_program;
GLuint reflective_program;
GLuint locationID;
GLuint textureID;
GLuint cubeTexID;

cy::TriMesh trimesh_obj;
cy::TriMesh cube_trimesh_obj;
cy::GLSLShader glsl_shader_obj;
cy::GLSLShader glsl_plane_shader_obj;
cy::GLSLProgram glsl_program_obj;
cy::GLSLProgram glsl_plane_program_obj;
cy::GLSLProgram glsl_cube_program_obj;
cy::GLSLProgram glsl_reflective_program_obj;
cy::GLRenderBuffer<GL_TEXTURE_RECTANGLE> glrb_obj;
cy::GLTexture<GL_TEXTURE_2D> texture_obj;
cy::Matrix4<float> matrix4_obj;
cy::Matrix3<float> matrix3_obj;

vmath::mat4 rotationMatrix = vmath::mat4::identity();
vmath::mat4 lightRotationMatrix = vmath::mat4::identity();
vmath::mat4 translationMatrix = vmath::mat4::identity();
vmath::mat4 projection;
vmath::mat4 reflectionProjection;
vmath::mat4 planeProjection;
vmath::mat4 planeRotationMatrix = vmath::mat4::identity();
vmath::mat4 planeTranslationMatrix = vmath::mat4::identity();
vmath::mat4 reflectionRotationMatrix = vmath::mat4::identity();
vmath::mat4 reflectionTranslationMatrix = vmath::mat4::identity();

vmath::vec3 getArcballVector(int x, int y);
vmath::vec3 axis_in_camera_coord;
vmath::vec3 light_axis_in_camera_coord;
vmath::vec3 plane_axis_in_camera_coord;
vmath::vec3 reflection_axis_in_camera_coord;

void idle()
{
	glutPostRedisplay();
}

void setShaders()
{
	glsl_program_obj.BuildFiles("Shaders/teapot_vs.vert", "Shaders/teapot_fs.frag", NULL, NULL, NULL, &std::cout);
	vs_id = glsl_shader_obj.GetID();
	fs_id = glsl_shader_obj.GetID();
	program1 = glsl_program_obj.GetID();
}

void setPlaneShaders()
{
	glsl_plane_program_obj.BuildFiles("Shaders/plane_vs.vert", "Shaders/plane_fs.frag", NULL, NULL, NULL, &std::cout);
	vs_id = glsl_plane_shader_obj.GetID();
	fs_id = glsl_plane_shader_obj.GetID();
	program2 = glsl_plane_program_obj.GetID();
}

void setCubeShaders()
{
	glsl_cube_program_obj.BuildFiles("Shaders/cube_vs.vert", "Shaders/cube_fs.frag", NULL, NULL, NULL, &std::cout);
	cube_program = glsl_cube_program_obj.GetID();
}

void setReflectiveShaders()
{
	glsl_reflective_program_obj.BuildFiles("Shaders/reflective_vs.vert", "Shaders/reflective_fs.frag", NULL, NULL, NULL, &std::cout);
	reflective_program = glsl_reflective_program_obj.GetID();
}

// Referred tutorial from the following website for Arcball:
// http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
//Also referred to Superbible book examples

vmath::vec3 getArcballVector(int x, int y) {
	vmath::vec3 vecP = vmath::vec3(1.0f * x / window_width * 2 - 1.0f, 1.0f * y / window_height * 2 - 1.0f, 0.0f);
	vecP[1] = -vecP[1];
	float vecPsquared = vecP[0] * vecP[0] + vecP[1] * vecP[1];
	if (vecPsquared <= 1)
		vecP[2] = sqrt(1 - vecPsquared);
	else
		vecP = vmath::normalize(vecP);
	return vecP;
}

void keys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: glutLeaveMainLoop();
		break;
	}

}

void special_keys(int key, int x, int y)
{
	if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
	{
		ctrl = true;
	}
	else if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
	{
		shift = true;
	}
	else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
	{
		alt = true;
	}
	switch (key)
	{
	case GLUT_KEY_F6:	glsl_program_obj.BuildFiles("Shaders/teapot_vs.vert", "Shaders/teapot_fs.frag", NULL, NULL, NULL, &std::cout);
		break;
	}
}

void mouseButtonPress(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (ctrl)
		{
			ctrl_flag = true;
		}
		else if (shift)
		{
			shift_flag = true;
		}
		else if (alt)
		{
			plane_rotate = true;
			plane_prevMouseX = plane_curMouseX = x;
			plane_prevMouseY = plane_curMouseY = y;
		}
		else
		{
			rotate = true;
			prevMouseX = curMouseX = x;
			prevMouseY = curMouseY = y;
		}

	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (alt)
		{
			plane_zoom = true;
			plane_prevMouseX = plane_curMouseX = x;
			plane_prevMouseY = plane_curMouseY = y;
		}
		else
		{
			zoom = true;
			prevMouseX = curMouseX = x;
			prevMouseY = curMouseY = y;
		}

	}
	if (state == GLUT_UP)
	{
		rotate = false;
		zoom = false;
		ctrl = false;
		ctrl_flag = false;
		shift = false;
		shift_flag = false;

		plane_rotate = false;
		plane_zoom = false;
		alt = false;
	}
}

void mouseMove(int x, int y)
{
	if (rotate || zoom || ctrl_flag || shift_flag)
	{
		curMouseX = x;
		curMouseY = y;
	}
	else if (plane_rotate || plane_zoom)
	{
		plane_curMouseX = x;
		plane_curMouseY = y;
	}
}

void load_img(char *filename)
{
	std::cout << filename;
	std::string file(filename);

	unsigned error = lodepng::decode(image, img_width, img_height, "Objects/Teapot/" + file);
	if (error != 0)
	{
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

}

void createRenderTarget()
{
	//CREATING THE RENDER TARGET:
	GLsizei rb_width = 600;
	GLsizei rb_height = 600;
	cyGL::Type type = cyGL::TYPE_UBYTE;

	bool check_init = glrb_obj.Initialize(true, 3, window_width, window_height, type);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not okay!";

	glrb_obj.BuildTextureMipmaps();
	glrb_obj.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR); //BILINEAR FILTERING FOR  MAGNIFICATION
	glrb_obj.SetTextureMaxAnisotropy();
}

void createQuad()
{

	//CREATING QUAD TO USE THE RENDERED TEXTURE:
	static const GLfloat quad_vertex[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &plane_vao);
	glBindVertexArray(plane_vao);

	glGenBuffers(1, &quad_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex), quad_vertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

GLuint loadCubeMapTextures(std::vector<const GLchar*> cube_faces)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	unsigned error = lodepng::decode(image1, img_width, img_height, cube_faces[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image1[0]);
	error = lodepng::decode(image2, img_width, img_height, cube_faces[1]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image2[0]);
	error = lodepng::decode(image3, img_width, img_height, cube_faces[2]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image3[0]);
	error = lodepng::decode(image4, img_width, img_height, cube_faces[3]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image4[0]);
	error = lodepng::decode(image5, img_width, img_height, cube_faces[4]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image5[0]);
	error = lodepng::decode(image6, img_width, img_height, cube_faces[5]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image6[0]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return texID;
}

void createCube()
{
	std::vector<const GLchar*> cube_faces;
	cube_faces.push_back("Objects/CubemapImages/cubemap_posx.PNG");
	cube_faces.push_back("Objects/CubemapImages/cubemap_negx.PNG");
	cube_faces.push_back("Objects/CubemapImages/cubemap_posy.PNG");
	cube_faces.push_back("Objects/CubemapImages/cubemap_negy.PNG");
	cube_faces.push_back("Objects/CubemapImages/cubemap_negz.PNG");
	cube_faces.push_back("Objects/CubemapImages/cubemap_posz.PNG");

	cube_trimesh_obj.LoadFromFileObj("Objects/OtherObjs/cube.obj", true, &std::cout);
	cube_trimesh_obj.ComputeNormals();

	for (int i = 0; i < cube_trimesh_obj.NF(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			unsigned int index = cube_trimesh_obj.F(i).v[j];
			cy::Point3f vertex = cube_trimesh_obj.V(index);
			cube_v_indices.push_back(vertex);
		}
	}

	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	glGenBuffers(1, &cube_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f)*cube_v_indices.size(), &cube_v_indices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	cubeTexID = loadCubeMapTextures(cube_faces);

	glBindVertexArray(0);
}

void initialize()
{
	rotation_angle = 0.0f;
	axis_in_camera_coord = vmath::vec3(0.0f, 1.0f, 0.0f);
	projection = vmath::perspective(20.0f, (float)window_width / (float)window_height, 0.1f, 1000.0f);
	rotationMatrix *= vmath::rotate(vmath::degrees(rotation_angle), axis_in_camera_coord);

	reflection_rotation_angle = 0.0f;
	reflection_axis_in_camera_coord = vmath::vec3(0.0f, 1.0f, 0.0f);
	reflectionProjection = vmath::perspective(20.0f, (float)window_width / (float)window_height, 0.1f, 1000.0f);
	reflectionRotationMatrix *= vmath::rotate(vmath::degrees(reflection_rotation_angle), reflection_axis_in_camera_coord);
	reflectionTranslationMatrix *= vmath::translate(0.0f, -1.0f, 0.0f); //Change: comment this out

	plane_rotation_angle = 90.0f;
	plane_axis_in_camera_coord = vmath::vec3(1.0f, 0.0f, 0.0f);
	planeProjection = vmath::perspective(20.0f, (float)window_width / (float)window_height, 0.1f, 1000.0f);
	planeRotationMatrix *= vmath::rotate(vmath::degrees(plane_rotation_angle), plane_axis_in_camera_coord);
	planeTranslationMatrix *= vmath::translate(0.0f, -1.0f, 0.0f);


	glClearColor(0.0, 0.0, 0.0, 1.0); //black background

	trimesh_obj.LoadFromFileObj("Objects/OtherObjs/teapot.obj", true, &std::cout);
	trimesh_obj.ComputeNormals();

	for (int i = 0; i < trimesh_obj.NF(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			unsigned int index = trimesh_obj.F(i).v[j];
			cy::Point3f vertex = trimesh_obj.V(index);
			v_indices.push_back(vertex);
		}
	}
	for (int i = 0; i < trimesh_obj.NF(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			unsigned int index = trimesh_obj.FN(i).v[j];
			cy::Point3f vertex = trimesh_obj.VN(index);
			n_indices.push_back(vertex);
		}
	}

	//now we generate and bind the vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//next, we generate a vertex buffer and set the vertices as its data
	glGenBuffers(1, &v_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f)*v_indices.size(), &v_indices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &n_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, n_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f)* n_indices.size(), &n_indices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	createRenderTarget();
	createQuad();

	createCube();

}

void render()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE);

	// MATRIX CALCULATIONS:

	light_rotation_angle = 0.0f;
	light_axis_in_camera_coord = vmath::vec3(0.0f, 1.0f, 0.0f);

	if (curMouseX != prevMouseX || curMouseY != prevMouseY) {
		if (rotate) {
			vmath::vec3 prev = getArcballVector(prevMouseX, prevMouseY);
			vmath::vec3 curr = getArcballVector(curMouseX, curMouseY);
			rotation_angle = acos(fmin(1.0f, vmath::dot(prev, curr)));
			axis_in_camera_coord = vmath::cross(prev, curr);
			axis_in_camera_coord = vmath::normalize(axis_in_camera_coord);
			prevMouseX = curMouseX;
			prevMouseY = curMouseY;
			rotationMatrix *= vmath::rotate(vmath::degrees(rotation_angle), axis_in_camera_coord);
			
		}
		
		if (ctrl_flag)
		{
			vmath::vec3 prev = getArcballVector(prevMouseX, prevMouseY);
			vmath::vec3 curr = getArcballVector(curMouseX, curMouseY);
			light_rotation_angle = acos(fmin(1.0f, vmath::dot(prev, curr)));
			light_axis_in_camera_coord = vmath::cross(prev, curr);
			light_axis_in_camera_coord = vmath::normalize(light_axis_in_camera_coord);
			prevMouseX = curMouseX;
			prevMouseY = curMouseY;
			lightRotationMatrix *= vmath::rotate(vmath::degrees(light_rotation_angle), light_axis_in_camera_coord);
		}
		if (shift_flag)
		{
			//Superbible book reference:
			vmath::vec3 prev = getArcballVector(prevMouseX, prevMouseY);
			vmath::vec3 curr = getArcballVector(curMouseX, curMouseY);
			reflection_rotation_angle = acos(fmin(1.0f, vmath::dot(prev, curr)));
			reflection_axis_in_camera_coord = vmath::cross(prev, curr);
			reflection_axis_in_camera_coord = vmath::normalize(reflection_axis_in_camera_coord);
			prevMouseX = curMouseX;
			prevMouseY = curMouseY;
			reflectionRotationMatrix *= vmath::rotate(vmath::degrees(reflection_rotation_angle), reflection_axis_in_camera_coord);
		}
	}

	vmath::mat4 perspective_matrix = projection;
	vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, z_pos), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f)); //setting up camera position and viewing direction
	view_matrix *= translationMatrix;
	view_matrix *= rotationMatrix;
	vmath::mat4 model_matrix = vmath::scale(scale_factor);

	vmath::mat4 mvp = perspective_matrix * view_matrix * model_matrix;
	vmath::mat4 mv = view_matrix * model_matrix;

	////-----------TRANSFORMATIONS FOR REFLECTIVE OBJ--------
	vmath::mat4 reflection_perspective_matrix = reflectionProjection;
	vmath::mat4 reflection_view_matrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, reflection_z_pos), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f)); //setting up camera position and viewing direction
	reflection_view_matrix *= reflectionTranslationMatrix;
	reflection_view_matrix *= rotationMatrix;

	vmath::mat4 reflection_model_matrix = vmath::scale(vmath::vec3(1.0f, -1.0f, 1.0f));

	vmath::mat4 reflection_mvp = reflection_perspective_matrix * reflection_view_matrix * reflection_model_matrix;

	vmath::mat4 light_view_matrix = vmath::lookat(vmath::vec3(70.0, 50.0, 0.0), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f)); //setting up camera position and viewing direction
	light_view_matrix *= lightRotationMatrix;
	vmath::mat4 light_model_matrix = vmath::scale(scale_factor);

	vmath::mat4 mv_light = light_view_matrix * light_model_matrix;

	cy::Point3<float> light_pos(-100.0, 100.0, 100.0);

	////-----------REFLECTIVE OBJECT-------------------------
	setReflectiveShaders();
	glrb_obj.Bind(); //Binding framebuffer to render into it.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glsl_reflective_program_obj.Bind();

	//Sending the mvp matrix to the vertex shader:
	locationID = glGetUniformLocation(reflective_program, "projection");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_perspective_matrix[0][0]);
	locationID = glGetUniformLocation(reflective_program, "view");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_view_matrix[0][0]);
	locationID = glGetUniformLocation(reflective_program, "model");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_model_matrix[0][0]);

	locationID = glGetUniformLocation(reflective_program, "light_matrix");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &mv_light[0][0]);

	locationID = glGetUniformLocation(reflective_program, "light_pos");
	glUniform3f(locationID, light_pos[0], light_pos[1], light_pos[2]);

	locationID = glGetUniformLocation(reflective_program, "camera");
	glUniform3f(locationID, 0.0f, 0.0f, reflection_z_pos);

	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, n_buffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);

	glDrawArrays(GL_TRIANGLES, 0, v_indices.size());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////////------REFLECTIVE OBJECT RENDERING----------------------------
	reflection_model_matrix = vmath::scale(vmath::vec3(1.0f, 1.0f, 1.0f));

	setReflectiveShaders();
	glsl_reflective_program_obj.Bind();

	//Sending the mvp matrix to the vertex shader:
	locationID = glGetUniformLocation(reflective_program, "projection");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_perspective_matrix[0][0]);

	locationID = glGetUniformLocation(reflective_program, "view");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_view_matrix[0][0]);

	locationID = glGetUniformLocation(reflective_program, "model");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &reflection_model_matrix[0][0]);

	locationID = glGetUniformLocation(reflective_program, "light_matrix");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &mv_light[0][0]);

	locationID = glGetUniformLocation(reflective_program, "light_pos");
	glUniform3f(locationID, light_pos[0], light_pos[1], light_pos[2]);

	locationID = glGetUniformLocation(reflective_program, "camera");
	glUniform3f(locationID, 0.0f, 0.0f, reflection_z_pos);

	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, n_buffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);
	glDrawArrays(GL_TRIANGLES, 0, v_indices.size());
	//// ------------------------------------------PLANE------------------------------------------------

	if (plane_curMouseX != plane_prevMouseX || plane_curMouseY != plane_prevMouseY) {
		if (plane_rotate) {
			//Superbible book reference:
			vmath::vec3 prev = getArcballVector(plane_prevMouseX, plane_prevMouseY);
			vmath::vec3 curr = getArcballVector(plane_curMouseX, plane_curMouseY);
			plane_rotation_angle = acos(fmin(1.0f, vmath::dot(prev, curr)));
			plane_axis_in_camera_coord = vmath::cross(prev, curr);
			plane_axis_in_camera_coord = vmath::normalize(plane_axis_in_camera_coord);
			plane_prevMouseX = plane_curMouseX;
			plane_prevMouseY = plane_curMouseY;
			planeRotationMatrix *= vmath::rotate(vmath::degrees(plane_rotation_angle), plane_axis_in_camera_coord);
		}
		if (plane_zoom) {
			plane_z_pos += (plane_curMouseY - plane_prevMouseY);
			if (plane_z_pos > 1000)
			{
				plane_z_pos = 1000;
			}
			else if (plane_z_pos < 10)
			{
				plane_z_pos = 10;
			}
			plane_prevMouseX = plane_curMouseX;
			plane_prevMouseY = plane_curMouseY;
		}
	}

	vmath::mat4 plane_perspective_matrix = planeProjection;
	// Change: vec3 70.0f, 50.0f, 0.0f from 0,0,plane_z_pos
	vmath::mat4 plane_view_matrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, plane_z_pos), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f)); //setting up camera position and viewing direction
	plane_view_matrix *= planeTranslationMatrix;
	plane_view_matrix *= planeRotationMatrix;
	vmath::mat4 plane_model_matrix = vmath::scale(plane_scale_factor); // Change: added rotation

	vmath::mat4 plane_mvp = plane_perspective_matrix * plane_view_matrix * plane_model_matrix;
	vmath::mat4 plane_mv = plane_view_matrix * plane_model_matrix;

	////----------PLANE--------------------------
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setPlaneShaders();
	glsl_plane_program_obj.Bind();

	GLuint planeTexID = glGetUniformLocation(program2, "renderTex");
	glUniform1i(planeTexID, 0);
	glrb_obj.BindTexture(0);

	locationID = glGetUniformLocation(program2, "projection");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &plane_perspective_matrix[0][0]);
	locationID = glGetUniformLocation(program2, "view");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &plane_view_matrix[0][0]);
	locationID = glGetUniformLocation(program2, "model");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &plane_model_matrix[0][0]);

	locationID = glGetUniformLocation(program2, "camera");
	glUniform3f(locationID, 0.0f, 0.0f, -10.0f);//-10


	glBindVertexArray(plane_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);


	glBindBuffer(GL_ARRAY_BUFFER, quad_n_buffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);



	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	/////////-------CUBE FOR ENVIRONMENT MAP -----------------------------

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glCullFace(GL_FRONT);
	glDepthMask(GL_FALSE); //disabling writing to depth buffer

	setCubeShaders();
	glsl_cube_program_obj.Bind();

	locationID = glGetUniformLocation(cube_program, "mvp_matrix");
	glUniformMatrix4fv(locationID, 1, GL_FALSE, &mvp[0][0]);

	glBindVertexArray(cube_vao);
	glActiveTexture(GL_TEXTURE0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);




	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv); //to initialize GLUT
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(window_height, window_width);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CS6610 Project 6 - Environment Mapping");
	glewInit();
	glutDisplayFunc(render);
	glutKeyboardFunc(keys);
	glutSpecialFunc(special_keys);
	glutMouseFunc(mouseButtonPress);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	initialize();
	glutMainLoop();
	return 0;
}
