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
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

float aspectRatio=1;

ShaderProgram *sp;

GLuint tex0; //Uchwyt – deklaracja globalna
GLuint tex1;

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

static const int nr_of_models = 7;
float* models_vertices[nr_of_models];
float* models_normals[nr_of_models];
float* models_texCoords[nr_of_models];
float* models_colors[nr_of_models];
int vertexCount[nr_of_models];

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

float tank_speed = 0;
float max_tank_speed = 2;
float wheel_radius = 0.27;
float wheel_speed = 0;
float max_wheel_speed = max_tank_speed / wheel_radius; //omega = v/r
float gear_radius = 0.34;
float gear_speed = 0;
float max_gear_speed = max_tank_speed / gear_radius; //omega = v/r
float tank_turn_speed = 0;
float turret_speed_x = 0;
float turret_speed_y = 0;

float speed_x = 0;
float speed_y = 0;

void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
	/*if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
		if (key == GLFW_KEY_UP) speed_y = PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_y = 0;
		if (key == GLFW_KEY_DOWN) speed_y = 0;
	}*/
	if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_D) tank_turn_speed =-PI/3;
        if (key==GLFW_KEY_A) tank_turn_speed =PI/3;
		if (key == GLFW_KEY_W) { wheel_speed = max_wheel_speed; gear_speed = max_gear_speed; tank_speed = max_tank_speed; }
		if (key == GLFW_KEY_S) { wheel_speed = -max_wheel_speed; gear_speed = -max_gear_speed; tank_speed = -max_tank_speed; }
		if (key==GLFW_KEY_LEFT) turret_speed_x = PI / 3;
		if (key==GLFW_KEY_RIGHT) turret_speed_x = -PI / 3;
        if (key==GLFW_KEY_UP) turret_speed_y=-PI/4;
        if (key==GLFW_KEY_DOWN) turret_speed_y=PI/4;
		if (key == GLFW_KEY_SPACE) {
			//mciSendString(TEXT("close sounds/tank_shot.wav"), NULL, 0, 0);
			//mciSendString(TEXT("open sounds/tank_shot.wav"), NULL, 0, 0);
			mciSendString(TEXT("play sounds/tank_shot.wav"), NULL, 0, 0);//PlaySound(TEXT("sounds/tank shot.wav"), NULL, SND_ASYNC);
		}
    }
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_D) tank_turn_speed = 0;
		if (key == GLFW_KEY_A) tank_turn_speed = 0;
		if (key == GLFW_KEY_W) { wheel_speed = 0; gear_speed = 0; tank_speed = 0; }
		if (key == GLFW_KEY_S) { wheel_speed = 0; gear_speed = 0; tank_speed = 0; }
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
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	sp=new ShaderProgram("vs.glsl",NULL,"fs.glsl");

	loadOBJ("models/Hull.obj", 0);
	loadOBJ("models/Wheels_Both.obj",1);
	loadOBJ("models/Track.obj", 2);
	loadOBJ("models/Single_Wheels_Both.obj", 3);
	loadOBJ("models/Gears_Both.obj", 4);
	loadOBJ("models/Turret.obj", 5);
	loadOBJ("models/Gun.obj", 6);
	//tex0 = readTexture("metal.png");
	//tex1 = readTexture("sky.png");

	//PlaySound(TEXT("sounds/Battlefield 1942 soundtrack.wav"), NULL, SND_ASYNC);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	glDeleteTextures(1, &tex0);
	glDeleteTextures(1, &tex1);
	deletePtrs();
	delete sp;
	
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, glm::mat4 center, float angle_x, float angle_y, float wheel_angle, float gear_angle, float turret_angle_x, float turret_angle_y) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania
	//center = glm::rotate(center, angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	//center = glm::rotate(center, angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu

    sp->use();//Aktywacja programu cieniującego
    //Przeslij parametry programu cieniującego do karty graficznej
    glUniformMatrix4fv(sp->u("P"),1,false,glm::value_ptr(P));

    glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(center));

	//Hull
    glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
    glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, models_vertices[0]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[0]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[0]); //Wskaż tablicę z danymi dla atrybutu color

	/* Kod do tekstury
	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, &texCoords);//an appropriate array

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);

	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);*/

    glDrawArrays(GL_TRIANGLES,0,vertexCount[0]); //Narysuj obiekt

	//Static powertrain
	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[1]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[1]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[1]); //Wskaż tablicę z danymi dla atrybutu color

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[1]); //Narysuj obiekt

	//Track
	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[2]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[2]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[2]); //Wskaż tablicę z danymi dla atrybutu color

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[2]); //Narysuj obiekt

	//Wheels
	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[3]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[3]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[3]); //Wskaż tablicę z danymi dla atrybutu color

	float zcoords[7] = { 0.15, 0.85, 1.5, 2.13, -0.52, -1.27, -2 };

	glm::mat4 wheel;

	for (int i = 0; i < 7; i++)
	{
		wheel = glm::translate(center, glm::vec3(0.0f, 0.34f, zcoords[i]));
		wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount[3]); //Narysuj obiekt
	}

	wheel = glm::translate(center, glm::vec3(0.0f, 0.71f, 2.82f));
	wheel = glm::rotate(wheel, wheel_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(wheel));
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[3]); //Narysuj obiekt

	//Gears
	glm::mat4 gears = glm::translate(center, glm::vec3(0.0f, 0.6f, -2.68f));
	gears = glm::rotate(gears, gear_angle, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(gears));
	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[4]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[4]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[4]); //Wskaż tablicę z danymi dla atrybutu color

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[4]); //Narysuj obiekt

	//Turret
	glm::mat4 turret = glm::rotate(center, turret_angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu
	glm::mat4 cam = glm::translate(turret, glm::vec3(0.0f, 5.0f, -10.0f));
	glm::mat4 V = glm::lookAt(
		glm::vec3(0, 3, -8), //kamera statyczna
		glm::vec3(0, 0, 0),
		glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku
	/*glm::mat4 V = glm::lookAt(
		glm::vec3(cam[3][0],cam[3][1],cam[3][2]), //kamera za wieżyczką
		glm::vec3(center[3][0], center[3][1], center[3][2]),
		glm::vec3(0.0f, 1.0f, 0.0f));*/
	//turret = glm::rotate(turret, turret_angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(turret));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[5]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[5]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[5]); //Wskaż tablicę z danymi dla atrybutu color

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[5]); //Narysuj obiekt

	//Gun
	glm::mat4 gun = glm::translate(turret, glm::vec3(0.0f, 1.81f, 2.13f));
	gun = glm::rotate(gun, turret_angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu

	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(gun));

	glEnableVertexAttribArray(sp->a("vertex"));  //Włącz przesyłanie danych do atrybutu vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, models_vertices[6]); //Wskaż tablicę z danymi dla atrybutu vertex

	glEnableVertexAttribArray(sp->a("normal"));  //Włącz przesyłanie danych do atrybutu normal
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, models_normals[6]); //Wskaż tablicę z danymi dla atrybutu normal

	glEnableVertexAttribArray(sp->a("color"));  //Włącz przesyłanie danych do atrybutu color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, models_colors[6]); //Wskaż tablicę z danymi dla atrybutu color

	glDrawArrays(GL_TRIANGLES, 0, vertexCount[6]); //Narysuj obiekt

    glDisableVertexAttribArray(sp->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(sp->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal
	glDisableVertexAttribArray(sp->a("color"));  //Wyłącz przesyłanie danych do atrybutu normal
	//glDisableVertexAttribArray(sp->a("texCoord0"));

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

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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
	float turret_angle_x=0; //Aktualny kąt obrotu wieżyczki
	float turret_angle_y=0; //Aktualny kąt obrotu wieżyczki
	float wheel_angle=0; //Aktualny kąt obrotu kół
	float gear_angle = 0; //Aktualny kąt obrotu zębatki

	float angle_x = 0; //Aktualny kąt obrotu wieżyczki
	float angle_y = 0; //Aktualny kąt obrotu wieżyczki

	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		float time_now = glfwGetTime();
		turret_angle_x += turret_speed_x * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        turret_angle_y+=turret_speed_y * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		wheel_angle += wheel_speed * time_now;
		gear_angle += gear_speed * time_now;

		center = glm::rotate(center, tank_turn_speed * time_now, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu
		center = glm::translate(center, glm::vec3(0.0f, 0.0f, tank_speed * time_now));
		//angle_x += speed_x * time_now; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		//angle_y += speed_y * time_now;
		if (turret_angle_y < -0.2)
			turret_angle_y = -0.2;
		else if (turret_angle_y > 0.1)
			turret_angle_y = 0.1;
		
        glfwSetTime(0); //Zeruj timer
		drawScene(window, center, angle_x, angle_y, wheel_angle, gear_angle, turret_angle_x, turret_angle_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
