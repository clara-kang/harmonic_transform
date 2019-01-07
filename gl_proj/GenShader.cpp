#include <stdio.h>
#include <GL/glew.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GenShader.h"

using namespace std;

string GenShader::toErrorName(GLenum  error) {
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

void GenShader::generateProgramObject() {
	GLenum err;
	progHandle = glCreateProgram();
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glCreateProgram error: " << err << toErrorName(err) << endl;
	}
}

void GenShader::attachComputeShader(const char *filePath) {
	GLenum err;
	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glCreateShader error: " << err << toErrorName(err) << endl;
	}
	//char *sourceCodeStr;
	//strcpy_s(sourceCodeStr, extractSourceCode(filePath).c_str());
	string sourceCodeStr = extractSourceCode(filePath);
	const char *sourceCode = sourceCodeStr.c_str();
	cout << "extractSourceCode: " << sourceCode << endl;
	glShaderSource(computeShader, 1, (const GLchar **)&sourceCode, NULL);
	glCompileShader(computeShader);
	int rvalue;
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(computeShader, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
		exit(40);
	}
	glAttachShader(progHandle, computeShader);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glAttachShader error: " << err << toErrorName(err) << endl;
	}
	glLinkProgram(progHandle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glLinkProgram error: " << err << toErrorName(err) << endl;
	}
	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(progHandle, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
		exit(41);
	}
	glUseProgram(progHandle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		cout << "glLinkProgram error: " << err << toErrorName(err) << endl;
	}
}

string GenShader::extractSourceCode(const char *filePath) {
	ifstream file;
	file.open(filePath);
	stringstream strBuffer;
	string str;

	while (file.good()) {
		getline(file, str, '\r');
		strBuffer << str;
	}
	file.close();
	strBuffer << '\0';  // Append null terminator.
	return strBuffer.str();

}

GLint GenShader::getUniformLocation(const char *name) {
	return glGetUniformLocation(progHandle, (const GLchar *)name);
}

void GenShader::useProgram() {
	glUseProgram(progHandle);
}