#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;

void readShaderFile(string file, string &source);

class Shader{
public:
	Shader(string file, GLenum type) : file(file), type(type){}
	~Shader();
	void create();
	operator GLuint(){return shader;}
private:
	void compile();
	GLenum type;
	GLuint shader;
	string file;
};