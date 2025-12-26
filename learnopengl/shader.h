#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath);
	void use();
	
	void setBool(const std::string &name, bool value) const;
	
	void setInt(const std::string& name, int value) const;
	
	void setFloat(const std::string& name, float value) const;
	void setFloat(const std::string& name, float value1, float value2) const;
	void setFloat(const std::string& name, float value1, float value2, float value3) const;
	void setFloat(const std::string& name, float value1, float value2, float value3, float value4) const;
	
	void setMat(const std::string& name, float* value) const;

	float getFloat(const std::string& name);

private:
	unsigned int compile(int type, const char* source);
	void checkShader(int type, unsigned int shaderID);
};

#endif // SHADER_H