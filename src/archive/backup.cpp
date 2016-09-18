#include <GL/glew.h>
#include <GL/gl.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include <gtx/transform.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <sstream>

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;
using std::ifstream;

void readShaderFile(string file, string &source){
	ifstream input(file);
	string user;
	while(std::getline(input, user)){
		source += user + "\n";
	}
}

class Shader{
public:
	Shader(string file, GLenum type) : file(file), type(type){}
	~Shader(){
		//if(!glGetShader(shader, GL_DELETE_STATUS)){
			glDeleteShader(shader);
		//}
	}
	void create(){
		shader = glCreateShader(type);
		string source;
		readShaderFile(file, source);
		glShaderSource(shader, 1, &(source.c_str()), NULL);
		compile();
	}
	operator GLuint(){return shader;}
private:
	void compile(){
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
	GLenum type;
	GLuint shader;
	string file;
};

class Program{
public:
	Program(){
		program = glCreateProgram();
	}
	~Program(){
		glDeleteProgram(program);
	}
	void attach(Shader &shader){
		glAttachShader(program, shader);
	}
	void link(){
		glLinkProgram(program);
	}
	void use(){
		glUseProgram(program);
	}
	operator GLuint(){return program;}
private:
	GLuint program;
};

Program *program = NULL;

static void keyboardInputFunc(GLFWwindow *window, int key, int, int action, int){
	if(true/*action == GLFW_RELEASE*/){
		switch(key){
			case (GLFW_KEY_UP):
				break;
			case (GLFW_KEY_DOWN):
				break;
			case (GLFW_KEY_RIGHT):
				break;
			case (GLFW_KEY_LEFT):
				break;
			case (GLFW_KEY_SPACE):
				break;
			case (GLFW_KEY_RIGHT_SHIFT):
				break;
			default:
				break;
		}
	}
}

int main(){	
	if(!glfwInit())
		return -1;

	glm::vec2 windowSize(500.f, 500.f);
	GLFWwindow *window = glfwCreateWindow(windowSize.x, windowSize.y, "Terrain", NULL, NULL);
	if(!window)
		return -2;

	glfwMakeContextCurrent(window);

	if(glewInit())
		return -3;

	//Shaders
	Shader *vertex = new Shader("../PassThruVertex.glsl", GL_VERTEX_SHADER);
	vertex->create();
	Shader *fragment = new Shader("../fragment.glsl", GL_FRAGMENT_SHADER);
	fragment->create();
	
	//Programs
	program = new Program();
	program->attach(*vertex);
	program->attach(*fragment);
	program->link();

	glUseProgram(*program);

	float position[] = {
		-1.f,  1.f,
		 1.f, -1.f,
		-1.f, -1.f,
		 1.f,  1.f,
		 1.f, -1.f,
		-1.f,  1.f
	};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glfwSetKeyCallback(window, keyboardInputFunc);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	while(!glfwWindowShouldClose(window)){
		glViewport(0, 0, windowSize.x, windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(*program);
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	return 0;
}