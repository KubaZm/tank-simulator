#ifndef MYCUBE_H_INCLUDED
#define MYCUBE_H_INCLUDED


//myCubeVertices - homogeniczne współrzędne wierzchołków w przestrzeni modelu
//myCubeNormals - homogeniczne wektory normalne ścian (per wierzchołek) w przestrzeni modelu
//myCubeVertexNormals - homogeniczne wektory normalne wierzchołków w przestrzeni modelu
//myCubeTexCoords - współrzędne teksturowania
//myCubeColors - kolory wierzchołków
//myCubeC1 - kolumna macierzy TBN^-1
//myCubeC2 - kolumna macierzy TBN^-1
//myCubeC3 - kolumna macierzy TBN^-1

int myCubeVertexCount=36;

float myCubeVertices[]={
                //Ściana 1
				1.0f,-1.0f,-1.0f,1.0f,
				-1.0f, 1.0f,-1.0f,1.0f,
				-1.0f,-1.0f,-1.0f,1.0f,

				1.0f,-1.0f,-1.0f,1.0f,
				1.0f, 1.0f,-1.0f,1.0f,
				-1.0f, 1.0f,-1.0f,1.0f,

                //Ściana 2
				-1.0f,-1.0f, 1.0f,1.0f,
				1.0f, 1.0f, 1.0f,1.0f,
				1.0f,-1.0f, 1.0f,1.0f,

				-1.0f,-1.0f, 1.0f,1.0f,
				-1.0f, 1.0f, 1.0f,1.0f,
				1.0f, 1.0f, 1.0f,1.0f,

				//Ściana 5
				-1.0f,-1.0f,-1.0f,1.0f,
				-1.0f, 1.0f, 1.0f,1.0f,
				-1.0f,-1.0f, 1.0f,1.0f,

				-1.0f,-1.0f,-1.0f,1.0f,
				-1.0f, 1.0f,-1.0f,1.0f,
				-1.0f, 1.0f, 1.0f,1.0f,

                //Ściana 6
				1.0f,-1.0f, 1.0f,1.0f,
				1.0f, 1.0f,-1.0f,1.0f,
				1.0f,-1.0f,-1.0f,1.0f,

				1.0f,-1.0f, 1.0f,1.0f,
				1.0f, 1.0f, 1.0f,1.0f,
				1.0f, 1.0f,-1.0f,1.0f,


				//Ściana 3 - dół
				-1.0f,-1.0f,-1.0f,1.0f,
				1.0f,-1.0f, 1.0f,1.0f,
				1.0f,-1.0f,-1.0f,1.0f,

				-1.0f,-1.0f,-1.0f,1.0f,
				-1.0f,-1.0f, 1.0f,1.0f,
				1.0f,-1.0f, 1.0f,1.0f,

				//Ściana 4 - góra
				-1.0f, 1.0f, 1.0f,1.0f,
				1.0f, 1.0f,-1.0f,1.0f,
				1.0f, 1.0f, 1.0f,1.0f,

				-1.0f, 1.0f, 1.0f,1.0f,
				-1.0f, 1.0f,-1.0f,1.0f,
				1.0f, 1.0f,-1.0f,1.0f,


			};

float myCubeColors[]={
                //Ściana 1
				1.0f,0.0f,0.0f,1.0f,
				1.0f,0.0f,0.0f,1.0f,
				1.0f,0.0f,0.0f,1.0f,

				1.0f,0.0f,0.0f,1.0f,
				1.0f,0.0f,0.0f,1.0f,
				1.0f,0.0f,0.0f,1.0f,

				//Ściana 2
				0.0f,1.0f,0.0f,1.0f,
				0.0f,1.0f,0.0f,1.0f,
				0.0f,1.0f,0.0f,1.0f,

				0.0f,1.0f,0.0f,1.0f,
				0.0f,1.0f,0.0f,1.0f,
				0.0f,1.0f,0.0f,1.0f,

				//Ściana 3
				0.0f,0.0f,1.0f,1.0f,
				0.0f,0.0f,1.0f,1.0f,
				0.0f,0.0f,1.0f,1.0f,

				0.0f,0.0f,1.0f,1.0f,
				0.0f,0.0f,1.0f,1.0f,
				0.0f,0.0f,1.0f,1.0f,

				//Ściana 4
				1.0f,1.0f,0.0f,1.0f,
				1.0f,1.0f,0.0f,1.0f,
				1.0f,1.0f,0.0f,1.0f,

				1.0f,1.0f,0.0f,1.0f,
				1.0f,1.0f,0.0f,1.0f,
				1.0f,1.0f,0.0f,1.0f,

				//Ściana 5
				0.0f,1.0f,1.0f,1.0f,
				0.0f,1.0f,1.0f,1.0f,
				0.0f,1.0f,1.0f,1.0f,

				0.0f,1.0f,1.0f,1.0f,
				0.0f,1.0f,1.0f,1.0f,
				0.0f,1.0f,1.0f,1.0f,

				//Ściana 6
				1.0f,1.0f,1.0f,1.0f,
				1.0f,1.0f,1.0f,1.0f,
				1.0f,1.0f,1.0f,1.0f,

				1.0f,1.0f,1.0f,1.0f,
				1.0f,1.0f,1.0f,1.0f,
				1.0f,1.0f,1.0f,1.0f,
			};

float myCubeNormals[]={
	//Ściana 1
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,

	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,

	//Ściana 2
	0.0f, 0.0f, 1.0f,0.0f,
	0.0f, 0.0f, 1.0f,0.0f,
	0.0f, 0.0f, 1.0f,0.0f,

	0.0f, 0.0f, 1.0f,0.0f,
	0.0f, 0.0f, 1.0f,0.0f,
	0.0f, 0.0f, 1.0f,0.0f,

	//Ściana 3
	0.0f,-1.0f, 0.0f,0.0f,
	0.0f,-1.0f, 0.0f,0.0f,
	0.0f,-1.0f, 0.0f,0.0f,

	0.0f,-1.0f, 0.0f,0.0f,
	0.0f,-1.0f, 0.0f,0.0f,
	0.0f,-1.0f, 0.0f,0.0f,

	//Ściana 4
	0.0f, 1.0f, 0.0f,0.0f,
	0.0f, 1.0f, 0.0f,0.0f,
	0.0f, 1.0f, 0.0f,0.0f,

	0.0f, 1.0f, 0.0f,0.0f,
	0.0f, 1.0f, 0.0f,0.0f,
	0.0f, 1.0f, 0.0f,0.0f,

	//Ściana 5
	-1.0f, 0.0f, 0.0f,0.0f,
	-1.0f, 0.0f, 0.0f,0.0f,
	-1.0f, 0.0f, 0.0f,0.0f,

	-1.0f, 0.0f, 0.0f,0.0f,
	-1.0f, 0.0f, 0.0f,0.0f,
	-1.0f, 0.0f, 0.0f,0.0f,

	//Ściana 6
	1.0f, 0.0f, 0.0f,0.0f,
	1.0f, 0.0f, 0.0f,0.0f,
	1.0f, 0.0f, 0.0f,0.0f,

	1.0f, 0.0f, 0.0f,0.0f,
	1.0f, 0.0f, 0.0f,0.0f,
	1.0f, 0.0f, 0.0f,0.0f,


};

float myCubeVertexNormals[]={
	//Ściana 1
	1.0f,-1.0f,-1.0f,0.0f,
	-1.0f, 1.0f,-1.0f,0.0f,
	-1.0f,-1.0f,-1.0f,0.0f,

	1.0f,-1.0f,-1.0f,0.0f,
	1.0f, 1.0f,-1.0f,0.0f,
	-1.0f, 1.0f,-1.0f,0.0f,

	//Ściana 2
	-1.0f,-1.0f, 1.0f,0.0f,
	1.0f, 1.0f, 1.0f,0.0f,
	1.0f,-1.0f, 1.0f,0.0f,

	-1.0f,-1.0f, 1.0f,0.0f,
	-1.0f, 1.0f, 1.0f,0.0f,
	1.0f, 1.0f, 1.0f,0.0f,

	//Ściana 5
	-1.0f,-1.0f,-1.0f,0.0f,
	-1.0f, 1.0f, 1.0f,0.0f,
	-1.0f,-1.0f, 1.0f,0.0f,

	-1.0f,-1.0f,-1.0f,0.0f,
	-1.0f, 1.0f,-1.0f,0.0f,
	-1.0f, 1.0f, 1.0f,0.0f,

	//Ściana 6
	1.0f,-1.0f, 1.0f,0.0f,
	1.0f, 1.0f,-1.0f,0.0f,
	1.0f,-1.0f,-1.0f,0.0f,

	1.0f,-1.0f, 1.0f,0.0f,
	1.0f, 1.0f, 1.0f,0.0f,
	1.0f, 1.0f,-1.0f,0.0f,


	//Ściana 3
	-1.0f,-1.0f,-1.0f,0.0f,
	1.0f,-1.0f, 1.0f,0.0f,
	1.0f,-1.0f,-1.0f,0.0f,

	-1.0f,-1.0f,-1.0f,0.0f,
	-1.0f,-1.0f, 1.0f,0.0f,
	1.0f,-1.0f, 1.0f,0.0f,

	//Ściana 4
	-1.0f, 1.0f, 1.0f,0.0f,
	1.0f, 1.0f,-1.0f,0.0f,
	1.0f, 1.0f, 1.0f,0.0f,

	-1.0f, 1.0f, 1.0f,0.0f,
	-1.0f, 1.0f,-1.0f,0.0f,
	1.0f, 1.0f,-1.0f,0.0f,
};

float myCubeTexCoords[]={
	//Ściana 1
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,

	//Ściana 2
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,

	//Ściana 5
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,

	//Ściana 6
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,


	//Ściana 3
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,

	//Ściana 4
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f,
};


#endif // MYCUBE_H_INCLUDED
