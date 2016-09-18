layout(location = 0) in vec2 position;
smooth out vec2 uv;
void main()
{
	uv = position;
	gl_Position = vec4(uv, 0., 1.);
}