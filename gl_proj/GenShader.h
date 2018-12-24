#pragma once
#include <GL/glew.h>
#include <string>

class GenShader {
public:
	//GenShader();
	//~GenShader();

	void generateProgramObject();
	void attachComputeShader(const char *filePath);
	GLint getUniformLocation(const char *name);

private:
	GLuint progHandle;
	GLuint computeShader;

	std::string extractSourceCode(const char *filePath);
	std::string toErrorName(GLenum  error);
};