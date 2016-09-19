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
#include <fstream>
#include <string>
#include <sstream>
#include <SOIL.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;

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
		const char *s = source.c_str();
		glShaderSource(shader, 1, &s, NULL);
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
	if(action == GLFW_RELEASE){
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
	GLFWwindow *window = glfwCreateWindow(windowSize.x, windowSize.y, "Scene", NULL, NULL);
	if(!window)
		return -2;

	glfwMakeContextCurrent(window);

	if(glewInit())
		return -3;

	Shader *vertex = new Shader("./assets/PassThruVertex.glsl", GL_VERTEX_SHADER);
	vertex->create();
	Shader *fragment = new Shader("./assets/MusicalBalls.glsl", GL_FRAGMENT_SHADER);
	fragment->create();
	
	program = new Program();
	program->attach(*vertex);
	program->attach(*fragment);
	program->link();

	string texturePath;
	cout << "Enter path to the texture file." << endl;
	std::cin >> texturePath;

	ifstream file(texturePath.c_str());
	if(!file.good()){
		cout << "Problem with the address provided. Loading the default sampler2D." << endl;
		texturePath = "./textures/noise_texture_0001.png";
	}
	else file.close();

	int width, height;
	unsigned char* image;
	GLuint time_uniform_program, texture_uniform_program, texture;

	glUseProgram(*program);

	time_uniform_program = glGetUniformLocation(*program, "time");
	texture_uniform_program = glGetUniformLocation(*program, "texture");

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	image = SOIL_load_image(texturePath.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

	if(image == 0){
		cout << "Error loading the texture file." << endl;
		return -4;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glUniform1i(texture_uniform_program, 0);
	glUniform1f(time_uniform_program, glfwGetTime());

	SOIL_free_image_data(image);


	float position[] = {
		-1.f,  1.f,
		 1.f, -1.f,
		-1.f, -1.f,
		 1.f,  1.f,
		 1.f, -1.f,
		-1.f,  1.f
	};

	float texCoord[] = {
		 0.f,  1.f,
		 1.f,  0.f,
		 0.f,  0.f,
		 1.f,  1.f,
		 1.f,  0.f,
		 0.f,  1.f,
	};

	GLuint vao, vbo[2];
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glfwSetKeyCallback(window, keyboardInputFunc);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	while(!glfwWindowShouldClose(window)){
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(*program);
		glUniform1f(time_uniform_program, glfwGetTime());

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
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