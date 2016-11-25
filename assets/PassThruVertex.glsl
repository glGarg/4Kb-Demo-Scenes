layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
smooth out vec2 uv;
smooth out vec2 texCoord;
void main() {
    uv = position;
    texCoord = texcoord;
    gl_Position = vec4(uv, 0., 1.);
}