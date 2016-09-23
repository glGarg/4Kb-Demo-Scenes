#include <GL/glew.h>
#include <GL/gl.h>
#include <glfw3.h>
#include <glm.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <SOIL.h>
#include "Shader.hpp"
#include "Program.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;

#define ENABLE_CUBEMAP

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

static void windowSizeChangeFunc(GLFWwindow *window, int width, int height){
	glUniform2f(glGetUniformLocation(*program, "resolution"), width, height);
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
	Shader *fragment = new Shader("./assets/MetaBalls.glsl", GL_FRAGMENT_SHADER);
	fragment->create();
	
	program = new Program();
	program->attach(*vertex);
	program->attach(*fragment);
	program->link();

	string texturePath;
	cout << "Enter path to the texture file/cubemap directory." << endl;
	std::cin >> texturePath;

#ifndef ENABLE_CUBEMAP
	ifstream file(texturePath.c_str());
#else
	ifstream file((texturePath + "/right.jpg").c_str());
#endif

	if(!file.good()){
#ifndef ENABLE_CUBEMAP
		cout << "Problem with the address provided. Loading the default sampler2D." << endl;
		texturePath = "../textures/noise_texture_0001.png";
#else
		cout << "Problem with the address provided. Loading the default samplerCube." << endl;
		texturePath = "../textures/cubemap";
#endif
	}
	else file.close();

	int width, height;
	unsigned char* image;
	GLuint resolution_uniform_program, time_uniform_program, texture_uniform_program,
		   texture;

	glUseProgram(*program);

	resolution_uniform_program = glGetUniformLocation(*program, "resolution");
	time_uniform_program = glGetUniformLocation(*program, "time");
	texture_uniform_program = glGetUniformLocation(*program, "texture");

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);

#ifndef ENABLE_CUBEMAP
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

	SOIL_free_image_data(image);
#else
	vector<string> faces;
	faces.push_back(texturePath + "/right.jpg");
	faces.push_back(texturePath + "/left.jpg");
	faces.push_back(texturePath + "/top.jpg");
	faces.push_back(texturePath + "/bottom.jpg");
	faces.push_back(texturePath + "/back.jpg");
	faces.push_back(texturePath + "/front.jpg");

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for(int i = 0; i < faces.size(); i++){
		image = SOIL_load_image(faces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		if(!image){
			cout << "Error loading the texture file." << endl;
			return -4;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

	glfwGetWindowSize(window, &width, &height);
	glUniform1i(texture_uniform_program, 0);
	glUniform1f(time_uniform_program, glfwGetTime());
	glUniform2f(resolution_uniform_program, width, height);

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
	glfwSetWindowSizeCallback(window, windowSizeChangeFunc);
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