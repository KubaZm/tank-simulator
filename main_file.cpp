#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

// stałe
const float cooldown = 10;
const float tankShellSpeed = 2;
const float max_tank_speed = 3;
const float wheel_radius = 0.27;
const float gear_radius = 0.34;
const float max_tank_turn_speed = PI / 3;
const int chunkNum = 4; // liczba renderowanych chunków = ((size*2)+1)^2 
const int chunkSize = 32; // wielkość chunków

float window_width = 1300;
float window_height = 800;
float aspectRatio = window_width / window_height;

ShaderProgram *sp;
ShaderProgram *skyBoxShader;
ShaderProgram *explosionShader;

GLuint texTank[5]; //Uchwyt – czołg
GLuint texTrack[10]; //Uchwyt – gąsienice
GLuint texWorld[6]; //Uchwyt – świat

// zmienne globalne
float tank_speed = 0;
float wheel_speed = 0;
float max_wheel_speed = max_tank_speed / wheel_radius; //omega = v/r
float gear_speed = 0;
float max_gear_speed = max_tank_speed / gear_radius; //omega = v/r
float tank_turn_speed = 0;
float turret_speed_x = 0;
float turret_speed_y = 0;
bool w_pressed = false;
bool s_pressed = false;
bool d_pressed = false;
bool a_pressed = false;
bool shot = false;
bool explosion = false;
float amount = 1; // do explozji i geometry shadera
float shotCooldown = cooldown;
float tank_acceleration_ratio = 0.05;
unsigned int camera = 6;

// współrzędne światła
std::vector<glm::vec3> lightPos = {
	glm::vec3(75.f, 75.f, 75.f), // główne światło 
	glm::vec3(125.f, 50.f, 0.f), // dodatkowe światło 
	glm::vec3(-125.f, 50.f, 0.f), // dodatkowe światło 
	glm::vec3(0.f, 50.f, 125.f), // dodatkowe światło 
	glm::vec3(0.f, 50.f, -125.f) // dodatkowe światło 
};


float speed_x = 0;
float speed_y = 0;

// tablice modelów
static const int nr_of_models = 11;
float* models_vertices[nr_of_models];
float* models_normals[nr_of_models];
float* models_texCoords[nr_of_models];
float* models_colors[nr_of_models];
int vertexCount[nr_of_models];


//Funkcja wczytująca teksturę 
GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

void loadOBJ(const char* file_name, int nr)
{
	std::cout << "Loading file " << file_name << "..." << std::endl;
	//Vertex portions
	std::vector<glm::fvec3> vertex_positions;
	std::vector<glm::fvec2> vertex_texcoords;
	std::vector<glm::fvec3> vertex_normals;

	//Face vectors
	std::vector<GLint> vertex_position_indicies;
	std::vector<GLint> vertex_texcoord_indicies;
	std::vector<GLint> vertex_normal_indicies;

	//Vertex array
	std::vector<glm::vec3> v;
	std::vector<glm::vec2> t;
	std::vector<glm::vec3> n;
	std::vector<glm::vec3> c;

	std::stringstream ss;
	std::ifstream in_file(file_name);
	std::string line = "";
	std::string prefix = "";
	glm::vec3 temp_vec3;
	glm::vec2 temp_vec2;
	GLint temp_glint = 0;

	//Read one line at a time
	while (std::getline(in_file, line))
	{
		//Get the prefix of the line
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "v") //Vertex position
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_positions.push_back(temp_vec3);
		}
		else if (prefix == "vt")
		{
			ss >> temp_vec2.x >> temp_vec2.y;
			vertex_texcoords.push_back(temp_vec2);
		}
		else if (prefix == "vn")
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_normals.push_back(temp_vec3);
		}
		else if (prefix == "f")
		{
			int counter = 0;
			while (ss >> temp_glint)
			{
				//Pushing indices into correct arrays
				if (counter == 0)
					vertex_position_indicies.push_back(temp_glint);
				else if (counter == 1)
					vertex_texcoord_indicies.push_back(temp_glint);
				else if (counter == 2)
					vertex_normal_indicies.push_back(temp_glint);

				//Handling characters
				if (ss.peek() == '/')
				{
					++counter;
					ss.ignore(1, '/');
				}
				else if (ss.peek() == ' ')
				{
					++counter;
					ss.ignore(1, ' ');
				}

				//Reset the counter
				if (counter > 2)
					counter = 0;
			}
		}
	}

	//Build final vertex array (mesh)
	v.resize(vertex_position_indicies.size(), glm::vec3(0.f,0.f,0.f));
	t.resize(vertex_position_indicies.size(), glm::vec2(0.f, 0.f));
	n.resize(vertex_position_indicies.size(), glm::vec3(0.f, 0.f, 0.f));
	c.resize(vertex_position_indicies.size(), glm::vec3(0.f, 0.f, 0.f));

	//Load in all indices
	for (size_t i = 0; i < vertex_position_indicies.size(); ++i)
	{
		v[i] = vertex_positions[vertex_position_indicies[i] - 1];
		t[i] = vertex_texcoords[vertex_texcoord_indicies[i] - 1];
		n[i] = vertex_normals[vertex_normal_indicies[i] - 1];
		c[i] = glm::vec3(1.f, 1.f, 1.f);
	}

	models_vertices[nr] = new float[vertex_position_indicies.size() * 4];
	models_normals[nr] = new float[vertex_position_indicies.size() * 4];
	models_texCoords[nr] = new float[vertex_position_indicies.size() * 4];
	models_colors[nr] = new float[vertex_position_indicies.size() * 4];

	int ctr1 = 0, ctr2 = 0;
	for (int i = 0; i < v.size(); i++)
	{
		for (int j = 0; j <= 2; j++)
		{
			models_vertices[nr][ctr1] = v[i][j];
			models_normals[nr][ctr1] = n[i][j];
			models_colors[nr][ctr1] = c[i][j];
			ctr1++;
		}
		models_vertices[nr][ctr1] = 1.0f;
		models_normals[nr][ctr1] = 0.0f;
		models_colors[nr][ctr1] = 1.0f;
		ctr1++;

		for (int j = 0; j <= 1; j++)
		{
			models_texCoords[nr][ctr2] = t[i][j];
			ctr2++;
		}
	}

	vertexCount[nr] = vertex_position_indicies.size();

	//Loaded success
	std::cout << "Loading file " << file_name << " finished" << std::endl;
}

void deletePtrs()
{
	for (int i = 0; i < nr_of_models; i++)
		delete models_vertices[i], models_normals[i], models_texCoords[i], models_colors[i];
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_1)
			camera = 1;
		else if (key == GLFW_KEY_2)
			camera = 2;
		else if (key == GLFW_KEY_3)
			camera = 3;
		else if (key == GLFW_KEY_4)
			camera = 4;
		else if (key == GLFW_KEY_5)
			camera = 5;
		else if (key == GLFW_KEY_6)
			camera = 6;
			

		if (key == GLFW_KEY_D)
		{
			a_pressed = false;
			d_pressed = true;
			if (a_pressed)
				tank_turn_speed = 0;
			else if (s_pressed && !w_pressed)
				tank_turn_speed = max_tank_turn_speed;
			else
				tank_turn_speed = -max_tank_turn_speed;
		}
		if (key == GLFW_KEY_A)
		{
			d_pressed = false;
			a_pressed = true;
			if (d_pressed)
				tank_turn_speed = 0;
			else if (s_pressed && !w_pressed)
				tank_turn_speed = -max_tank_turn_speed;
			else
				tank_turn_speed = max_tank_turn_speed;
		}
		if (key == GLFW_KEY_W)
		{
			w_pressed = true;
			if ((d_pressed && tank_turn_speed == max_tank_turn_speed) || (a_pressed && tank_turn_speed == -max_tank_turn_speed))
				tank_turn_speed = -tank_turn_speed;
		}
		if (key == GLFW_KEY_S)
		{
			s_pressed = true;
			if (!w_pressed)
				tank_turn_speed = -tank_turn_speed;
		}
		if (key == GLFW_KEY_LEFT) turret_speed_x = PI / 3;
		if (key == GLFW_KEY_RIGHT) turret_speed_x = -PI / 3;
		if (key == GLFW_KEY_UP) turret_speed_y = -PI / 4;
		if (key == GLFW_KEY_DOWN) turret_speed_y = PI / 4;
		if (key == GLFW_KEY_SPACE && shotCooldown == cooldown) {
			shot = true;
			PlaySound(TEXT("sounds/tank shot.wav"), NULL, SND_ASYNC);
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_D)
		{
			d_pressed = false;
			if (a_pressed)
			{
				a_pressed = true;
				if (!s_pressed)
					tank_turn_speed = max_tank_turn_speed;
				else
					tank_turn_speed = -max_tank_turn_speed;
			}
			else
				tank_turn_speed = 0;
		}
		if (key == GLFW_KEY_A)
		{
			a_pressed = false;
			if (d_pressed)
			{
				d_pressed = true;
				if (!s_pressed)
					tank_turn_speed = -max_tank_turn_speed;
				else
					tank_turn_speed = max_tank_turn_speed;
			}
			else
				tank_turn_speed = 0;
		}
		if (key == GLFW_KEY_W) { w_pressed = false; }
		if (key == GLFW_KEY_S) { s_pressed = false; }
		if (key == GLFW_KEY_LEFT) turret_speed_x = 0;
		if (key == GLFW_KEY_RIGHT) turret_speed_x = 0;
		if (key == GLFW_KEY_UP) turret_speed_y = 0;
		if (key == GLFW_KEY_DOWN) turret_speed_y = 0;
	}
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(1,1,1,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	sp = new ShaderProgram("vs.glsl",NULL,"fs.glsl");
	skyBoxShader = new ShaderProgram("vs_skyBox.glsl", NULL, "fs_skyBox.glsl");
	explosionShader = new ShaderProgram("v_explosion.glsl", "g_explosion.glsl", "f_explosion.glsl");

	loadOBJ("models/Hull.obj", 0);
	loadOBJ("models/Wheels_Both.obj",1);
	loadOBJ("models/Track_Left.obj", 2);
	loadOBJ("models/Track_Right.obj", 3);
	loadOBJ("models/Single_Wheel_Left.obj", 4);
	loadOBJ("models/Single_Wheel_Right.obj", 5);
	loadOBJ("models/Gears_Both.obj", 6);
	loadOBJ("models/Turret.obj", 7);
	loadOBJ("models/Gun.obj", 8);
	loadOBJ("models/Tank_Shell.obj", 9);
	loadOBJ("models/Termanation.obj", 10);
	texTank[0] = readTexture("textures/bodyLight.png");
	texTank[1] = readTexture("textures/bodyLightSpecular.png");
	texTank[2] = readTexture("textures/lightMetal.png");
	texTank[3] = readTexture("textures/lightMetalSpecular.png");
	texTank[4] = readTexture("textures/greenMetal.png");

	texTrack[0] = readTexture("textures/track0.png");
	texTrack[1] = readTexture("textures/track1.png");
	texTrack[2] = readTexture("textures/track2.png");
	texTrack[3] = readTexture("textures/track3.png");
	texTrack[4] = readTexture("textures/track4.png");
	texTrack[5] = readTexture("textures/track5.png");
	texTrack[6] = readTexture("textures/track6.png");
	texTrack[7] = readTexture("textures/track7.png");
	texTrack[8] = readTexture("textures/track8.png");
	texTrack[9] = readTexture("textures/track9.png");

	texWorld[0] = readTexture("textures/groundSand.png");
	texWorld[1] = readTexture("textures/sky1.png");
	texWorld[2] = readTexture("textures/sky3.png");
	texWorld[3] = readTexture("textures/sky0.png");
	texWorld[4] = readTexture("textures/sky2.png");
	texWorld[5] = readTexture("textures/skyTop.png");

	//PlaySound(TEXT("sounds/Battlefield 1942 soundtrack.wav"), NULL, SND_ASYNC);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //kod, który należy wykonać po zakończeniu pętli głównej
	glDeleteTextures(5, texTank);
	glDeleteTextures(10, texTrack);
	glDeleteTextures(6, texWorld);
	deletePtrs();
	delete sp;
	delete skyBoxShader;
	delete explosionShader;
}

void drawHull() {
	sp->use();
	//Hull
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[0]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[0]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[0]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTank[0]);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texTank[1]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[0]); //Narysuj obiekt
}

void drawStaticPowertrain() {
	sp->use();
	//Static powertrain
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[1]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[1]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[1]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTank[2]);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texTank[3]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[1]); //Narysuj obiekt
}

void drawTracks(float wheel_angle, float tracks_speed) {
	sp->use();
	//Track Left
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[2]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[2]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[2]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);

	int texNumL = (int)(wheel_angle * tracks_speed) % 10; // Wyliczenie którą teksturą oteksturować gąsienicę aby stworzyć pozór ruchu 

	glBindTexture(GL_TEXTURE_2D, texTrack[abs(texNumL)]); // Wysłanie odpowiedniej tekstury
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[2]); //Narysuj obiekt

	//Track Right
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[3]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[3]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[3]);//an appropriate array

	int texNumR = (int)(wheel_angle * tracks_speed) % 10; // Wyliczenie którą teksturą oteksturować gąsienicę aby stworzyć pozór ruchu 
	glBindTexture(GL_TEXTURE_2D, texTrack[abs(texNumR)]); // Wysłanie odpowiedniej tekstury
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[3]); //Narysuj obiekt
}

void drawDynamicPowertrain(glm::mat4 M, float wheel_angle, float gear_angle) {
	sp->use();
	//Wheels Left
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[4]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[4]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[4]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTank[2]);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texTank[3]);

	float zcoords[7] = { 0.15, 0.85, 1.5, 2.13, -0.52, -1.27, -2 };

	glm::mat4 wheel;

	for (int i = 0; i < 7; i++)
	{
		//Lewa strona czołgu
		wheel = glm::translate(M, glm::vec3(0.0f, 0.34f, zcoords[i]));
		wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount[4]); //Narysuj obiekt
	}

	//Lewa strona czołgu
	wheel = glm::translate(M, glm::vec3(0.0f, 0.71f, 2.82f));
	wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[4]); //Narysuj obiekt

	//Wheels Right
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[5]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[5]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[5]);//an appropriate array

	for (int i = 0; i < 7; i++)
	{
		//Prawa strona czołgu
		wheel = glm::translate(M, glm::vec3(0.0f, 0.34f, zcoords[i]));
		wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount[5]); //Narysuj obiekt
	}

	//Prawa strona czołgu
	wheel = glm::translate(M, glm::vec3(0.0f, 0.71f, 2.82f));
	wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[5]); //Narysuj obiekt


	//Gears
	glm::mat4 gears = glm::translate(M, glm::vec3(0.0f, 0.6f, -2.68f));
	gears = glm::rotate(gears, gear_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(gears));

	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[6]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[6]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[6]);//an appropriate array

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[6]); //Narysuj obiekt
}

void drawTurret(glm::mat4 M, glm::mat4 V, float turret_angle_x, float turret_angle_y, glm::vec4 lightPosition1, glm::vec4 lightPosition2, glm::vec4 lightPosition3, glm::vec4 lightPosition4, glm::vec4 lightPosition5) {
	sp->use();
	//Turret
	glm::mat4 turret = glm::rotate(M, turret_angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu

	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(turret));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[7]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[7]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[7]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTank[0]);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texTank[1]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[7]); //Narysuj obiekt

	//Gun
	glm::mat4 gun = glm::translate(turret, glm::vec3(0.0f, 1.81f, 2.13f));
	gun = glm::rotate(gun, turret_angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu

	// Mnożymy pozycję światła przez -1 bo wtedy działa xD
	lightPosition1 *= -1;
	lightPosition2 *= -1;
	lightPosition3 *= -1;
	lightPosition4 *= -1;
	lightPosition5 *= -1;

	glUniform4fv(sp->u("lp1"), 1, glm::value_ptr(lightPosition1));
	glUniform4fv(sp->u("lp2"), 1, glm::value_ptr(lightPosition2));
	glUniform4fv(sp->u("lp3"), 1, glm::value_ptr(lightPosition3));
	glUniform4fv(sp->u("lp4"), 1, glm::value_ptr(lightPosition4));
	glUniform4fv(sp->u("lp5"), 1, glm::value_ptr(lightPosition5));

	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(gun));

	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[8]); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[8]); //Wskaż tablicę z danymi dla atrybutu normal
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[8]);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTank[0]);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texTank[1]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[8]); //Narysuj obiekt
}

void drawTankShell(glm::mat4 tankShell) {
	glm::vec3 position = glm::vec3(tankShell[3][0], tankShell[3][1], tankShell[3][2]);
	if (shot == true && explosion == false) {	
		skyBoxShader->use();
		glUniformMatrix4fv(skyBoxShader->u("M"), 1, false, glm::value_ptr(tankShell));
		glVertexAttribPointer(skyBoxShader->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[9]); //Wskaż tablicę z danymi dla atrybutu vertex
		glVertexAttribPointer(skyBoxShader->a("normal"), 4, GL_FLOAT, false, 0, models_normals[9]); //Wskaż tablicę z danymi dla atrybutu normal
		glVertexAttribPointer(skyBoxShader->a("texCoord0"), 2, GL_FLOAT, false, 0, models_texCoords[9]);//an appropriate array

		glUniform1i(skyBoxShader->u("textureMap0"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texTank[4]);

		glDrawArrays(GL_TRIANGLES, 0, vertexCount[9]); //Narysuj obiekt

		if (position.y < 0.5 || shotCooldown < 2) {
			explosion = true;
			shotCooldown += 5;
		}
			
	}
	else if (shot == true && explosion == true) {
		explosionShader->use();
		tankShell = glm::scale(tankShell, glm::vec3(2.f, 2.f, 2.f));
		glUniformMatrix4fv(explosionShader->u("M"), 1, false, glm::value_ptr(tankShell));
		glUniform1f(explosionShader->u("amount"), amount);

		glVertexAttribPointer(explosionShader->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[10]); //Wskaż tablicę z danymi dla atrybutu vertex

		glDrawArrays(GL_TRIANGLES, 0, vertexCount[10]); //Narysuj obiekt
	}
}

void drawGround(glm::vec3 Pos) {
	skyBoxShader->use();

	glEnableVertexAttribArray(skyBoxShader->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(skyBoxShader->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(skyBoxShader->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(skyBoxShader->a("normal"), 4, GL_FLOAT, false, 0, myCubeNormals); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(skyBoxShader->a("texCoord0"));
	glVertexAttribPointer(skyBoxShader->a("texCoord0"), 2, GL_FLOAT, false, 0, myCubeTexCoords);//an appropriate array

	glUniform1i(skyBoxShader->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texWorld[0]);

	// Oblicz środkowy chunk (chunk 32x32)
	float x = (int)((abs(Pos.x)+ chunkSize / 2) / chunkSize);
	float z = (int)((abs(Pos.z)+ chunkSize / 2) / chunkSize);

	if (Pos.x < 0)
		x *= -chunkSize;
	else
		x *= chunkSize;
	if (Pos.z < 0)
		z *= -chunkSize;
	else
		z *= chunkSize;
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, glm::vec3(x, 0.0f, z));

	// Rysowanie chunków
	for (int i = -chunkNum; i <= chunkNum; i++) {
		for (int j = -chunkNum; j <= chunkNum; j++) {
			glm::mat4 Mi = M;
			Mi = glm::translate(Mi, glm::vec3(i* chunkSize, -0.1f, j* chunkSize));
			Mi = glm::scale(Mi, glm::vec3(chunkSize / 2, 0.1f, chunkSize / 2));
			glUniformMatrix4fv(skyBoxShader->u("M"), 1, false, glm::value_ptr(Mi));
			glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
		}
	}
		
}

void drawSky(glm::vec3 Pos) {
	skyBoxShader->use();

	glm::mat4 M = glm::mat4(1.f);
	M = glm::translate(M, Pos);
	M = glm::scale(M, glm::vec3(128.f, 64.f, 128.f));
	M = glm::translate(M, glm::vec3(0.0f, 0.35f, 0.0f));

	glUniformMatrix4fv(skyBoxShader->u("M"), 1, false, glm::value_ptr(M));

	glVertexAttribPointer(skyBoxShader->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(skyBoxShader->a("texCoord0"), 2, GL_FLOAT, false, 0, myCubeTexCoords);//an appropriate array

	glUniform1i(skyBoxShader->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);

	for (int i = 0; i < 4; i++) {
		glBindTexture(GL_TEXTURE_2D, texWorld[i+1]);
		glDrawArrays(GL_TRIANGLES, 6*i, 6+(6*i)); //Narysuj obiekt
	}

	glBindTexture(GL_TEXTURE_2D, texWorld[5]);
	glDrawArrays(GL_TRIANGLES, 30, 36); //Narysuj obiekt	
}

glm::mat4 cameraChanger(glm::mat4 center, float turret_angle_x, float turret_angle_y) {
	glm::mat4 V;
	switch (camera)
	{
	case 1:	{
		// kamera za wieżyczką
		glm::mat4 M = glm::rotate(center, turret_angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu
		V = glm::translate(M, glm::vec3(0.0f, 5.0f, -10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(center[3][0], center[3][1], center[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
		}
	case 2: {
		// kamera prawa gąsienica
		V = glm::translate(center, glm::vec3(-2.5f, 1.5f, 2.1f));
		glm::mat4 M = glm::translate(center, glm::vec3(0.0f, 0.0f, 10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(M[3][0], M[3][1], M[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
		}
	case 3: {
		// kamera lewa gąsienica
		V = glm::translate(center, glm::vec3(2.5f, 1.5f, 2.1f));
		glm::mat4 M = glm::translate(center, glm::vec3(0.0f, 0.0f, 10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(M[3][0], M[3][1], M[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	}
	case 4: {
		// kamera przód
		V = glm::translate(center, glm::vec3(0.0f, 1.5f, 2.1f));
		glm::mat4 M = glm::translate(center, glm::vec3(0.0f, 0.0f, 10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(M[3][0], M[3][1], M[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	}
	case 5: {
		// kamera wieżyczka
		glm::mat4 M = glm::rotate(center, turret_angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu
		M = glm::rotate(M, turret_angle_y/2, glm::vec3(1.0f, 0.0f, 0.0f));
		V = glm::translate(M, glm::vec3(0.0f, 3.2f, 0.f));
		M = glm::translate(M, glm::vec3(0.0f, 0.0f, 10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(M[3][0], M[3][1], M[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	}
	case 6: {
		V = glm::translate(center, glm::vec3(0.0f, 5.0f, -10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(center[3][0], center[3][1], center[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	}
	default: {
		V = glm::translate(center, glm::vec3(0.0f, 5.0f, -10.0f));
		V = glm::lookAt(glm::vec3(V[3][0], V[3][1], V[3][2]), glm::vec3(center[3][0], center[3][1], center[3][2]), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
		}
	}
	return V;
}

//przyspieszanie i zwalnianie czołgu
void CalculateTankSpeed()
{
	if (w_pressed)
	{
		if (s_pressed)
		{
			if (tank_speed > 0)
			{
				tank_speed -= 2 * tank_acceleration_ratio;
				if (tank_speed < 0)
					tank_speed = 0;
			}
			else if (tank_speed < 0)
			{
				tank_speed += 2 * tank_acceleration_ratio;
				if (tank_speed > 0)
					tank_speed = 0;
			}
		}
		else if (tank_speed < 0)
		{
			tank_speed += 2 * tank_acceleration_ratio;
			if (tank_speed > max_tank_speed)
				tank_speed = max_tank_speed;
		}
		else if (tank_speed < max_tank_speed)
		{
			tank_speed += tank_acceleration_ratio;
			if (tank_speed > max_tank_speed)
				tank_speed = max_tank_speed;
		}
	}
	else
	{
		if (s_pressed)
		{
			if (tank_speed > 0)
			{
				tank_speed -= 2 * tank_acceleration_ratio;
				if (tank_speed < -max_tank_speed)
					tank_speed = -max_tank_speed;
			}
			else if (tank_speed > -max_tank_speed)
			{
				tank_speed -= tank_acceleration_ratio;
				if (tank_speed < -max_tank_speed)
					tank_speed = -max_tank_speed;
			}
		}
		else
		{
			if (tank_speed < 0)
			{
				tank_speed += tank_acceleration_ratio;
				if (tank_speed > 0)
					tank_speed = 0;
			}
			else if (tank_speed > 0)
			{
				tank_speed -= tank_acceleration_ratio;
				if (tank_speed < 0)
					tank_speed = 0;
			}
		}
	}
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, glm::mat4 center, glm::mat4 tankShell, float angle_x, float angle_y, float wheel_angle, float gear_angle, float turret_angle_x, float turret_angle_y, float timeStep) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::vec3 position = glm::vec3(center[3][0], center[3][1], center[3][2]);
	//std::cout << "x: " << position.x << "\ty: " << position.y << "\tz: " << position.z << std::endl;

	glm::vec4 lightPosition1 = glm::vec4(lightPos[0] + position, 1.f);
	glm::vec4 lightPosition2 = glm::vec4(lightPos[1] + position, 1.f);
	glm::vec4 lightPosition3 = glm::vec4(lightPos[2] + position, 1.f);
	glm::vec4 lightPosition4 = glm::vec4(lightPos[3] + position, 1.f);
	glm::vec4 lightPosition5 = glm::vec4(lightPos[4] + position, 1.f);

    glm::mat4 P = glm::perspective(60.0f*PI/180.0f, aspectRatio, 0.01f, 1024.0f); //Wylicz macierz rzutowania
	glm::mat4 V = cameraChanger(center, turret_angle_x, turret_angle_y);

	// Włączenie shaderów
	skyBoxShader->use();
	glEnableVertexAttribArray(skyBoxShader->a("vertex"));   //Włącz przesyłanie danych do atrybutu vertex
	glEnableVertexAttribArray(skyBoxShader->a("texCoord0")); //Włącz przesyłanie danych do atrybutu texCoord0
	glUniformMatrix4fv(skyBoxShader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(skyBoxShader->u("V"), 1, false, glm::value_ptr(V));

	// używają skyBoxShader
	glDepthMask(GL_FALSE);
	drawSky(position);
	glDepthMask(GL_TRUE);
	drawGround(position);

	explosionShader->use();//Aktywacja programu cieniującego
	glEnableVertexAttribArray(explosionShader->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glUniformMatrix4fv(explosionShader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(explosionShader->u("V"), 1, false, glm::value_ptr(V));

	// używają skyBoxShader lub explosionShader
	drawTankShell(tankShell);

	sp->use();//Aktywacja programu cieniującego
	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glEnableVertexAttribArray(sp->a("texCoord0")); //Włącz przesyłanie danych do atrybutu texCoord0
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(center));
	glUniform4fv(sp->u("lp1"), 1, glm::value_ptr(lightPosition1));
	glUniform4fv(sp->u("lp2"), 1, glm::value_ptr(lightPosition2));
	glUniform4fv(sp->u("lp3"), 1, glm::value_ptr(lightPosition3));
	glUniform4fv(sp->u("lp4"), 1, glm::value_ptr(lightPosition4));
	glUniform4fv(sp->u("lp5"), 1, glm::value_ptr(lightPosition5));

	// używają sp
	drawHull();

	// kolejność musi być następująca ponieważ drawTurret zmienia pozycję świateł
	drawTurret(center, V, turret_angle_x, turret_angle_y, lightPosition1, lightPosition2, lightPosition3, lightPosition4, lightPosition5);
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(center));
	drawTracks(wheel_angle, 6);
	drawStaticPowertrain();	
	drawDynamicPowertrain(center, wheel_angle, gear_angle);
	

	glDisableVertexAttribArray(skyBoxShader->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(skyBoxShader->a("texCoord0")); //Wyłącz przesyłanie danych do atrybutu texCoord0

	glDisableVertexAttribArray(explosionShader->a("vertex"));

	glDisableVertexAttribArray(sp->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(sp->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
	glDisableVertexAttribArray(sp->a("texCoord0")); //Wyłącz przesyłanie danych do atrybutu texCoord0

    glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL); 

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	glm::mat4 center = glm::mat4(1.0f);
	glm::mat4 tankShell = glm::mat4(1.0f);

	float turret_angle_x=0; //Aktualny kąt obrotu wieżyczki
	float turret_angle_y=0; //Aktualny kąt obrotu wieżyczki
	float wheel_angle = 1000; //Aktualny kąt obrotu kół
	float gear_angle = 1000; //Aktualny kąt obrotu zębatki

	float angle_x = 0; //Aktualny kąt obrotu wieżyczki
	float angle_y = 0; //Aktualny kąt obrotu wieżyczki

	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		float time_now = glfwGetTime();

		//przyspieszanie i zwalnianie czołgu
		CalculateTankSpeed();
		
		wheel_speed = tank_speed / wheel_radius;
		gear_speed = tank_speed / gear_radius;

		turret_angle_x += turret_speed_x * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		turret_angle_y += turret_speed_y * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		wheel_angle += wheel_speed * time_now;
		gear_angle += gear_speed * time_now;

		turret_angle_x += turret_speed_x * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        turret_angle_y+=turret_speed_y * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		gear_angle += gear_speed * time_now;

		center = glm::rotate(center, tank_turn_speed * time_now, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu
		center = glm::translate(center, glm::vec3(0.0f, 0.0f, tank_speed * time_now));
		
		//angle_x += speed_x * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		//angle_y += speed_y * time_now;
		if (turret_angle_y < -0.15)
			turret_angle_y = -0.15;
		else if (turret_angle_y > 0.145)
			turret_angle_y = 0.145;
		
		if (shot == true) {
			if (shotCooldown == cooldown) {
				shotCooldown -= time_now;
				tankShell = glm::rotate(center, turret_angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
				tankShell = glm::translate(tankShell, glm::vec3(0.0f, 1.81f, 2.13f));
				tankShell = glm::rotate(tankShell, turret_angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
				tankShell = glm::translate(tankShell, glm::vec3(0.0f, 0.0f, 5.f));
				explosion = false;
			}
			else if (shotCooldown <= 0 || amount > 4) {
				shot = false;
				explosion = false;
				amount = 1;
				shotCooldown = cooldown;
			}
			else if (explosion == false) {
				shotCooldown -= time_now;
				tankShell = glm::rotate(tankShell, 0.001f, glm::vec3(1.0f, 0.0f, 0.0f)); // Złudzenie grawitacji
				tankShell = glm::translate(tankShell, glm::vec3(0.0f, 0.f, tankShellSpeed)); // Przemieszczenie pocisku
			}
			else if (explosion == true) {
				amount += time_now;
			}
		}

        glfwSetTime(0); //Zeruj timer
		drawScene(window, center, tankShell, angle_x, angle_y, wheel_angle, gear_angle, turret_angle_x, turret_angle_y, time_now); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
