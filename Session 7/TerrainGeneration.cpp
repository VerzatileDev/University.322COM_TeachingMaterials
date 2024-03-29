#include <iostream>
#include <fstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#pragma comment(lib, "glew32.lib") 

using namespace std;
using namespace glm;

// Size of the terrain
const int MAP_SIZE = 33; //33

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

static const vec4 globAmb = vec4(0.2, 0.2, 0.2, 1.0);
static mat3 normalMat = mat3(1.0);

struct Vertex
{
	vec4 coords;
	vec3 normals;
};

struct Matrix4x4
{
	float entries[16];
};

struct Material
{
	vec4 ambRefl;
	vec4 difRefl;
	vec4 specRefl;
	vec4 emitCols;
	float shininess;
};

struct Light
{
	vec4 ambCols;
	vec4 difCols;
	vec4 specCols;
	vec4 coords;
};

static mat4 projMat = mat4(1.0);

static const Matrix4x4 IDENTITY_MATRIX4x4 =
{
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	}
};

// Front and back material properties.
static const Material terrainFandB =
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 1.0),
	50.0f
};

static const Light light0 =
{
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 0.0, 0.0)
};

static enum buffer { TERRAIN_VERTICES };
static enum object { TERRAIN };

// Globals
static Vertex terrainVertices[MAP_SIZE*MAP_SIZE] = {};

const int numStripsRequired = MAP_SIZE - 1;
const int verticesPerStrip = 2 * MAP_SIZE;

unsigned int terrainIndexData[numStripsRequired][verticesPerStrip];

static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
projMatLoc,
buffer[1],
vao[1];

// Function to read text file, used to read shader files
char* readTextFile(char* aTextFile)
{
	FILE* filePointer = fopen(aTextFile, "rb");
	char* content = NULL;
	long numVal = 0;

	fseek(filePointer, 0L, SEEK_END);
	numVal = ftell(filePointer);
	fseek(filePointer, 0L, SEEK_SET);
	content = (char*)malloc((numVal + 1) * sizeof(char));
	fread(content, 1, numVal, filePointer);
	content[numVal] = '\0';
	fclose(filePointer);
	return content;
}

void shaderCompileTest(GLuint shader)
{
	GLint result = GL_FALSE;
	int logLength;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> vertShaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(shader, logLength, NULL, &vertShaderError[0]);
	std::cout << &vertShaderError[0] << std::endl;
}

// Initialization routine.
void setup(void)
{
	float h1, h2, h3, h4,aver,h;
	///Generate random numbers ///
	srand(3); // get different srand values //seed
	h1 = (rand() % 10) / 5.0 - 1.0;
	h2 = (rand() % 10) / 5.0 - 1.0;
	h3 = (rand() % 10) / 5.0 - 1.0;
	h4 = (rand() % 10) / 5.0 - 1.0;

	// Initialise terrain - set values in the height map to 0
	float terrain[MAP_SIZE][MAP_SIZE] = {};
	for (int x = 0; x < MAP_SIZE; x++)
	{
		for (int z = 0; z < MAP_SIZE; z++)
		{
			terrain[x][z] = 0;
		}
	}

	// TODO: Add your code here to calculate the height values of the terrain using the Diamond square algorithm
	terrain[0][0] = h1*10.0;
	terrain[MAP_SIZE - 1][0] = h2*10.0;
	terrain[MAP_SIZE - 1][MAP_SIZE - 1] = h3*10.0;
	terrain[0][MAP_SIZE-1] = h4*10.0;
	int step_size,tt,H,count;
	float rand_max;
	tt  = MAP_SIZE;
	step_size = tt - 1;
	H = 1;
	rand_max = 1.0;

	while (step_size > 1)
	{
		for (int x = 0; x < MAP_SIZE - 1; x += step_size)
			for (int y = 0; y < MAP_SIZE - 1; y += step_size)
			{
				//diamond_step(x, y, stepsize)
				h1 = terrain[x][y];
				h2 = terrain[x+ step_size][y];
				h3 = terrain[x][y + step_size];
				h4 = terrain[x + step_size][y + step_size];
				aver = (h1 + h2 + h3 + h4) / 4.0;
				h = (rand() % 10) / 5.0 - 1.0;
				aver = aver + h*10.0*rand_max;
				terrain[x + step_size / 2][y + step_size / 2] = aver;
			}

		for (int x = 0; x < MAP_SIZE - 1; x += step_size)
			for (int y = 0; y < MAP_SIZE - 1; y += step_size)
			{
				//square_step(x, y)
				count = 0;
				h1 = terrain[x][y];  count++;
				h2 = terrain[x][y + step_size];  count++; //below
				if ((x - step_size / 2) >= 0) { h3 = terrain[x - step_size / 2][y + step_size / 2]; count++; }
				else { h3 = 0.0; }
				if ((x + step_size / 2) <MAP_SIZE) { h4 = terrain[x + step_size / 2][y + step_size / 2]; count++; }
				else { h4 = 0.0; }
				aver = (h1 + h2 + h3 + h4) /(float)count;
				h = (rand() % 10) / 5.0 - 1.0;
				aver = aver + h*10.0*rand_max;
				terrain[x][y + step_size / 2] = aver;

				//second one
				count = 0;
				h1 = terrain[x][y];  count++;
				h2 = terrain[x + step_size][y];  count++; //below
				if ((y - step_size / 2) >= 0) { h3 = terrain[x + step_size / 2][y - step_size / 2]; count++; }
				else { h3 = 0.0; }
				if ((y + step_size / 2) <MAP_SIZE) { h4 = terrain[x + step_size / 2][y + step_size / 2]; count++; }
				else { h4 = 0.0; }
				aver = (h1 + h2 + h3 + h4) / (float)count;
				h = (rand() % 10) / 5.0 - 1.0;
				aver = aver + h*10.0*rand_max;
				terrain[x + step_size / 2][y] = aver;

				//third one
				count = 0;
				h1 = terrain[x + step_size][y];  count++;
				h2 = terrain[x + step_size][y + step_size];  count++; //below
				h3 = terrain[x + step_size / 2][y + step_size / 2]; count++; 
				if ((x + 3*step_size / 2) <MAP_SIZE) { h4 = terrain[x + 3 * step_size / 2][y + step_size / 2]; count++; }
				else { h4 = 0.0; }
				aver = (h1 + h2 + h3 + h4) / (float)count;
				h = (rand() % 10) / 5.0 - 1.0;
				aver = aver + h*10.0*rand_max;
				terrain[x + step_size][y + step_size / 2] = aver;

				//fourth one
				count = 0;
				h1 = terrain[x][y + step_size];  count++;
				h2 = terrain[x + step_size][y + step_size];  count++; //below
				h3 = terrain[x + step_size / 2][y + step_size / 2]; count++;
				if ((y + 3 * step_size / 2) <MAP_SIZE) { h4 = terrain[x + step_size / 2][y + 3 * step_size / 2]; count++; }
				else { h4 = 0.0; }
				aver = (h1 + h2 + h3 + h4) / (float)count;
				h = (rand() % 10) / 5.0 - 1.0;
				aver = aver + h*10.0*rand_max;
				terrain[x + step_size / 2][y + step_size] = aver;
			}

		rand_max = rand_max * pow(2, -H);
		step_size = step_size / 2;
	}


	// Intialise vertex array
	int i = 0;

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int x = 0; x < MAP_SIZE; x++)
		{
			// Set the coords (1st 4 elements) and a default colour of black (2nd 4 elements) 
			terrainVertices[i].coords.x = (float)x;
			terrainVertices[i].coords.y = terrain[x][z];
			terrainVertices[i].coords.z = (float)z;
			terrainVertices[i].coords.w = 1.0;

			terrainVertices[i].normals.x = 0.0;
			terrainVertices[i].normals.y = 0.0;
			terrainVertices[i].normals.z = 0.0;

			i++;
		}
	}

	// Now build the index data 
	i = 0;
	for (int z = 0; z < MAP_SIZE - 1; z++)
	{
		i = z * MAP_SIZE;
		for (int x = 0; x < MAP_SIZE * 2; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
		for (int x = 1; x < MAP_SIZE * 2 + 1; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
	}

	///compute normal vectors for each vertices
	int index1, index2, index3;
	float dot_value;
	vec3 Pt1, Pt2, Pt3,ttVec,edgeVec1,edgeVec2,norVec,upvec;
	upvec.x = 0.0; upvec.y = 1.0; upvec.z = 0.0;
	for (int z = 0; z < MAP_SIZE - 1; z++)
	{
		for (int x = 0; x < (MAP_SIZE * 2-2); x ++)
		{
			index1 = terrainIndexData[z][x];
			index2 = terrainIndexData[z][x+1];
			index3 = terrainIndexData[z][x+2];

			Pt1.x = terrainVertices[index1].coords.x;
			Pt1.y = terrainVertices[index1].coords.y;
			Pt1.z = terrainVertices[index1].coords.z;

			Pt2.x = terrainVertices[index2].coords.x;
			Pt2.y = terrainVertices[index2].coords.y;
			Pt2.z = terrainVertices[index2].coords.z;

			Pt3.x = terrainVertices[index3].coords.x;
			Pt3.y = terrainVertices[index3].coords.y;
			Pt3.z = terrainVertices[index3].coords.z;

			edgeVec1 = Pt2 - Pt1;
			edgeVec2 = Pt3 - Pt1;
			if(x%2==1)
				ttVec = cross(edgeVec2, edgeVec1);
			else
				ttVec = cross(edgeVec1, edgeVec2);
			//norVec = normalize(ttVec);
			dot_value = dot(ttVec, upvec);
			if(dot_value<0.0000001) 
				norVec = -ttVec;
			else 
				norVec = ttVec;

			terrainVertices[index1].normals = norVec + terrainVertices[index1].normals;
			terrainVertices[index2].normals = norVec + terrainVertices[index2].normals;
			terrainVertices[index3].normals = norVec + terrainVertices[index3].normals;

			//terrainVertices[index1].normals = norVec;
			//terrainVertices[index2].normals = norVec;
			//terrainVertices[index3].normals = norVec;
		}
	}

	///smooth the normal vectors 
	//vector<vec3> ttVecArr;
	int total;
	total = MAP_SIZE*MAP_SIZE;
	for (i = 0; i < (total - 1); i++)
	{
		ttVec = terrainVertices[i].normals;
		norVec = normalize(ttVec);
		terrainVertices[i].normals = norVec;
	}

	glClearColor(1.0, 1.0, 1.0, 0.0);

	// Create shader program executable - read, compile and link shaders
	char* vertexShader = readTextFile("vertexShader.glsl");
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, (const char**)&vertexShader, NULL);
	glCompileShader(vertexShaderId);
	// Test for vertex shader compilation errors
	std::cout << "VERTEX::" << std::endl;
	shaderCompileTest(vertexShaderId);

	char* fragmentShader = readTextFile("fragmentShader.glsl");
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, (const char**)&fragmentShader, NULL);
	glCompileShader(fragmentShaderId);
	// Test for vertex shader compilation errors
	std::cout << "FRAGMENT::" << std::endl;
	shaderCompileTest(vertexShaderId);

	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);
	glUseProgram(programId);
	///////////////////////////////////////

	////Add for workshop 7
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.ambRefl"), 1,
		&terrainFandB.ambRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.difRefl"), 1,
		&terrainFandB.difRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.specRefl"), 1,
		&terrainFandB.specRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.emitCols"), 1,
		&terrainFandB.emitCols[0]);
	glUniform1f(glGetUniformLocation(programId, "terrainFandB.shininess"),
		terrainFandB.shininess);

	glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);

	glUniform4fv(glGetUniformLocation(programId, "light0.ambCols"), 1,
		&light0.ambCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.difCols"), 1,
		&light0.difCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.specCols"), 1,
		&light0.specCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.coords"), 1,
		&light0.coords[0]);


	// Create vertex array object (VAO) and vertex buffer object (VBO) and associate data with vertex shader.
	glGenVertexArrays(1, vao);
	glGenBuffers(1, buffer);
	glBindVertexArray(vao[TERRAIN]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[TERRAIN_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)sizeof(terrainVertices[0].normals));
	glEnableVertexAttribArray(1);
	///////////////////////////////////////

	// Obtain projection matrix uniform location and set value.
	projMatLoc = glGetUniformLocation(programId, "projMat");
	projMat = perspective(radians(60.0), (double) SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.1, 100.0);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	///////////////////////////////////////

	// Obtain modelview matrix uniform location and set value.
	mat4 modelViewMat = mat4(1.0);
	// Move terrain into view - glm::translate replaces glTranslatef
	//modelViewMat = translate(modelViewMat, vec3(-2.5f, -2.5f, -10.0f)); // 5x5 grid
	modelViewMat = translate(modelViewMat, vec3(-5.0f, -5.0f, -50.0f)); // 5x5 grid
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

	///////////////////////////////////////

	normalMat = transpose(inverse(mat3(modelViewMat)));
	glUniformMatrix3fv(glGetUniformLocation(programId, "normalMat"), 1, GL_FALSE, value_ptr(normalMat));

	//GLenum error = glGetError();
}

// Drawing routine.
void drawScene(void)
{
	//glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// For each row - draw the triangle strip
	for (int i = 0; i < MAP_SIZE - 1; i++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, verticesPerStrip, GL_UNSIGNED_INT, terrainIndexData[i]);
	}

	glFlush();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

// Main routine.
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	// Set the version of OpenGL (4.2)
	glutInitContextVersion(4, 2);
	// The core profile excludes all discarded features
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// Forward compatibility excludes features marked for deprecation ensuring compatability with future versions
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TerrainGeneration");

	////Add for workshop 7
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_SMOOTH);

	// Set OpenGL to render in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
