#include <iostream>
#include <string>

class Program{
public:
	Program();
	~Program();
	void attach(Shader &shader);
	void link();
	void use();
	operator GLuint(){return program;}
private:
	GLuint program;
};