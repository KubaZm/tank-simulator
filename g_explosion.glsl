#version 330

layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices = 3) out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float amount;

in vec4 gVertex[];
in vec4 gColor[];
out vec4 iColor;

void main(void) {
	int i;

	vec3 a = (gVertex[1] - gVertex[0]).xyz;
	vec3 b = (gVertex[2] - gVertex[0]).xyz;

	vec4 n = vec4(normalize(cross(b, a)), 0 );

	for (i = 0; i < gl_in.length(); i++) {
		gl_Position = P * V * M * (gVertex[i] + amount * n);
		iColor = gColor[i];
		EmitVertex();
	}

	EndPrimitive();
}







/*
##########################################################################

#version 330

layout(triangles) in;
layout(line_strip) out;
layout(max_vertices = 6) out;

in vec4 gColor[];
out vec4 iColor;

void main(void) {
	int i;

	vec4 A = gl_in[0].gl_Position;
	vec4 B = gl_in[1].gl_Position;
	vec4 C = gl_in[2].gl_Position;

	vec4 cA = gColor[0];
	vec4 cB = gColor[0];
	vec4 cC = gColor[0];

	vec4 D = (A + B + C) / 3;

	gl_Position = A;
	iColor = cA;
	EmitVertex();
	gl_Position = D;
	iColor = cA;
	EmitVertex();
	EndPrimitive();

	gl_Position = B;
	iColor = cB;
	EmitVertex();
	gl_Position = D;
	iColor = cB;
	EmitVertex();
	EndPrimitive();

	gl_Position = C;
	iColor = cC;
	EmitVertex();
	gl_Position = D;
	iColor = cC;
	EmitVertex();
	EndPrimitive();

}

####################################################################
*/