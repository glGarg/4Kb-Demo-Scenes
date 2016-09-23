#include "Shader.hpp"
#include "Program.hpp"

Program::Program(){
	program = glCreateProgram();
}
Program::~Program(){
	glDeleteProgram(program);
}
void Program::attach(Shader &shader){
	glAttachShader(program, shader);
}
void Program::link(){
	glLinkProgram(program);
}
void Program::use(){
	glUseProgram(program);
}