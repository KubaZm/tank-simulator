#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec4 vertex;

out vec4 gColor;
out vec4 gVertex;

void main(void) {
	gColor = vec4(0.972f, 0.839f, 0.509f, 1.f);
	gVertex = vertex;
}
