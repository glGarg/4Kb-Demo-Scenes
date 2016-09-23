#include "Shader.hpp"

void readShaderFile(string file, string &source){
	ifstream input;
	input.open(file.c_str(), std::ifstream::in);
	if(!input.good())
		cout << "Shader file is not loaded. Please check address." << endl;
	string user;
	while(std::getline(input, user)){
		source += user + "\n";
	}
}

Shader::~Shader(){
	glDeleteShader(shader);
}

void Shader::create(){
	shader = glCreateShader(type);
	string source;
	readShaderFile(file, source);
	const char *s = source.c_str();
	glShaderSource(shader, 1, &s, NULL);
	compile();
}

void Shader::compile(){
	glCompileShader(shader);
	GLint length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	std::vector<GLchar> errorLog(length);
	glGetShaderInfoLog(shader, length, &length, &errorLog[0]);
	int i = 0;
	while(errorLog[i]){
		std::cout << errorLog[i++];
	}
}
