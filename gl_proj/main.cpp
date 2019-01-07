#pragma warning(disable:4996)
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "GenShader.h"

using namespace std;
using namespace glm;

int w, h, c;
static const int NUM_CTRL_PTS_X = 20, NUM_CTRL_PTS_Y = 20;

string toErrorName(GLenum  error) {
	switch (error) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		default:
			return "other";
	}
}

void checkError(const char * function_name) {
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << function_name << " error: " << toErrorName(err) << endl;
	}
}

void fillUpperTriangle(vec2 v1, vec2 v2, vec2 v3, unsigned char* ptr_map, int index) {
	vec2 v1To3 = v3 - v1;
	vec2 v2To3 = v3 - v2;
	for (int i = v1.y; i < v3.y; i++) {
		float t1 = (i - v1.y) / v1To3.y;
		float t2 = (i - v2.y) / v2To3.y;
		for (int j = ceil(v1.x + t1 * v1To3.x); j < v2.x + t2 * v2To3.x; j++) {
			ptr_map[(h-i)* w + j] = (unsigned char)index;
		}
	}
}

void fillLowerTriangle(vec2 v1, vec2 v2, vec2 v3, unsigned char* ptr_map, int index) {
	vec2 v1To3 = v3 - v1;
	vec2 v2To3 = v3 - v2;
	for (int i = v1.y; i > v3.y; i--) {
		float t1 = (i - v1.y) / v1To3.y;
		float t2 = (i - v2.y) / v2To3.y;
		for (int j = ceil(v1.x + t1 * v1To3.x); j < v2.x + t2 * v2To3.x; j++) {
			ptr_map[(h-i) * w + j] = (unsigned char)index;
		}
	}
}

void fillTriangle(vec2 v[3], unsigned char* ptr_map, int index) {
	std::sort(v, v+3, [](const vec2& v1, const vec2& v2) -> bool {
		return v1.y < v2.y;
	});
	vec2 v0ToV2 = v[2] - v[0];
	float t = (v[1] - v[0])[1] / v0ToV2.y;
	vec2 v4 = v[0] + t * v0ToV2;
	if (v[1].x < v4.x) {
		fillUpperTriangle(v[1], v4, v[2], ptr_map, index);
		fillLowerTriangle(v[1], v4, v[0], ptr_map, index);
	}
	else {
		fillUpperTriangle(v4, v[1], v[2], ptr_map, index);
		fillLowerTriangle(v4, v[1], v[0], ptr_map, index);
	}
}

unsigned char* getUvPtrs(int sizeX, int sizeY) {
	unsigned char* uvPtrs = new unsigned char[sizeX*sizeY*4];
	for (int i = 0; i < sizeX; i++) {
		for (int j = 0; j < sizeY; j++) {
			uvPtrs[(sizeX * j + i) * 4] = (unsigned char) 0;
		}
	}
	return uvPtrs;
}

void createCtrlGrid(float *map, int n_x, int n_y, vec2 botleft, vec2 topright) {
	double w = topright.x - botleft.x;
	double h = topright.y - botleft.y;
	float r = ((topright.x - botleft.x) / (float)n_x + (topright.y - botleft.y) / (float)n_y) / 2.0f;
	for (int j = 0; j < n_y; j++) {
		for (int i = 0; i < n_x; i++) {
			map[(j * n_x + i) * 4] = botleft.x + w * i / (n_x - 1.0);
			map[(j * n_x + i) * 4 + 1] = botleft.y + h * j / (n_y - 1.0);
			map[(j * n_x + i) * 4 + 2] = 0.0f;
			map[(j * n_x + i) * 4 + 3] = r;
		}
	}
}

void renderGrid(unsigned char* origin_img, float *map) {
	unsigned char* img;
	img = (unsigned char*)malloc(w*h*c);
	memcpy(img, origin_img, w*h*c);
	/* draw ctrl points as red dots*/
	for (int i = 0; i < NUM_CTRL_PTS_Y; i++) {
		for (int j = 0; j < NUM_CTRL_PTS_X; j++) {
			int posX = map[(NUM_CTRL_PTS_X * i + j) * 4];
			int posY = map[(NUM_CTRL_PTS_X * i + j) * 4 + 1];
			if (posX == 0 || posY == 0) {
				continue;
			}
			for (int m = posX; m < posX + 3; m++) {
				for (int n = posY; n < posY + 3; n++) {
					int index = ((h - n) * w + m)*c;
					img[index] = (unsigned char)255;
					img[index + 1] = (unsigned char)0;
					img[index + 2] = (unsigned char)0;
					img[index + 3] = (unsigned char)255;
				}
			}
		}
	}
	stbi_write_png("grid.png", w, h, c, img, 0);
	free(img);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutCreateWindow("GLEW Test");
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	if (glewInit()) {
		cout << "GLEW not initialized";
		return 1;
	}
	GenShader m_shader;
	GenShader m_shader_ctrlpoints;
	m_shader_ctrlpoints.generateProgramObject();
	m_shader_ctrlpoints.attachComputeShader("shaders/move_ctrl_points.cs");

	m_shader.generateProgramObject();
	m_shader.attachComputeShader("shaders/computeShader.cs");

	/* prepare data */
	unsigned char* image_data = stbi_load("watch.png", &w, &h, &c, STBI_rgb_alpha);
	cout << "c: " << c << endl;
	cout << "image size: " << strlen((char *)image_data) << endl;
	if (image_data == nullptr) {
		throw(std::string("Failed to load texture"));
	}
	unsigned char* output_data = new unsigned char[w*h*c];

	/* create grid */
	float ctrl_pts[NUM_CTRL_PTS_X * NUM_CTRL_PTS_Y * 4];
	float out_ctrl_pts[NUM_CTRL_PTS_X * NUM_CTRL_PTS_Y * 4];
	createCtrlGrid(ctrl_pts, NUM_CTRL_PTS_X, NUM_CTRL_PTS_Y, vec2(20, 325), vec2(720, 1024));
	//renderGrid(image_data, ctrl_pts);

	/* pass circle to shader */
	m_shader_ctrlpoints.useProgram();
	GLint circle_center_loc = m_shader_ctrlpoints.getUniformLocation("circle.center");
	GLint circle_radius_loc = m_shader_ctrlpoints.getUniformLocation("circle.radius");
	checkError("getUniformLocation");
	glUniform2fv(circle_center_loc, 1, value_ptr(vec2(370.0f, 675.0f)));
	checkError("glUniform2fv");
	glUniform1f(circle_radius_loc, 350.0f);
	checkError("glUniform1f");

	/* pass grid to shader*/
	GLuint tex_ctrl_pts;
	glGenTextures(1, &tex_ctrl_pts);
	checkError("glGenTextures");
	glBindTexture(GL_TEXTURE_2D, tex_ctrl_pts);
	checkError("glBindTexture");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, NUM_CTRL_PTS_X, NUM_CTRL_PTS_Y, 0, GL_RGBA, GL_FLOAT, ctrl_pts);
	checkError("glTexImage2D");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindImageTexture(1, tex_ctrl_pts, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	checkError("glBindImageTexture");

	/* pass output image */
	GLuint tex_out_ctrl_pts;
	glGenTextures(1, &tex_out_ctrl_pts);
	glBindTexture(GL_TEXTURE_2D, tex_out_ctrl_pts);
	checkError("glBindTexture");
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, NUM_CTRL_PTS_X, NUM_CTRL_PTS_Y);
	checkError("glTexStorage2D");
	glBindImageTexture(2, tex_out_ctrl_pts, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	checkError("glBindImageTexture");

	/* invoke shader */
	int num_group_x = ceil((float)NUM_CTRL_PTS_X / 10.0f);
	int num_group_y = ceil((float)NUM_CTRL_PTS_Y / 10.0f);
	glDispatchCompute((GLuint)num_group_x, (GLuint)num_group_y, (GLuint)1);
	checkError("glDispatchCompute");

	/* get output */
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, out_ctrl_pts);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glGetTexImage error: " << err << toErrorName(err) << endl;
	}
	renderGrid(image_data, out_ctrl_pts);

	exit(0);

	unsigned char* A_ptr = new unsigned char[w*h];
	for (int i = 0; i < w * h; i++) {
		A_ptr[i] = (unsigned char)0;
	}


	// bot: 325, top:1024, left: 20, right: 720, centerx: 370, centery: 675
	// botleft, botright, topright, topleft
	//vec2 uv_ctrl_pts[4] = { vec2(20.0f, 325.0f), vec2(720.0f, 325.0f), vec2(720.0f, 1024.0f), vec2(20.0f, 1024.0f) };
	//vec2 xy_ctrl_pts[4] = { vec2(370.0f, 325.0f), vec2(720.0f, 675.0f), vec2(370.0f, 1024.0f), vec2(20.0f, 675.0f) };

	vec2 uv_ctrl_pts[4] = { vec2(10.0f, 325.0f), vec2(700.0f, 325.0f), vec2(740.0f, 1000.0f), vec2(20.0f, 900.0f) };
	vec2 xy_ctrl_pts[4] = { vec2(375.0f, 320.0f), vec2(720.0f, 650.0f), vec2(365.0f, 1020.0f), vec2(15.0f, 600.0f) };
	vec2 triangle1[3] = { xy_ctrl_pts[0], xy_ctrl_pts[1], xy_ctrl_pts[2] };
	vec2 triangle2[3] = { xy_ctrl_pts[0], xy_ctrl_pts[2], xy_ctrl_pts[3] };
	fillTriangle(triangle1, A_ptr, 1);
	fillTriangle(triangle2, A_ptr, 1);
	stbi_write_png("filled.png", w, h, 1, A_ptr, 0);

	mat4 M = mat4(1.0);
	for (int i = 0; i < 4; i++) {
		M[i] = vec4(1, xy_ctrl_pts[i].x, xy_ctrl_pts[i].y, xy_ctrl_pts[i].x*xy_ctrl_pts[i].y);
	}
	cout << "M: " << glm::to_string(M) << endl;
	cout << "inverse M: " << glm::to_string(glm::inverse(M)) << endl;
	vec4 A = vec4(uv_ctrl_pts[0].x, uv_ctrl_pts[1].x, uv_ctrl_pts[2].x, uv_ctrl_pts[3].x) * glm::inverse(M);
	vec4 B = vec4(uv_ctrl_pts[0].y, uv_ctrl_pts[1].y, uv_ctrl_pts[2].y, uv_ctrl_pts[3].y) * glm::inverse(M);
	vec4 A_array[4] = { vec4(1.0), A, vec4(1.0), vec4(1.0) };
	vec4 B_array[4] = { vec4(1.0), B, vec4(1.0), vec4(1.0) };
	cout << "test: " << (dot(A, vec4(1.0, xy_ctrl_pts[0].x, xy_ctrl_pts[0].y, xy_ctrl_pts[0].x * xy_ctrl_pts[0].y))) << endl;

	///* pass input image to shader */
	GLuint tex_in, tex_out;
	glGenTextures(1, &tex_in);
	checkError("glGenTextures");
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_in);
	checkError("glBindTexture");
	cout << "w: " << w << ", h: " << h << endl;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, w, h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8_REV, image_data);
	checkError("glTexImage2D");
	//glGetTexParameter()
	/*
	Some people make the mistake of not calling glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter); 
	and they get the default state GL_LINEAR_MIPMAP_NEAREST and they don't define the mipmaps, 
	so the texture is considered incomplete and you just get a white textur
	https://www.gamedev.net/forums/topic/500456-glteximage2d-fails-somehow/
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindImageTexture(1, tex_in, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8UI);
	checkError("glBindImageTexture");

	/* pass transform vector pointers*/
	GLuint tex_ptrs;
	glGenTextures(1, &tex_ptrs);
	checkError("glGenTextures");
	glBindTexture(GL_TEXTURE_2D, tex_ptrs);
	checkError("glBindTexture");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, A_ptr);
	checkError("glTexImage2D");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindImageTexture(3, tex_ptrs, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
	checkError("glBindImageTexture");

	/* pass output image */
	glGenTextures(1, &tex_out);
	glBindTexture(GL_TEXTURE_2D, tex_out);
	checkError("glBindTexture");
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	checkError("glTexStorage2D");
	glBindImageTexture(2, tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	checkError("glBindImageTexture");

	/* pass A, B */
	GLint A_loc = m_shader.getUniformLocation("A");
	GLint B_loc = m_shader.getUniformLocation("B");
	glUniform4fv(A_loc, 4, value_ptr(A_array[0]));
	glUniform4fv(B_loc, 4, value_ptr(B_array[0]));

	/* invoke shader */
	num_group_x = ceil((double)w / 32.0);
	num_group_y = ceil((double)h / 32.0);
	cout << "num_group_x: " << num_group_x << ", num_group_y: " << num_group_y << endl;
	glDispatchCompute((GLuint)num_group_x, (GLuint)num_group_y, (GLuint)1);
	checkError("glDispatchCompute");
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, output_data);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glGetTexImage error: " << err << toErrorName(err) << endl;
	}
	//glGetTextureImage(tex_in, 0, GL_RGBA32F, GL_UNSIGNED_BYTE, w*h * sizeof(char), output_data);
	stbi_write_png("out_image.png", w, h, c, output_data, 0);
	delete[] output_data;
	delete[] A_ptr;
	stbi_image_free(image_data);

}