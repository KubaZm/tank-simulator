#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lp1;
uniform vec4 lp2;
uniform vec4 lp3;
uniform vec4 lp4;
uniform vec4 lp5;

//Atrybuty
in vec4 vertex;
in vec4 normal;
in vec2 texCoord0;

//Zmienna interpolowana
out vec4 n;
out vec4 v;
out vec4 l1;
out vec4 l2;
out vec4 l3;
out vec4 l4;
out vec4 l5;
out vec2 iTexCoord0;

void main(void) {
    l1 = normalize(V*lp1 - V*M*vertex); // znormalizowany wektor l (do światła 1)
    l2 = normalize(V*lp2 - V*M*vertex); // znormalizowany wektor l (do światła 2)
    l3 = normalize(V*lp3 - V*M*vertex); // znormalizowany wektor l (do światła 3)
    l4 = normalize(V*lp4 - V*M*vertex); // znormalizowany wektor l (do światła 4)
    l5 = normalize(V*lp5 - V*M*vertex); // znormalizowany wektor l (do światła 5)
    n = normalize(V*M*normal); // znormalizowany wektor n (wektor normalny)
    v = normalize(vec4(0,0,0,1)-V*M*vertex); // znormalizowany wektor v (do obserwatorna)

    iTexCoord0 = texCoord0;

    gl_Position=P*V*M*vertex;
}
