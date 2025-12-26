#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR - SHADER: FILE NOT SUCESSFULLY READ." << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	unsigned int vertex, fragment;
	
	vertex = compile(GL_VERTEX_SHADER, vShaderCode);
	checkShader(GL_VERTEX_SHADER, vertex);

	fragment = compile(GL_FRAGMENT_SHADER, fShaderCode);
	checkShader(GL_FRAGMENT_SHADER, fragment);

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	int success;
	char infoLog[512];
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR - SHADER: PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use()
{
	glUseProgram(ID);
}

unsigned int Shader::compile(int type, const char* source)
{
	unsigned int shaderId;
	shaderId = glCreateShader(type);
	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);

	return shaderId;
}

void Shader::checkShader(int type, unsigned int shaderID)
{
	int success;
	char infoLog[512];

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
		if (type == GL_VERTEX_SHADER)
			std::cout << "ERROR - SHADER: VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		else
			std::cout << "ERROR - SHADER: FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	};
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value1, float value2) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), value1, value2);
}

void Shader::setFloat(const std::string& name, float value1, float value2, float value3) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3);
}

void Shader::setFloat(const std::string& name, float value1, float value2, float value3, float value4) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3, value4);
}

void Shader::setMat(const std::string& name, float* value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

float Shader::getFloat(const std::string& name)
{
	GLint uniformLocation = glGetUniformLocation(ID, name.c_str());
	float value;
	glGetUniformfv(ID, uniformLocation, &value);

	return value;
}
