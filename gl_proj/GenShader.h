#pragma once
#include <GL/glew.h>
#include <string>

class GenShader {
public:
	//GenShader();
	//~GenShader();

	void generateProgramObject();
	void attachComputeShader(const char *filePath);
	void useProgram();
	GLint getUniformLocation(const char *name);

private:
	GLuint progHandle = 100;
	GLuint computeShader = 100;

	std::string extractSourceCode(const char *filePath);
	std::string toErrorName(GLenum  error);
};