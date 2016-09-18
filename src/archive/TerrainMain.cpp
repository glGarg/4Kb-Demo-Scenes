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

void readFile(string file, string &source){
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
		readFile(file, source);
		const char *s = source.c_str();
		glShaderSource(shader, 1, &s, NULL);
		compile();
		if(false){
			//error log
		}
	}
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
	operator GLuint(){
		return shader;
	}
private:
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

class Camera{
public:
	Camera(glm::vec3 pos) : position(pos), center(pos + glm::vec3(0., 0., 1.)/*glm::vec3(0.f)*/){
		view = glm::angleAxis(-22.f/21.f, glm::vec3(1.f, 0., 0.)) * glm::quat_cast(glm::translate(position/*glm::vec3(0., 10., 0.)*/));
		//view *= glm::angleAxis(-22.f/21.f, glm::vec3(1.f, 0., 0.));
		//view = glm::quat_cast(glm::lookAt(position, glm::vec3(0.), glm::vec3(0, 1.f, 0)));
		//view = normalize(view);
	}
	~Camera(){}
	/*void move(glm::vec3 change){
		position += change;
		updateViewMatrix(change);
	}*/
	void move(glm::vec3 delta) {
		auto tmp = glm::mat4_cast(view);
		auto x = glm::vec3(tmp[0][2], tmp[1][2], tmp[2][2]);
		auto y = -glm::vec3(tmp[0][0], tmp[1][0], tmp[2][0]);
		auto z = -glm::vec3(tmp[0][1], tmp[1][1], tmp[2][1]);
		position += /*delta*/x*delta.x + y*delta.y + z*delta.z;
		//center += delta;
		center = position + glm::vec3(0., 0., 1.);
		updateViewMatrix(delta);
	}
	void updateViewMatrix(glm::vec3 change){
		glm::mat4 temp = glm::mat4_cast(view);
		temp = temp*glm::translate(change);
		view = glm::quat_cast(temp);
		view = normalize(view);
		view = glm::quat_cast(glm::lookAt(position, position + glm::vec3(center-position), glm::vec3(0.f, 1.f, 0)));
	}
	glm::mat4 getView(){
		//return glm::lookAt(position, center, glm::vec3(0, 1.f, 0));
		return glm::mat4_cast(view);//glm::translate(glm::mat4_cast(view), position);
	}
	glm::vec3 getPosition(){
		return position;
	}
private:
	glm::vec3 position;
	glm::quat view;
	glm::vec3 center;
};
/*class Camera{
public:
	Camera(glm::vec3 pos) : position(pos){
		view = glm::translate(view, position);
	}
	~Camera(){}
	void move(glm::vec3 change){
		position += change;
		updateViewMatrix(change);
	}
	void updateViewMatrix(glm::vec3 change){
		glm::quat temp;
		//view = glm::translate(view, change);
		view = glm::lookAt(position, glm::vec3(0.), glm::vec3(0, 1.f, 0));
	}
	glm::mat4 getView(){
		return glm::mat4(view);
	}
	glm::vec3 getPosition(){
		return position;
	}
private:
	glm::vec3 position;
	glm::mat4 view;
};*/

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
Program *lighting_program = NULL;
//Camera camera(glm::vec3(10.1, 10., 10.1));
Camera camera(glm::vec3(0.1, 0., 0.1));
glm::vec2 windowSize = glm::vec2(400, 400);
GLint normal_uniform_lighting;
GLint mvp_uniform_lighting;
GLint eye_uniform_lighting;

void removeBackslash(std::istringstream &line){
	string str = line.str();
	for(int i = 0; i < str.size(); i++){
		if(str[i] == '/')
			str[i] = ' ';
	}
	line = std::istringstream(str);
}

void loadObj(string file, vector<float> &vertices, vector<float> &normals){
	ifstream objFile(file.c_str());
	string temp;
	vector<float> verticesRaw, normalsRaw;
	vector<int> vertexIndices, normalIndices;
	while(getline(objFile, temp)){
		std::istringstream line(temp);
		string keyword;
		float x, y, z;
		line >> keyword >> x >> y >> z;
		if(keyword == "vn"){
			normalsRaw.push_back(x);
			normalsRaw.push_back(y);
			normalsRaw.push_back(z);
		}
		else if(keyword == "v"){
			verticesRaw.push_back(x);
			verticesRaw.push_back(y);
			verticesRaw.push_back(z);
		}
		else if(keyword == "f"){
			int vx, vy, vz, nx, ny, nz;
			removeBackslash(line);
			line >> keyword >> vx >> nx >> vy >> ny >> vz >> nz;
			vertexIndices.push_back(vx);
			vertexIndices.push_back(vy);
			vertexIndices.push_back(vz);
			normalIndices.push_back(nx);
			normalIndices.push_back(ny);
			normalIndices.push_back(nz);
			/*vx = (vx > 1520) ? 1520 : vx;
			vy = (vy > 1520) ? 1520 : vy;
			vz = (vz > 1520) ? 1520 : vz;
			nx = (nx > 2873) ? 2873 : nx;
			ny = (ny > 2873) ? 2873 : ny;
			nz = (nz > 2873) ? 2873 : nz;*/
			vertices.push_back(verticesRaw[vx]);
			vertices.push_back(verticesRaw[vy]);
			vertices.push_back(verticesRaw[vz]);
			normals.push_back(normalsRaw[nx]);
			normals.push_back(normalsRaw[ny]);
			normals.push_back(normalsRaw[nz]);
		}
		else break;
	}
}

void updateCamera(){
	if(lighting_program){
		glUseProgram(*lighting_program);
		glUniform3fv(eye_uniform_lighting, 1, glm::value_ptr(camera.getPosition()));
		glm::mat4 mvpMatrix = glm::perspective(glm::radians(100.0f), windowSize.x/windowSize.y
			, .1f, 120.0f) * camera.getView() * glm::mat4(1.f);
		/*glm::mat4 temp = camera.getView();
		cout << glm::to_string(temp[0]) << endl;
		cout << glm::to_string(temp[1]) << endl;
		cout << glm::to_string(temp[2]) << endl;
		cout << glm::to_string(temp[3]) << endl;*/
		glUniformMatrix4fv(mvp_uniform_lighting, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(glm::mat4(1.)*camera.getView()));
		glUniformMatrix4fv(normal_uniform_lighting, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUseProgram(0);
	}
}

static void keyboardInputFunc(GLFWwindow *window, int key, int, int action, int){
	if(true/*action == GLFW_RELEASE*/){
		switch(key){
			case (GLFW_KEY_UP):
				camera.move(glm::vec3(0., 0., .1f));
				//cout << camera.getPosition().z;
				updateCamera();
				break;
			case (GLFW_KEY_DOWN):
				camera.move(glm::vec3(0., 0., -.1f));
				updateCamera();
				break;
			case (GLFW_KEY_RIGHT):
				camera.move(glm::vec3(.1f, 0., 0.f));
				updateCamera();
				break;
			case (GLFW_KEY_LEFT):
				camera.move(glm::vec3(-.1f, 0., 0.f));
				updateCamera();
				break;
			case (GLFW_KEY_SPACE):
				camera.move(glm::vec3(0., -0.1, .0f));
				updateCamera();
				break;
			case (GLFW_KEY_RIGHT_SHIFT):
				camera.move(glm::vec3(0., 0.1, .0f));
				updateCamera();
				break;
			default:
				break;
		}
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//glfwSwapBuffers(window);
	}
}

int main(){
	glm::vec2 map_size = glm::vec2(500.f, 500.f);
	vector<glm::vec2> plot(int(6*map_size.x*map_size.y));

	if(!glfwInit()) return -1;
	
	GLFWwindow *window = glfwCreateWindow(windowSize.x, windowSize.y, "Terrain", NULL, NULL);
	if(!window) return -2;

	glfwMakeContextCurrent(window);

	if(glewInit()) return -3;

	glm::mat4 view = camera.getView();

	//Shaders
	Shader *lighting_vertex = new Shader("./assets/lighting.vert", GL_VERTEX_SHADER);
	lighting_vertex->create();
	lighting_vertex->compile();
	Shader *lighting_fragment = new Shader("./assets/lighting.frag", GL_FRAGMENT_SHADER);
	lighting_fragment->create();
	lighting_fragment->compile();
	Shader *pixel_vertex = new Shader("./assets/pixel.vert", GL_VERTEX_SHADER);
	pixel_vertex->create();
	pixel_vertex->compile();
	Shader *pixel_fragment = new Shader("./assets/pixel.frag", GL_FRAGMENT_SHADER);
	pixel_fragment->create();
	pixel_fragment->compile();
	//Programs
	lighting_program = new Program();
	lighting_program->attach(*lighting_vertex);
	lighting_program->attach(*lighting_fragment);
	lighting_program->link();
	Program *pixel_program = new Program();
	pixel_program->attach(*pixel_vertex);
	pixel_program->attach(*pixel_fragment);
	pixel_program->link();

	for(int i = 0; i < map_size.x; i++)
		for(int j = 0; j < map_size.y; j++){
			plot[6*(i*map_size.x + j)+0] = .03f*(glm::vec2(i, j) - .5f*map_size);
			plot[6*(i*map_size.x + j)+1] = .03f*(glm::vec2(i, j+1) - .5f*map_size);
			plot[6*(i*map_size.x + j)+2] = .03f*(glm::vec2(i+1, j) - .5f*map_size);
			plot[6*(i*map_size.x + j)+3] = .03f*(glm::vec2(i+1, j) - .5f*map_size);
			plot[6*(i*map_size.x + j)+4] = .03f*(glm::vec2(i, j+1) - .5f*map_size);
			plot[6*(i*map_size.x + j)+5] = .03f*(glm::vec2(i+1, j+1) - .5f*map_size);
		}

	glUseProgram(*lighting_program);

	eye_uniform_lighting = glGetUniformLocation(*lighting_program, "eye");
	glUniform3fv(eye_uniform_lighting, 1, glm::value_ptr(camera.getPosition()));

	glm::mat4 mvpMatrix = glm::perspective(glm::radians(45.0f), windowSize.x/windowSize.y
		, .1f, 20.0f) * camera.getView() * glm::mat4(1.f);
	mvp_uniform_lighting = glGetUniformLocation(*lighting_program, "MVP");
	glUniformMatrix4fv(mvp_uniform_lighting, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

	glm::mat4 normalMatrix = glm::transpose(glm::inverse(glm::mat4(1.)*camera.getView()));
	normal_uniform_lighting = glGetUniformLocation(*lighting_program, "NormalMatrix");
	glUniformMatrix4fv(normal_uniform_lighting, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	GLuint vbo, vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*plot.size(), plot.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glEnableVertexAttribArray(0);
	
	glUseProgram(*pixel_program);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLint frame_uniform_pixel = glGetUniformLocation(*pixel_program, "frame");
	glProgramUniform1i(*pixel_program, frame_uniform_pixel, 2);

	GLuint frame_texture;
	glGenTextures(1, &frame_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frame_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);

	float position[] = {
		-1.f,  1.f,
		 1.f, -1.f,
		-1.f, -1.f,
		 1.f,  1.f,
		 1.f, -1.f,
		-1.f,  1.f
	};
	float texcoord[] = {
		0.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
		1.f, 1.f,
		1.f, 0.f,
		0.f, 1.f,
	};

	GLuint framebuffer_vao, framebuffer_vbo[2];
	glGenVertexArrays(1, &framebuffer_vao);
	glBindVertexArray(framebuffer_vao);

	glGenBuffers(2, framebuffer_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, attachments);

	glfwSetKeyCallback(window, keyboardInputFunc);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	while(!glfwWindowShouldClose(window)){
		glViewport(0, 0, windowSize.x, windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(*lighting_program);

		glUniform3fv(eye_uniform_lighting, 1, glm::value_ptr(camera.getPosition()));
		glm::mat4 mvpMatrix = glm::perspective(glm::radians(45.0f), windowSize.x/windowSize.y
			, .1f, 200.0f) * camera.getView() * glm::mat4(1.f);
		glUniformMatrix4fv(mvp_uniform_lighting, 1, GL_FALSE, glm::value_ptr(camera.getView()));
		glUniformMatrix4fv(normal_uniform_lighting, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(0., 0., 500, 500);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(glm::vec2)*plot.size());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(*pixel_program);
		glBindVertexArray(framebuffer_vao);
		glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vbo[0]);
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