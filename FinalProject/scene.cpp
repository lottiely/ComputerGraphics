/*************************************************************
CG: Shaded Scene
Shanan Almario
CSCE385

This code uses the assignment 4's code provided as a base reference.

Camera: Free flying
x/X turns scene on the x axis
z/Z turns scene on the z axis
y/Y turns scene on the y axis
Directional Up moves forward.
Directional Down moves backwards.
Directional Right moves right.
Directional Left moves left.

Keyboard:
v/V: toggles eye view
ESC: closes all windows


****************************************************************/

#include "Angel.h"
#include "glm.h"
#include<iostream>
#include <sstream> 
#include <assert.h>
#include <vector>
#include <fstream>
#include <string>
#include <time.h>

///////CONSTANT/////////////////////
#ifndef PI
#define PI 3.14159265358979323846
#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     4.0
#endif

typedef vec4 point4;
typedef vec4 color4;

void m_glewInitAndVersion(void);

void specKey(int key, int x, int y);
void animate(int);
void display(void);
void init(void);
void keyboard(unsigned char, int, int);
void reshape(int width, int height);
void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
void idle(void);
void buildRoof1();

int currentHeight = 700, currentWidth = 800;

//sphere constants
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices = 3 * NumTriangles;

GLuint buffers[12];
int  TextureWidth[12];
int  TextureHeight[12];
GLuint textures[12];
vec2 cylinder_tex_coords[600];
vec2 sphere_tex_coords[NumVertices];

GLubyte* groundImage;
GLubyte* house1Image;
GLubyte* roof1Image;
GLubyte* trunkImage;
GLubyte* treeImage;
GLubyte* roadImage;
GLubyte* house2Image;
GLubyte* roof2Image;
GLubyte* carImage;
GLubyte* mountainImage;
GLubyte* wheelImage;
GLubyte* waterImage;

//instancing
const int treeGrid = 80;
const int NumInstances = treeGrid*treeGrid;
vec4 shiftATTR[NumInstances];
GLuint vShift;

//create a vertex array object
GLuint vao[12];
GLuint program[3];
//Spinning light: orbit parameterization
float spin = 0.0, spin_step = 0.001;

//--------------------------------------
//   Program and Vao Objects 
//--------------------------------------

enum {
	//objects
	house1 = 0,
	house2 = 1,
	ground = 2,
	tree = 3,
	trunk = 4,
	mountain = 5,
	road = 6,
	sky = 7,
	car = 8,
	windshield = 9,
	wheels = 10,
	nodesnum
};

enum {
	groundTextureImage = 0,
	house1TextureImage = 1,
	roof1TextureImage = 2,
	treeTextureImage = 3,
	trunkTextureImage = 4,
	roadTextureImage = 5,
	house2TextureImage = 6,
	roof2TextureImage = 7,
	carTextureImage = 8,
	mountainTextureImage = 9,
	wheelTextureImage = 10,
	waterTextureImage = 11
};

///////////////MATRIX STACK////////////

class stackClass {
public:
	static const int CAPACITY = 100;
	stackClass();
	~stackClass();
	mat4 pop();
	void push(const mat4&);
private:
	int index;
	mat4 matrix[CAPACITY];
} stack, normalStack;

stackClass::stackClass() {
	index = 0;
}
stackClass::~stackClass() {
	//delete[] matrix;
}
void stackClass::push(const mat4& s) {
	matrix[index] = s;
	index++;
}

mat4 stackClass::pop() {
	index--;
	return matrix[index];
}

//---------------------------------------------------------
//	Matrix Stack 
//			Probably redundant of the above, but 
//			this one is used for the tree nodes
//---------------------------------------------------------
class MatrixStack {
	int    _index;
	int    _size;
	mat4*  _matrices;

public:
	MatrixStack(int numMatrices = 32) :_index(0), _size(numMatrices)
	{
		_matrices = new mat4[numMatrices];
	}

	~MatrixStack()
	{
		delete[]_matrices;
	}

	void push(const mat4& m) {
		assert(_index + 1 < _size);
		_matrices[_index++] = m;
	}

	mat4& pop(void) {
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};
MatrixStack  mvstack;

using namespace std;

//--------------------------------------------------------
// Sky box stuff
//--------------------------------------------------------
GLuint texture;
typedef struct
{

	int topWidth;
	int topHeight;
	GLubyte *top;

	int bottomWidth;
	int bottomHeight;
	GLubyte *bottom;

	int leftHeight;
	int leftWidth;
	GLubyte *left;


	int rightWidth;
	int rightHeight;
	GLubyte *right;

	int frontWidth;
	int frontHeight;
	GLubyte *front;

	int backWidth;
	int backHeight;
	GLubyte *back;

} CubeMap;

CubeMap skybox;

point4 cube_vertices_positions[24] = {
	//x_positive
	point4(1.0f, -1.0f, -1.0f, 1.0f),
	point4(1.0f, -1.0f,  1.0f, 1.0f),
	point4(1.0f,  1.0f,  1.0f, 1.0f),
	point4(1.0f,  1.0f, -1.0f, 1.0f),
	//x-negative
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	//y-positive
	point4(-1.0f,  1.0f, -1.0f, 1.0f),
	point4(1.0f,  1.0f, -1.0f, 1.0f),
	point4(1.0f,  1.0f,  1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	//y-negatve
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4(1.0f, -1.0f,  1.0f, 1.0f),
	point4(1.0f, -1.0f, -1.0f, 1.0f),
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	//z-positive
	point4(1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	point4(1.0f,  1.0f,  1.0f, 1.0f),
	//z-negative
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	point4(1.0f, -1.0f, -1.0f, 1.0f),
	point4(1.0f,  1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f, -1.0f, 1.0f)
};

/////ATTRIBUTES, BUFFERS, ARRAYS///////////////////
GLuint vPositionCylinder, vNormalCylinder, vPositionSphere, vNormalSphere, vPositionEllipsoid, vNormalEllipsoid, vTexCoord;

// Model-view and projection matrices uniform location
GLuint  ModelView, NormalView, Projection, camera_loc;

mat4 projection = mat4(1.0);
mat4 model_view = mat4(1.0);
mat4 normal_view = mat4(1.0);

int pitchUp = 0;
float roll = 0, yaw = 0, pitch = 0;
float spin2 = 0.0, spin_step2 = 0.1;
float scaleX = 0, scaleY = 1, scaleZ = 0;
GLfloat s = 0.5; //strafe step
float angle = 0.01f;
float cosine = cos(angle);
float sine = sin(angle);
int speedForward = 0;
int speedRight = 0;

GLfloat d = 0.5;

/////Camera unv basis///////
GLfloat theta = 0;
GLfloat thetaY = 0;

Angel::vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 n = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 eye = vec4(63, 4.0, 80, 1.0);
vec4 nref = n, vref = v, uref = u;


Angel::vec4 car_v = vec4(0.0, 1.0, 0.0, 0.0);
Angel::vec4 car_u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 car_n = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 car_eye = vec4(63, 2.0, 80, 1.0);

///////LIGHTING and SHADING////////////////////////

int Index = 0;

//--------------------------------
// Build the ground
//
//	This is just a giant sqare whose with a large x and z
//	and a small y
//	This will appear to be the land where the town is on
//
int groundPoints = 6;
point4 groundData[6];
vec3 groundNormals[6];
vec2 groundTextures[6];

void
triangleGround(const point4& a, const point4& b, const point4& c)
{
	groundNormals[Index] = normalize(vec3(a[0], a[1], a[2])); groundData[Index] = a; Index++;
	groundNormals[Index] = normalize(vec3(b[0], b[1], b[2])); groundData[Index] = b; Index++;
	groundNormals[Index] = normalize(vec3(c[0], c[1], c[2])); groundData[Index] = c; Index++;
}

void buildGround() {
	Index = 0;
	int vertexCounter = 0;
	point4 v[16] = {
		//wall1
		vec4(-77, 0, -77, 1.0),			// 0
		vec4(-77, 0, 77, 1.0),			// 1
		vec4(77, 0, 77, 1.0),			// 2
		vec4(77, 0, -77, 1.0),			// 3
	};

	triangleGround(v[0], v[1], v[2]);
	groundTextures[vertexCounter] = vec2(0,0);
	vertexCounter++;
	groundTextures[vertexCounter] = vec2(0, 40);
	vertexCounter++; 
	groundTextures[vertexCounter] = vec2(40, 40);
	vertexCounter++;

	triangleGround(v[0], v[2], v[3]);
	groundTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	groundTextures[vertexCounter] = vec2(40, 40);
	vertexCounter++;
	groundTextures[vertexCounter] = vec2(40, 0);
	vertexCounter++;

}

//--------------------------------
// Builds House Type 1
//
//	This house is the one with a flat roof
//	colored with a lighter green and brown
//
point4 house1Data[66];
vec3 house1Normals[66];
vec2 house1BuildingTexture[24];

void
triangleHouse1(const point4& a, const point4& b, const point4& c)
{
	house1Normals[Index] = normalize(vec3(a[0], a[1], a[2])); house1Data[Index] = a; Index++;
	house1Normals[Index] = normalize(vec3(b[0], b[1], b[2])); house1Data[Index] = b; Index++;
	house1Normals[Index] = normalize(vec3(c[0], c[1], c[2])); house1Data[Index] = c; Index++;
}

void buildHouse1() {
	Index = 0;
	int vertexCounter = 0;
	point4 v[16] = {
		//wall1
		vec4(0, 1.0, 0.0, 1.0),			// 0
		vec4(0, 0, 0, 1.0),				// 1
		vec4(2.0, 1.0, 0, 1.0),			// 2
		vec4(2.0, 0, 0, 1.0),			// 3

		//wall2
		vec4(0, 1.0, 2.0, 1.0),			// 4
		vec4(0, 0, 2.0, 1.0),			// 5
		vec4(2.0, 1.0, 2.0, 1.0),		// 6
		vec4(2.0, 0, 2.0, 1.0),			// 7

		//Wall 3
		vec4(0, 1.0, 0.0, 1.0),			// 8	//8 9 10, 10 8 11
		vec4(0, 0, 0.0, 1.0),			// 9
		vec4(0, 0, 2.0, 1.0),			// 10
		vec4(0, 1.0, 2.0, 1.0),			// 11

		//Wall 4
		vec4(2.0, 1.0, 0.0, 1.0),		// 12
		vec4(2.0, 0.0, 0.0, 1.0),		// 13
		vec4(2.0, 0.0, 2.0, 1.0),		// 14
		vec4(2.0, 1.0, 2.0, 1.0)		// 15
	};

	//Wall 1
	triangleHouse1(v[0], v[1], v[2]);
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleHouse1(v[2], v[1], v[3]);
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	
	//Wall 2
	triangleHouse1(v[4], v[5], v[6]);
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleHouse1(v[6], v[5], v[7]);
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;

	//Wall 30
	triangleHouse1(v[8], v[9], v[10]);
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	triangleHouse1(v[10], v[8], v[11]);
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	//Wall 4
	triangleHouse1(v[12], v[13], v[14]);
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	triangleHouse1(v[14], v[12], v[15]);
	house1BuildingTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	house1BuildingTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	buildRoof1();
}

//--------------------------------
// Builds House Type 1
//
//	This house is the one with a flat roof
//	colored with a lighter green and brown
//
point4 roof1Data[42];
vec3 roof1Normals[42];
vec2 roof1Texture[42];

void
triangleRoof1(const point4& a, const point4& b, const point4& c)
{
	roof1Normals[Index] = normalize(vec3(a[0], a[1], a[2])); roof1Data[Index] = a; Index++;
	roof1Normals[Index] = normalize(vec3(b[0], b[1], b[2])); roof1Data[Index] = b; Index++;
	roof1Normals[Index] = normalize(vec3(c[0], c[1], c[2])); roof1Data[Index] = c; Index++;
}

void buildRoof1() {
	//Index = 0;
	int vertexCounter = 0;
	point4 v[12] = {
		//trapezoid front
		vec4(0, 1.0, 0, 1.0),				// 0
		vec4(0.5, 1.0, 0.0, 1.0),			// 1
		vec4(0.5, 1.5, 0, 1.0),				// 2

		vec4(2.0, 1.0, 0, 1.0),				// 3
		vec4(1.5, 1.0, 0.0, 1.0),			// 4
		vec4(1.5, 1.5, 0, 1.0),				// 5

		vec4(0, 1.0, 2.0, 1.0),				// 6
		vec4(0.5, 1.0, 2.0, 1.0),			// 7
		vec4(0.5, 1.5, 2.0, 1.0),			// 8 

		vec4(2.0, 1.0, 2.0, 1.0),			// 9
		vec4(1.5, 1.0, 2.0, 1.0),			// 10
		vec4(1.5, 1.5, 2.0, 1.0)			// 11
	};

	//trapezoid roof front
	// 0, 1, 2
	// 3, 4, 5
	// 5, 2, 1
	// 5, 4, 1
	triangleHouse1(v[0], v[1], v[2]);
	roof1Texture[vertexCounter] = vec2(v[0].x * 2, v[0].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[1].x * 2, v[1].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[2].x * 2, v[2].y * 2);
	vertexCounter++;

	triangleHouse1(v[3], v[4], v[5]);
	roof1Texture[vertexCounter] = vec2(v[3].x * 2, v[3].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[4].x * 2, v[4].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[5].x * 2, v[5].y * 2);
	vertexCounter++;

	triangleHouse1(v[5], v[2], v[1]);
	roof1Texture[vertexCounter] = vec2(v[5].x * 2, v[5].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[2].x * 2, v[2].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[1].x * 2, v[1].y * 2);
	vertexCounter++;

	triangleHouse1(v[5], v[4], v[1]);
	roof1Texture[vertexCounter] = vec2(v[5].x * 2, v[5].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[4].x * 2, v[4].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[1].x * 2, v[1].y * 2);
	vertexCounter++;

	//trapezoid  back
	// 6, 7, 8
	// 9, 10, 11
	// 11, 8, 7
	// 11, 10, 7
	triangleHouse1(v[6], v[7], v[8]);
	roof1Texture[vertexCounter] = vec2(v[6].x * 2, v[6].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[7].x * 2, v[7].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[8].x * 2, v[8].y * 2);
	vertexCounter++;

	triangleHouse1(v[9], v[10], v[11]);
	roof1Texture[vertexCounter] = vec2(v[9].x * 2, v[9].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[10].x * 2, v[10].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[11].x * 2, v[11].y * 2);
	vertexCounter++;

	triangleHouse1(v[11], v[8], v[7]);
	roof1Texture[vertexCounter] = vec2(v[11].x * 2, v[11].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[8].x * 2, v[8].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[7].x * 2, v[7].y * 2);
	vertexCounter++;

	triangleHouse1(v[11], v[10], v[7]);
	roof1Texture[vertexCounter] = vec2(v[11].x * 2, v[11].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[10].x * 2, v[10].y * 2);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(v[7].x * 2, v[7].y * 2);
	vertexCounter++;

	// rectangle on one side
	// 2, 0, 8
	// 8, 0, 6
	triangleHouse1(v[2], v[0], v[8]);
	roof1Texture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;

	triangleHouse1(v[8], v[0], v[6]);
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 0);
	vertexCounter++;

	// rectangle on other side
	// 5, 3, 11
	// 11, 3, 9
	triangleHouse1(v[5], v[3], v[11]);
	roof1Texture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;

	triangleHouse1(v[11], v[3], v[9]);
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 0);
	vertexCounter++;

	//roof rectangle top
	// 2, 11, 8
	// 2, 5, 11
	triangleHouse1(v[2], v[11], v[8]);
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(0, 1);
	vertexCounter++;

	triangleHouse1(v[2], v[5], v[11]);
	roof1Texture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 0);
	vertexCounter++;
	roof1Texture[vertexCounter] = vec2(3, 1);
	vertexCounter++;
}

//--------------------------------
// Builds Tree
//
//	The tree consists of one cylinder (used from the cannon assignment)
//	and three pyramids stacked on each other.
//	Pyramids are three triangles joined by sides, created exactly like the mountain.
//	Pyramids are colored exactly as the mountain with bottom two vertices being a darker 
//	color and the top vertex being the lighter color
//

//---------------------------------------------------------
//	Building the Cylinder
//---------------------------------------------------------

vec4 cylinderData[600];
vec3 cylinderNormal[600];
int iMax = 50, vertexCounter = 0;
GLfloat x, z, thetaC;

void buildTrunk() {
	//cylinder
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 4.0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 4.0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 4.0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 4.0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 4.0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 4.0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0);
		cylinderNormal[vertexCounter] = vec3(x, 0, z);
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;
	}
}


point4 treeData[27];
vec3 treeNormals[27];
vec2 treeTextures[27];

void
triangleTree(const point4& a, const point4& b, const point4& c)
{
	treeNormals[Index] = normalize(vec3(a[0], a[1], a[2])); treeData[Index] = a; Index++;
	treeNormals[Index] = normalize(vec3(b[0], b[1], b[2])); treeData[Index] = b; Index++;
	treeNormals[Index] = normalize(vec3(c[0], c[1], c[2])); treeData[Index] = c; Index++;
}

void buildTree() {
	Index = 0;
	int vertexCounter = 0;

	point4 v[15] = {
		//bottomPyramid
		vec4(-4.0, 2.5, 2, 1.0),			// 0
		vec4(4.0, 2.5, 2, 1.0),				// 1
		vec4(0.0, 6.0, 0.0, 1.0),			// 2
		vec4(0.0, 2.5, -4, 1.0),			// 3
		vec4(0.0, 6.0, 0, 1.0),				// 4

		//middlePyramid
		vec4(-4.0, 4.0, 2, 1.0),			// 5
		vec4(4.0, 4.0, 2, 1.0),				// 6
		vec4(0.0, 7.5, 0.0, 1.0),			// 7
		vec4(0.0, 4.0, -4, 1.0),			// 8
		vec4(0.0, 7.5, 0, 1.0),				// 9

		//topPyramid
		vec4(-4.0, 5.5, 2, 1.0),			// 10
		vec4(4.0, 5.5, 2, 1.0),				// 11
		vec4(0.0, 9.0, 0.0, 1.0),			// 12
		vec4(0.0, 5.5, -4, 1.0),			// 13
		vec4(0.0, 9.0, 0, 1.0),				// 14

	};
	//bottomPyramid
	// 0, 1, 2
	// 0, 3, 4
	// 1, 3, 2
	//side 1
	triangleTree(v[0], v[1], v[2]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 2
	triangleTree(v[0], v[3], v[4]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 3
	triangleTree(v[1], v[3], v[2]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//middle pyramid
	// 5, 6, 7
	// 5, 8, 9
	// 6, 8, 7
	//side 1
	triangleTree(v[5], v[6], v[7]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 2
	triangleTree(v[5], v[8], v[9]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 3
	triangleTree(v[6], v[8], v[7]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//top pyramid
	// 10, 11, 12
	// 10, 13, 14
	// 11, 13, 12
	//side 1
	triangleTree(v[10], v[11], v[12]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 2
	triangleTree(v[10], v[13], v[14]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	//side 3
	triangleTree(v[11], v[13], v[12]);
	treeTextures[vertexCounter] = vec2(-1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(1 * 1, 0);
	vertexCounter++;
	treeTextures[vertexCounter] = vec2(0.5 * 1, 1 * 1);
	vertexCounter++;

	buildTrunk();
}

//--------------------------------
// Builds Road
//
//	The roads and stretch of water are all rectangles
//	Roads are constructed as rectangles of the same color 
//	placed next to each other
//

point4 roadData[30];
vec3 roadNormals[30];
vec2 roadTextures[24];
vec2 waterTextures[6];

void
triangleRoad(const point4& a, const point4& b, const point4& c)
{
	roadNormals[Index] = normalize(vec3(a[0], a[1], a[2])); roadData[Index] = a; Index++;
	roadNormals[Index] = normalize(vec3(b[0], b[1], b[2])); roadData[Index] = b; Index++;
	roadNormals[Index] = normalize(vec3(c[0], c[1], c[2])); roadData[Index] = c; Index++;
}

void buildRoad() {
	Index = 0;
	int vertexCounter = 0;

	point4 v[20] = {
		vec4(-10.0, 0.01, -10.0, 1.0),		// 0
		vec4(-10.0, 0.01, -5.0, 1.0),		// 1
		vec4(70, 0.01, -10.0, 1.0),			// 2
		vec4(70, 0.01, -5.0, 1.0),			// 3

		vec4(15, 0.01, -5, 1.0),			// 4
		vec4(15, 0.01, 30.0, 1.0),			// 5
		vec4(20, 0.01, -5, 1.0),			// 6
		vec4(20, 0.01, 30.0, 1.0),			// 7

		vec4(20, 0.01, 25, 1.0),			// 8
		vec4(20, 0.01, 30, 1.0),			// 9
		vec4(65, 0.01, 25, 1.0),			// 10
		vec4(65, 0.01, 30, 1.0),			// 11

		vec4(60, 0.01, -5, 1.0),			// 12
		vec4(60, 0.01, 25, 1.0),			// 13
		vec4(65, 0.01, -5, 1.0),			// 14
		vec4(65, 0.01, 25, 1.0),			// 15

		vec4(70,0.01,-10,1.0),				// 16
		vec4(70, 0.01, -5.0, 1.0),			// 17
		vec4(100, 0.01, -10, 1.0),			// 18
		vec4(100, 0.01, -5, 1.0)				// 19

		// 16, 17, 18
		// 18, 17, 19
	};

	//roads
	//each block are patches of rectangles placed next to each other
	//rectangle1
	// 0, 1, 2
	// 2, 1, 3
	triangleRoad(v[0], v[1], v[2]);
	roadTextures[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1* 10, 1);
	vertexCounter++;

	triangleRoad(v[2], v[1], v[3]);
	roadTextures[vertexCounter] = vec2(1 * 10, 1 );
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1 * 10, 0);
	vertexCounter++;

	//rectangle 2
	// 4, 5, 6
	// 6, 5, 7
	triangleRoad(v[4], v[5], v[6]);
	roadTextures[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleRoad(v[6], v[5], v[7]);
	roadTextures[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1, 0);
	vertexCounter++;

	//rectangle3
	// 8, 9, 10
	// 10, 9, 11
	triangleRoad(v[8], v[9], v[10]);
	roadTextures[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1 * 10, 1);
	vertexCounter++;

	triangleRoad(v[10], v[9], v[11]);
	roadTextures[vertexCounter] = vec2(1 * 10, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1 * 10, 0);
	vertexCounter++;

	//rectangle 4
	// 12, 13, 14
	// 14, 13, 15
	triangleRoad(v[12], v[13], v[14]);
	roadTextures[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleRoad(v[14], v[13], v[15]);
	roadTextures[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	roadTextures[vertexCounter] = vec2(1, 0);
	vertexCounter++;

	//rectangle 5 water
	// 16, 17, 18
	// 18, 17, 19
	triangleRoad(v[16], v[17], v[18]);
	waterTextures[vertexCounter] = vec2(0, 1*5);
	vertexCounter++;
	waterTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	waterTextures[vertexCounter] = vec2(1*5, 1*5);
	vertexCounter++;

	triangleRoad(v[18], v[17], v[19]);
	waterTextures[vertexCounter] = vec2(1*5, 1*5);
	vertexCounter++;
	waterTextures[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	waterTextures[vertexCounter] = vec2(1*5, 0);
	vertexCounter++;
}

//--------------------------------
// Builds House Type 1
//
//	This house is the one with a flat roof
//	colored with a lighter green and brown
//
point4 carData[30];
vec3 carNormals[30];
vec2 carTexture[30];

point4 windShieldData[6];
vec3 windshieldNormals[6];
vec4 carColor[6];

void
triangleCar(const point4& a, const point4& b, const point4& c)
{
	carNormals[Index] = normalize(vec3(a[0], a[1], a[2])); carData[Index] = a; Index++;
	carNormals[Index] = normalize(vec3(b[0], b[1], b[2])); carData[Index] = b; Index++;
	carNormals[Index] = normalize(vec3(c[0], c[1], c[2])); carData[Index] = c; Index++;
}

void
triangleWindshield(const point4& a, const point4& b, const point4& c)
{
	windshieldNormals[Index] = normalize(vec3(a[0], a[1], a[2])); windShieldData[Index] = a; Index++;
	windshieldNormals[Index] = normalize(vec3(b[0], b[1], b[2])); windShieldData[Index] = b; Index++;
	windshieldNormals[Index] = normalize(vec3(c[0], c[1], c[2])); windShieldData[Index] = c; Index++;
}

vec4 wheelData[600];
vec3 wheelNormal[600];
vec2 wheelTexture[600];

void buildWheels() {
	int iMax = 50, vertexCounter = 0;
	GLfloat x, z, thetaC;

	//top
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(0, 0.5, 0, 1.0);
		wheelNormal[vertexCounter] = vec3(0, 0.5, 0);
		wheelTexture[vertexCounter] = vec2(0, 0);
		vertexCounter++;

		wheelData[vertexCounter] = vec4(x, 0.5, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0.5, z);
		wheelTexture[vertexCounter] = vec2(x, z);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0.5, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0.5, z);
		wheelTexture[vertexCounter] = vec2(x, z);
		vertexCounter++;
	}

	//bottom
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(0, 0, 0, 1.0);
		wheelNormal[vertexCounter] = vec3(0, 0, 0);
		wheelTexture[vertexCounter] = vec2(0, 0);
		vertexCounter++;

		wheelData[vertexCounter] = vec4(x, 0, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0, z);
		wheelTexture[vertexCounter] = vec2(x, z);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0, z);
		wheelTexture[vertexCounter] = vec2(x, z);
		vertexCounter++;
	}

	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0.5, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0.5, z);
		wheelTexture[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		wheelData[vertexCounter] = vec4(x, 0, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0, z);
		wheelTexture[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0.5, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0.5, z);
		wheelTexture[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		wheelData[vertexCounter] = vec4(x, 0.5, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0.5, z);
		wheelTexture[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0, z);
		wheelTexture[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0);
		wheelNormal[vertexCounter] = vec3(x, 0, z);
		wheelTexture[vertexCounter] = vec2(x, 0);
		vertexCounter++;
	}
}


void buildWindshield() {
	Index = 0;
	vec4 windshieldColor = vec4(0.69, 0.88, 0.91, 1.0);

	point4 v[6] = {
		//windshield
		vec4(0, 1.0, 0, 1.0),			// 0
		vec4(0, 1.5, 0, 1.0),			// 1
		vec4(0, 1.5, 1.0, 1.0),			// 2
		vec4(0, 1.0, 1.0, 1.0),			// 3
	};

	//windshield
	triangleWindshield(v[0], v[1], v[2]);
	triangleWindshield(v[2], v[3], v[0]);

	for (int i = 0; i < 6; i++)
		carColor[i] = windshieldColor;
}

void buildCar() {
	Index = 0;
	int vertexCounter = 0;

	point4 v[24] = {
		//wall1
		vec4(0, 1.0, 0.0, 1.0),			// 0
		vec4(0, 0.25, 0, 1.0),			// 1
		vec4(2.0, 1.0, 0, 1.0),			// 2
		vec4(2.0, 0.25, 0, 1.0),		// 3

		//wall2
		vec4(0, 1.0, 1.0, 1.0),			// 4
		vec4(0, 0.25, 1.0, 1.0),		// 5
		vec4(2.0, 1.0, 1.0, 1.0),		// 6
		vec4(2.0, 0.25, 1.0, 1.0),		// 7

		//Wall 3
		vec4(0, 1.0, 0.0, 1.0),			// 8	//8 9 10, 10 8 11
		vec4(0, 0.25, 0.0, 1.0),		// 9
		vec4(0, 0.25, 1.0, 1.0),		// 10
		vec4(0, 1.0, 1.0, 1.0),			// 11

		//Wall 4
		vec4(2.0, 1.0, 0.0, 1.0),		// 12
		vec4(2.0, 0.25, 0.0, 1.0),		// 13
		vec4(2.0, 0.25, 1.0, 1.0),		// 14
		vec4(2.0, 1.0, 1.0, 1.0),		// 15

		//bottom
		vec4(0,0.25,0, 1.0),			// 20 16 
		vec4(2.0,0.25,0, 1.0),			// 21 17
		vec4(2.0,0.25,1.0, 1.0),		// 22 18 
		vec4(0.0,0.25,1.0,1.0)			// 23 19
	};

	//bottom
	triangleCar(v[16], v[17], v[18]);
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleCar(v[18], v[19], v[16]);
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;

	//Wall 1
	triangleCar(v[0], v[1], v[2]);
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleCar(v[2], v[1], v[3]);
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;

	//Wall 2
	triangleCar(v[4], v[5], v[6]);
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	triangleCar(v[6], v[5], v[7]);
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;

	//Wall 30
	triangleCar(v[8], v[9], v[10]);
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	triangleCar(v[10], v[8], v[11]);
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;

	//Wall 4
	triangleCar(v[12], v[13], v[14]);
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	triangleCar(v[14], v[12], v[15]);
	carTexture[vertexCounter] = vec2(1, 0);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(0, 1);
	vertexCounter++;
	carTexture[vertexCounter] = vec2(1, 1);
	vertexCounter++;
	//buildRoof1();
}

//--------------------------------
// Builds Mountain
//
//	Mountain is just three triangles used to form a pyramid
//	colored with a darker blue for the bottom two vertex
//	and a lighter blue for the top vertex
//

point4 mountainData[9];
vec3 mountainNormals[9];
vec2 mountainTextures[9];

void
triangleMountain(const point4& a, const point4& b, const point4& c)
{
	mountainNormals[Index] = normalize(vec3(a[0], a[1], a[2])); mountainData[Index] = a; Index++;
	mountainNormals[Index] = normalize(vec3(b[0], b[1], b[2])); mountainData[Index] = b; Index++;
	mountainNormals[Index] = normalize(vec3(c[0], c[1], c[2])); mountainData[Index] = c; Index++;
}

void buildMountain() {
	Index = 0;
	int vertexCounter = 0;

	point4 v[15] = {
		//bottomPyramid
		vec4(-10.0, 0.0, 5.0, 1.0),			// 0
		vec4(10.0, 0.0, 5.0, 1.0),			// 1
		vec4(0.0, 10.0, 0.0, 1.0),			// 2
		vec4(0.0, 0.0, -10.0, 1.0),			// 3
	};

	// 0 1 2
	// 1 3 2
	// 0 3 2

	//side 1
	triangleMountain(v[0], v[1], v[2]);
	mountainTextures[vertexCounter] = vec2(-1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(0.5 * 2, 1 * 1);
	vertexCounter++;

	//side 2
	triangleMountain(v[1], v[3], v[2]);
	mountainTextures[vertexCounter] = vec2(-1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(0.5 * 2, 1 * 1);
	vertexCounter++;

	//side 3
	triangleMountain(v[0], v[3], v[2]);
	mountainTextures[vertexCounter] = vec2(-1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(1 * 2, 0);
	vertexCounter++;
	mountainTextures[vertexCounter] = vec2(0.5 * 2, 1 * 1);
	vertexCounter++;
}

//---------------------------------------------------------
//	Building the Sphere
//---------------------------------------------------------
/*const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices = 3 * NumTriangles;*/

point4 points[NumVertices];
vec3   normals[NumVertices];

void
triangle(const point4& a, const point4& b, const point4& c)
{
	normals[Index] = normalize(vec3(a[0], a[1], a[2])); points[Index] = a; sphere_tex_coords[Index] = vec2(a[0], a[2]); Index++;
	normals[Index] = normalize(vec3(b[0], b[1], b[2])); points[Index] = b; sphere_tex_coords[Index] = vec2(b[0], b[2]); Index++;
	normals[Index] = normalize(vec3(c[0], c[1], c[2])); points[Index] = c; sphere_tex_coords[Index] = vec2(c[0], c[2]); Index++;
}

//----------------------------------------------------------------------------

point4
unit(const point4& p)
{
	float len = p.x*p.x + p.y*p.y + p.z*p.z;

	point4 t;
	if (len > DivideByZeroTolerance) {
		t = p / sqrt(len);
		t.w = 1.0;
	}

	return t;
}

void
divide_triangle(const point4& a, const point4& b,
	const point4& c, int count)
{
	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

void
tetrahedron(int count)
{
	Index = 0;
	point4 v[4] = {
		vec4(0.0, 0.0, 1.0, 1.0),
		vec4(0.0, 0.942809, -0.333333, 1.0),
		vec4(-0.816497, -0.471405, -0.333333, 1.0),
		vec4(0.816497, -0.471405, -0.333333, 1.0)
	};

	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);
}

//---------------------------------------------------------
//		Draw Functions
//---------------------------------------------------------
void drawGround()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[ground]);

	color4 material_ambient = vec4(0.5, 0.5, 0.5, 0.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 0.0);
	color4 material_specular = vec4(1.0, 1.0, 1.0, 0.0);
	GLfloat material_shininess = 10.0;

	point4 light_position2 = vec4(5.0, 5.0, 5.0, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = vec4(0, 0, 0,0);//light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);


	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[groundTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawHouse1()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[house1]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[house1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 24);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawHouse2()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[house1]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[house2TextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 24);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof2TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

double zSub = -0.5;
double ySub = 3.5;
double xSub = 0.5;
void drawWheels()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[wheels]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	mat4 instance = Translate(eye.x - xSub, eye.y - ySub, eye.z - zSub)*RotateY(90);
	mat4 wheelChange = Translate(0.25,0,0)*RotateX(90)*Scale(0.25,0.5,0.25);

	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view*instance*wheelChange);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[wheelTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 600);
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);*/

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawWheels2()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[wheels]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	mat4 instance = Translate(eye.x - xSub, eye.y - ySub, eye.z - zSub)*RotateY(90);
	mat4 wheelChange = Translate(0.25, 0, 0.75)*RotateX(90)*Scale(0.25, 0.5, 0.25);
	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view*instance*wheelChange);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[wheelTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 600);
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);*/

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawWheels3()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[wheels]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	mat4 instance = Translate(eye.x - xSub, eye.y - ySub, eye.z - zSub)*RotateY(90);
	mat4 wheelChange = Translate(1.75, 0, 0.75)*RotateX(90)*Scale(0.25, 0.5, 0.25);
	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view*instance*wheelChange);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[wheelTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 600);
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);*/

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawWheels4()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[wheels]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	mat4 instance = Translate(eye.x - xSub, eye.y - ySub, eye.z - zSub)*RotateY(90);
	mat4 wheelChange = Translate(1.75, 0, 0.0)*RotateX(90)*Scale(0.25, 0.5, 0.25);
	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view*instance*wheelChange);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[wheelTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 600);
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 42);*/

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void drawMountain()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[mountain]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[mountainTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 9);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawTree()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[tree]);


	color4 material_ambient = vec4(0.5, 0.5, 0.5, 1.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
	color4 material_specular = vec4(0.5, 0.5, 0.5, 1.0);
	GLfloat material_shininess = 10.0;

	color4 light_position2 = vec4(5, 5.0, 5, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[treeTextureImage]);

	//glDrawArrays(GL_TRIANGLES, 0, 27);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 27, treeGrid);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[trunkTextureImage]);

	glDrawArraysInstanced(GL_TRIANGLES, 27, 600, treeGrid);
	//glDrawArrays(GL_TRIANGLES, 27, 600);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawRoad()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[road]);

	color4 material_ambient = vec4(0.5, 0.5, 0.5, 0.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 0.0);
	color4 material_specular = vec4(1.0, 1.0, 1.0, 0.0);
	GLfloat material_shininess = 10.0;

	point4 light_position2 = vec4(5.0, 5.0, 5.0, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = vec4(0, 0, 0, 0);//light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);


	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[roadTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 24);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[waterTextureImage]);

	glDrawArrays(GL_TRIANGLES, 24, 6);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawCar()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[0]);
	glBindVertexArray(vao[car]);

	color4 material_ambient = vec4(0.5, 0.5, 0.5, 0.0);
	color4 material_diffuse = vec4(0.8, 0.8, 0.8, 0.0);
	color4 material_specular = vec4(1.0, 1.0, 1.0, 0.0);
	GLfloat material_shininess = 10.0;

	point4 light_position2 = vec4(5.0, 5.0, 5.0, 0.0);
	color4 light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	color4 light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	color4 light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

	ambient_product2 = light_ambient2 * material_ambient;
	diffuse_product2 = light_diffuse2 * material_diffuse;
	specular_product2 = light_specular2 * material_specular;

	glUniform4fv(glGetUniformLocation(program[0], "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program[0], "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[0], "SpecularProduct2"), 1, specular_product2);

	//stationary light 
	GLuint light_pos_loc = glGetUniformLocation(program[0], "LightPosition");
	glUniform4fv(light_pos_loc, 1, light_position2);

	//Thanks Levi and banky!
	mat4 instance = Translate(eye.x - xSub, eye.y - ySub, eye.z - zSub)*RotateY(90);
	//Angel::vec4 eye = vec4(63, 1.5, 30, 1.0);
	//---------------------//
	ModelView = glGetUniformLocation(program[0], "ModelView");
	NormalView = glGetUniformLocation(program[0], "NormalView");
	Projection = glGetUniformLocation(program[0], "Projection");
	camera_loc = glGetUniformLocation(program[0], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view*instance);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[0], "Shininess"), material_shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[carTextureImage]);

	glDrawArrays(GL_TRIANGLES, 0, 30);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void drawWindshield()
{
	//stack.push(model_view);
	//normalStack.push(normal_view);

	glUseProgram(program[2]);
	glBindVertexArray(vao[windshield]);


	ModelView = glGetUniformLocation(program[0], "ModelView");
	Projection = glGetUniformLocation(program[0], "Projection");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	//model_view = stack.pop();
	//normal_view = normalStack.pop();

	glutPostRedisplay();
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//---------------------------------------------------------
//	Setting up Nodes for the scene
//---------------------------------------------------------
struct Node {
	mat4  transform;
	void(*render)(void);
	Node* sibling;
	Node* child;

	Node() :
		render(NULL), sibling(NULL), child(NULL) {}

	Node(mat4& m, void(*render)(void), Node* sibling, Node* child) :
		transform(m), render(render), sibling(sibling), child(child) {}
};

void traverse(Node* node)
{
	if (node == NULL) { return; }

	mvstack.push(model_view);

	model_view *= node->transform;
	node->render();

	if (node->child) { traverse(node->child); }

	model_view = mvstack.pop();

	if (node->sibling) { traverse(node->sibling); }
}

//objects in scene
enum {
	groundnode = 0,
	house1node = 1,
	house2node = 2,
	house3node = 3,
	treenode = 4,
	roadnode = 5,
	house4node = 6,
	carnode =7,
	windshieldnode = 8,
	mountainnode = 9,
	mountainnode2 = 10,
	mountainnode3 = 11,
	wheelnode = 12,
	wheelnode2 = 13,
	wheelnode3 = 14,
	wheelnode4 = 15,
	NumNodes
};
Node  nodes[NumNodes];

//---------------------------------------------------------
//	Tree Scene
//---------------------------------------------------------
void initScene(void) {
	mat4  m;

	//objects
	m = mat4(1.0)*Translate(20.0, 0, 0);
	nodes[groundnode] = Node(m, drawGround, &nodes[house1node], NULL);

	m = mat4(1.0)*Scale(5.0, 5.0, 5.0);
	nodes[house1node] = Node(m, drawHouse1, &nodes[treenode], &nodes[house2node]);

	m = mat4(1.0)*Translate(-40, 0.0, 20);
	nodes[treenode] = Node(m, drawTree, &nodes[roadnode], NULL);

	m = mat4(1.0);
	nodes[roadnode] = Node(m, drawRoad, &nodes[carnode], NULL);

	m = mat4(1.0);
	nodes[carnode] = Node(m, drawCar, &nodes[mountainnode], &nodes[wheelnode]);

	m = mat4(1.0)*Translate(0.0,0.0,-70)*Scale(1,2,1);
	nodes[mountainnode] = Node(m, drawMountain, &nodes[windshieldnode], &nodes[mountainnode2]);

	m = mat4(1.0)*Translate(0, 0, -10) * Scale(10,10,10);
	nodes[windshieldnode] = Node(m, drawWindshield, NULL, NULL);

	//children
	//--- house1 ---//
	m = mat4(1.0)*Translate(8.0, 0.0, 2.0);
	nodes[house2node] = Node(m, drawHouse1, &nodes[house3node], NULL);
	
	m = mat4(1.0)*Translate(1.5, 0.0, -8.0)*Scale(1.5, 1.5, 1.5);
	nodes[house3node] = Node(m, drawHouse1, &nodes[house4node], NULL);

	m = mat4(1.0)*Translate(10.0, 0.0, -7.0)*Scale(1.5, 1.5, 1.5);
	nodes[house4node] = Node(m, drawHouse2, NULL, NULL);

	//--- mountain ---//
	m = mat4(1.0)*Translate(10,0,5)*RotateY(90)*Scale(1, 0.5, 1);
	nodes[mountainnode2] = Node(m, drawMountain, &nodes[mountainnode3], NULL);

	m = mat4(1.0)*Translate(-10, 0, 5)*RotateY(45)*Scale(2, 0.5, 1);
	nodes[mountainnode3] = Node(m, drawMountain, NULL, NULL);

	//--- car  ---//
	m = mat4(1.0);
	nodes[wheelnode] = Node(m, drawWheels, NULL, &nodes[wheelnode2]);

	m = mat4(1.0);
	nodes[wheelnode2] = Node(m, drawWheels2, NULL, &nodes[wheelnode3]);

	m = mat4(1.0);
	nodes[wheelnode3] = Node(m, drawWheels3, NULL, &nodes[wheelnode4]);

	m = mat4(1.0);
	nodes[wheelnode4] = Node(m, drawWheels4, NULL, NULL);
}

void buildInit() {
	//----------------------------------------------------------------------
	// initializes the build functions
	//----------------------------------------------------------------------
	buildGround();
	buildHouse1();
	buildTree();
	buildRoad();
	buildCar();
	buildMountain();
	buildWindshield();
	buildWheels();
}

void init(void) {

	//instancinginit
	srand((unsigned)time(0));
	for (int i = 0; i < treeGrid; i++) {
		for (int j = 0; j < treeGrid; j++) {
			shiftATTR[i*treeGrid + j] = vec4(((float)rand() / RAND_MAX) * 100, 0, abs(((float)rand() / RAND_MAX) * j + 15), 0);
		}
	}

	groundImage = glmReadPPM("smallgrass.ppm", &TextureWidth[groundTextureImage], &TextureHeight[groundTextureImage]);
	house1Image = glmReadPPM("house1Body.ppm", &TextureWidth[house1TextureImage], &TextureHeight[house1TextureImage]);
	roof1Image = glmReadPPM("house2.ppm", &TextureWidth[roof1TextureImage], &TextureHeight[roof1TextureImage]);
	house2Image = glmReadPPM("house2Body.ppm", &TextureWidth[house2TextureImage], &TextureHeight[house2TextureImage]);
	roof2Image = glmReadPPM("house1.ppm", &TextureWidth[roof2TextureImage], &TextureHeight[roof2TextureImage]);
	trunkImage = glmReadPPM("treeTrunk.ppm", &TextureWidth[trunkTextureImage], &TextureHeight[trunkTextureImage]);
	treeImage = glmReadPPM("treeLeaves.ppm", &TextureWidth[treeTextureImage], &TextureHeight[treeTextureImage]);
	roadImage = glmReadPPM("road.ppm", &TextureWidth[roadTextureImage], &TextureHeight[roadTextureImage]);
	carImage = glmReadPPM("car.ppm", &TextureWidth[carTextureImage], &TextureHeight[carTextureImage]);
	mountainImage = glmReadPPM("cliff_mountain.ppm", &TextureWidth[mountainTextureImage], &TextureHeight[mountainTextureImage]);
	wheelImage = glmReadPPM("wheel.ppm", &TextureWidth[wheelTextureImage], &TextureHeight[wheelTextureImage]);
	waterImage = glmReadPPM("water.ppm", &TextureWidth[waterTextureImage], &TextureHeight[waterTextureImage]);

	// Initialize texture objects
	glGenTextures(12, textures);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, textures[groundTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[groundTextureImage], TextureHeight[groundTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, groundImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[house1TextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[house1TextureImage], TextureHeight[house1TextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, house1Image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[roof1TextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[roof1TextureImage], TextureHeight[roof1TextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, roof1Image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[treeTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[treeTextureImage], TextureHeight[treeTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, treeImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[trunkTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[trunkTextureImage], TextureHeight[trunkTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, trunkImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[roadTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[roadTextureImage], TextureHeight[roadTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, roadImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[house2TextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[house2TextureImage], TextureHeight[house2TextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, house2Image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[roof2TextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[roof2TextureImage], TextureHeight[roof2TextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, roof2Image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[carTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[carTextureImage], TextureHeight[carTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, carImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[mountainTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[mountainTextureImage], TextureHeight[mountainTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, mountainImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[wheelTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[wheelTextureImage], TextureHeight[wheelTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, wheelImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[waterTextureImage]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[waterTextureImage], TextureHeight[waterTextureImage], 0,
		GL_RGB, GL_UNSIGNED_BYTE, waterImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Load Skybox Images. 6 images to represent the 6 angles of view. Inside it's own structured Cubemap
	skybox.top = glmReadPPM("skybox\\skytop.ppm", &skybox.topWidth, &skybox.topHeight);
	skybox.bottom = glmReadPPM("skybox\\skybottom.ppm", &skybox.bottomWidth, &skybox.bottomHeight);
	skybox.right = glmReadPPM("skybox\\skyright.ppm", &skybox.rightWidth, &skybox.rightHeight);
	skybox.left = glmReadPPM("skybox\\skyleft.ppm", &skybox.leftWidth, &skybox.leftHeight);
	skybox.front = glmReadPPM("skybox\\skyfront.ppm", &skybox.frontWidth, &skybox.frontHeight);
	skybox.back = glmReadPPM("skybox\\skyback.ppm", &skybox.backWidth, &skybox.backHeight);


	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texture);

	int isEnabled = 0;

	if (glIsEnabled(GL_TEXTURE_CUBE_MAP) == GL_TRUE) { isEnabled = 1; }
	else { isEnabled = 0; };


	std::cout << isEnabled << std::endl;

	glEnable(GL_TEXTURE_CUBE_MAP);


	if (glIsEnabled(GL_TEXTURE_CUBE_MAP) == GL_TRUE) { isEnabled = 1; }
	else { isEnabled = 0; };

	std::cout << isEnabled << std::endl;


	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, skybox.topWidth, skybox.topHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.top);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, skybox.bottomWidth, skybox.bottomHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.bottom);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, skybox.rightWidth, skybox.rightHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.right);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, skybox.leftWidth, skybox.rightWidth, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.left);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, skybox.frontWidth, skybox.frontHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.front);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, skybox.backWidth, skybox.backHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, skybox.back);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//shaders
	program[0] = InitShader("vLIGHTS_v120.glsl", "fLIGHTS_v120.glsl");
	program[1] = InitShader("./shaders/skyboxvertex.glsl", "./shaders/skyboxfragment.glsl");
	program[2] = InitShader("vShaderColor.glsl", "fShaderColor.glsl");

	//----------------------------------------------------------------------
	// initializing the buffers
	//----------------------------------------------------------------------
	glGenVertexArrays(12, vao);
	glGenBuffers(12, buffers);

	//ground
	glBindVertexArray(vao[ground]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[ground]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundData) + sizeof(groundNormals) + sizeof(groundTextures), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(groundData), groundData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(groundData), sizeof(groundNormals), groundNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(groundData) + sizeof(groundNormals), sizeof(groundTextures), groundTextures);

	glUseProgram(program[0]);

	GLuint vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(groundData)));

	GLuint vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(groundData) + sizeof(groundNormals)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);

	glBindVertexArray(0);
	glUseProgram(0);

	//carinit
	glBindVertexArray(vao[car]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[car]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(carData) + sizeof(carNormals) + sizeof(carTexture), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(carData), carData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(carData), sizeof(carNormals), carNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(carData) + sizeof(carNormals), sizeof(carTexture), carTexture);

	glUseProgram(program[0]);

	 vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	 vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(carData)));

	 vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(carData) + sizeof(carNormals)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);

	glBindVertexArray(0);
	glUseProgram(0);

	//windshieldinit
	glBindVertexArray(vao[windshield]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[windshield]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windShieldData), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(windShieldData), windShieldData);

	glUseProgram(program[2]);

	 vPosition = glGetAttribLocation(program[2], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);
	glUseProgram(0);

	//house1
	glBindVertexArray(vao[house1]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[house1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(house1Data) + sizeof(house1Normals) + sizeof(house1BuildingTexture) + sizeof(roof1Texture), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(house1Data), house1Data);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(house1Data), sizeof(house1Normals), house1Normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(house1Data) + sizeof(house1Normals), sizeof(house1BuildingTexture), house1BuildingTexture);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(house1Data) + sizeof(house1Normals)+ sizeof(house1BuildingTexture), sizeof(roof1Texture), roof1Texture);

	glUseProgram(program[0]);

	 vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	 vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(house1Data)));

	 vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(house1Data) + sizeof(house1Normals)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord2");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(house1Data) + sizeof(house1Normals) + sizeof(house1BuildingTexture)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);
	glUniform1i(glGetUniformLocation(program[0], "texture2"), 1);

	glBindVertexArray(0);
	glUseProgram(0);

	//wheelsinit
	glBindVertexArray(vao[wheels]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[wheels]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wheelData) + sizeof(wheelNormal) + sizeof(wheelTexture), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(wheelData), wheelData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(wheelData), sizeof(wheelNormal), wheelNormal);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(wheelData) + sizeof(wheelNormal), sizeof(wheelTexture), wheelTexture);

	glUseProgram(program[0]);

	vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(wheelData)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(wheelData) + sizeof(wheelNormal)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);

	glBindVertexArray(0);
	glUseProgram(0);

	//treeinit
	glBindVertexArray(vao[tree]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[tree]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderData) + sizeof(treeData) + sizeof(cylinderNormal) + sizeof(treeNormals) + sizeof(cylinder_tex_coords) + sizeof(treeTextures) + sizeof(shiftATTR), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(treeData), treeData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(treeData), sizeof(cylinderData), cylinderData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(treeData)+ sizeof(cylinderData), sizeof(treeNormals), treeNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(treeData) + sizeof(treeNormals) + sizeof(cylinderData), sizeof(cylinderNormal), cylinderNormal);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(treeData) + sizeof(treeNormals) + sizeof(cylinderData) + sizeof(cylinderNormal), sizeof(treeTextures), treeTextures);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(cylinderData) + sizeof(treeData) + sizeof(treeNormals) + sizeof(cylinderNormal) + sizeof(treeTextures), sizeof(cylinder_tex_coords), cylinder_tex_coords);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(cylinderData) + sizeof(treeData) + sizeof(treeNormals) + sizeof(cylinderNormal) + sizeof(treeTextures) + sizeof(cylinder_tex_coords), sizeof(shiftATTR), shiftATTR);

	glUseProgram(program[0]);

	vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderData) + sizeof(treeData)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(cylinderData) + sizeof(cylinderNormal) + sizeof(treeData) + sizeof(treeNormals)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord2");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(cylinderData) + sizeof(cylinderNormal) + sizeof(treeData) + sizeof(treeNormals) + sizeof(treeTextures)));

	vShift = glGetAttribLocation(program[0], "vShift");
	glEnableVertexAttribArray(vShift);
	glVertexAttribPointer(vShift, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderData) + sizeof(cylinderNormal) + sizeof(treeData) + sizeof(treeNormals) + sizeof(treeTextures) + sizeof(cylinder_tex_coords)));
	glVertexAttribDivisor(vShift, 1);

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);
	glUniform1i(glGetUniformLocation(program[0], "texture2"), 1);

	glBindVertexArray(0);
	glUseProgram(0);

	//road
	glBindVertexArray(vao[road]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[road]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(roadData) + sizeof(roadNormals) + sizeof(roadTextures) + sizeof(waterTextures), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roadData), roadData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roadData), sizeof(roadNormals), roadNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roadData) + sizeof(roadNormals), sizeof(roadTextures), roadTextures);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roadData) + sizeof(roadNormals) + sizeof(roadTextures), sizeof(waterTextures), waterTextures);

	glUseProgram(program[0]);

	 vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	 vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(roadData)));

	 vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(roadData) + sizeof(roadNormals)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord2");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(roadData) + sizeof(roadNormals) + sizeof(roadTextures)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);
	glUniform1i(glGetUniformLocation(program[0], "texture2"), 1);

	glBindVertexArray(0);
	glUseProgram(0);


	//mountaininit
	glBindVertexArray(vao[mountain]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[mountain]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mountainData) + sizeof(mountainNormals) + sizeof(mountainTextures), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mountainData), mountainData);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(mountainData), sizeof(mountainNormals), mountainNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(mountainData) + sizeof(mountainNormals), sizeof(mountainTextures), mountainTextures);

	glUseProgram(program[0]);

	vPosition = glGetAttribLocation(program[0], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(program[0], "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(mountainData)));

	vTexCoord = glGetAttribLocation(program[0], "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(mountainData) + sizeof(mountainNormals)));

	glUniform1i(glGetUniformLocation(program[0], "texture"), 0);

	glBindVertexArray(0);
	glUseProgram(0);

	
	//skybox
	glBindBuffer(GL_ARRAY_BUFFER, buffers[sky]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices_positions), cube_vertices_positions, GL_STATIC_DRAW);

	glUseProgram(program[1]);
	glBindVertexArray(vao[sky]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[sky]);
	vPosition = glGetAttribLocation(program[1], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	//done with this packet
	glBindVertexArray(0);
	glUseProgram(0);

	initScene();

	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT, GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glEnable(GL_DEPTH_TEST);
	//glClearDepth(1.0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

bool switchview = false;
//---------------------------------------------------------
//		Display Function
//---------------------------------------------------------
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projection = Perspective(90, 1.0, 0.1, 10.0);
	model_view = mat4(1.0);

	//Sky Box @  infinity
	vec4 skyboxEye = vec4(0.0, 0.0, 0.0, 1.0);
	model_view = LookAt(skyboxEye, skyboxEye - n, v);

	glUseProgram(program[1]);
	GLuint proj_loc = glGetUniformLocation(program[1], "Projection");
	GLuint model_view_loc = glGetUniformLocation(program[1], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projection);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);
	glUniform1i(glGetUniformLocation(program[1], "CubeMap"), 0);

	glBindVertexArray(vao[sky]);

	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_QUADS, 0, 24);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glUseProgram(0);

	model_view = Angel::mat4(1.0);
	model_view = LookAt(eye, eye - n, v)*model_view;
	normal_view = mat4(1.0);
	if (switchview == true) {
		model_view = Angel::mat4(1.0);
		model_view = LookAt(eye, eye - n, v)*model_view;
	}
	else {
		model_view = Angel::mat4(1.0);
		model_view = LookAt(car_eye, car_eye - n, v)*model_view;
	}

	traverse(&nodes[groundnode]);

	glutSwapBuffers();
}

//---------------------------------------------------------
//		Main Function
//---------------------------------------------------------
void constants() {
	

	glutReshapeFunc(reshape);
	glutSpecialFunc(specKey);
	glutTimerFunc(20, animate, 1);
	glutIdleFunc(idle);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(currentWidth, currentHeight);

	buildInit();

	int window1 = glutCreateWindow("Texture Mapping Assignment");
	m_glewInitAndVersion();

	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	constants();

	glutMainLoop();
	return 0;
}

void idle(void)
{
	spin += spin_step; // movement for the spinning light

	glutPostRedisplay();
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	currentHeight = h; currentWidth = w;
}

void animate(int j) {
	eye += speedForward*0.02*n;
	eye += speedRight*0.02*u;
	glutTimerFunc(20, animate, 1);
	glutPostRedisplay();
}


//---------------------------------------------------------
//		Keyboard Functions
//---------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	int count = 0;
	GLfloat xt, yt, zt;
	GLfloat cosine, sine;

	//prees  spacebar to  return to default (initial) camera position and oreintation 
	if (key == 32)
	{
		vec4 n = vec4(0.0, 0.0, 1.0, 0.0);
		vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
		vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
		vec4 eye = vec4(0.0, 0.0, 2.0, 1.0);
	}

	// positive or negative rotation depending on upper or lower case letter
	if (key > 96)
	{
		cosine = cos(SCALE_ANGLE * PI / -180.0);
		sine = sin(SCALE_ANGLE * PI / -180.0);
	}
	else
	{
		cosine = cos(SCALE_ANGLE * PI / 180.0);
		sine = sin(SCALE_ANGLE * PI / 180.0);
	}

	switch (key)
	{

	case 'Z': // roll counterclockwise in the xy plane
	case 'z': // roll clockwise in the xy plane
		xt = u[X];
		yt = u[Y];
		zt = u[Z];
		u[X] = xt*cosine - v[X] * sine;
		u[Y] = yt*cosine - v[Y] * sine;
		u[Z] = zt*cosine - v[Z] * sine;
		v[X] = xt*sine + v[X] * cosine;
		v[Y] = yt*sine + v[Y] * cosine;
		v[Z] = zt*sine + v[Z] * cosine;
		break;
	case 'X'://down
	case 'x': //up
		xt = v[X];
		yt = v[Y];
		zt = v[Z];
		v[X] = xt*cosine - n[X] * sine;
		v[Y] = yt*cosine - n[Y] * sine;
		v[Z] = zt*cosine - n[Z] * sine;
		n[X] = xt*sine + n[X] * cosine;
		n[Y] = yt*sine + n[Y] * cosine;
		n[Z] = zt*sine + n[Z] * cosine;
		break;
	case 'Y'://side to side
	case 'y':
		xt = u[X];
		yt = u[Y];
		zt = u[Z];
		u[X] = xt*cosine - n[X] * sine;
		u[Z] = zt*cosine - n[Z] * sine;
		n[X] = xt*sine + n[X] * cosine;
		n[Z] = zt*sine + n[Z] * cosine;
		break;
	case 033:  // Escape key
		exit(EXIT_SUCCESS);
	case 'q': case 'Q':
		glutHideWindow();
		break;
	case 'v': case 'V':
		if (switchview == true) {
			ySub = 3.5;
			zSub = -0.5;
			switchview = false;
		}
		else {
			ySub = 3.5;
			zSub = 10;
			switchview = true;
		}
		break;

	}
	glutPostRedisplay();
}


void specKey(int key, int x, int y)
{
	// Car doesn't fly or go through ground
	// Thanks, Banky!
	switch (key) {
	case GLUT_KEY_UP: // MOVE FORWARD
		eye[0] -= SCALE_VECTOR * n[0];
		eye[2] -= SCALE_VECTOR * n[2];
		car_eye[0] -= SCALE_VECTOR * n[0];
		car_eye[2] -= SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_DOWN: // MOVE BACKWARD
		eye[0] += SCALE_VECTOR * n[0];
		eye[2] += SCALE_VECTOR * n[2];
		car_eye[0] += SCALE_VECTOR * n[0];
		car_eye[2] += SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_LEFT: // MOVE LEFT
		eye[0] -= SCALE_VECTOR * u[0];
		eye[2] -= SCALE_VECTOR * u[2];
		car_eye[0] -= SCALE_VECTOR * u[0];
		car_eye[2] -= SCALE_VECTOR * u[2];
		break;
	case GLUT_KEY_RIGHT: // MOVE RIGHT
		eye[0] += SCALE_VECTOR * u[0];
		eye[2] += SCALE_VECTOR * u[2];
		car_eye[0] += SCALE_VECTOR * u[0];
		car_eye[2] += SCALE_VECTOR * u[2];
		break;
	default:
		break;
	}
	glutPostRedisplay();
}


void m_glewInitAndVersion(void)
{
	fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

//////////////////////////////
//Mouse Functions
//////////////////////////////


int startx;
int starty;
int moving;

GLsizei ww;
GLsizei wh;
int pFlag = 1;
int counter = 0;

static void
mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			startx = x;
			starty = y;
			moving = 1;
		}
		if (state == GLUT_UP) {
			moving = 0;
		}
	}
}

static void
motion(int x, int y) {
	if (moving) {
		thetaY = thetaY + (x - startx);
		theta = theta + (y - starty);
		startx = x;
		starty = y;
		glutPostRedisplay();
	}
}

/*Angel::vec4 rayOriginWorldCoords;
Angel::vec4 rayDirectionWorldCoords;
Angel::vec4 rayOriginObjectCoords;
Angel::vec4 rayDirectionObjectCoords;

GLfloat firstTimeOfPenetration(const Angel::vec4& origin, const Angel::vec4& direction, const GLfloat maxExitTime);
mat4 lookAtInverse(const vec4& eye, const vec4& u, const vec4& v, const vec4& n);
mat4 modelingTransformInverse(void);

//Perspective ------------------------------------------------------
GLfloat fovy = 90.0;
GLfloat aspect = 1.0;
GLfloat zNear = 1.0;
GLfloat zFar = 5.0;
//------------------------------------------------------------------

mat4 lookAtInverse(const vec4& eye, const vec4& u, const vec4& v, const vec4& n)
{
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = transpose(mat4(u, v, n, t));
	c[0][3] = eye[0];
	c[1][3] = eye[1];
	c[2][3] = eye[2];
	c[3][3] = 1.0;
	return c;
}

mat4 modelingTransformInverse(void)
{
	mat4 cc(1.0);
	return cc;
}

//Object is single square 
//  z= 0, -1 =< x, y =< 1
GLfloat firstTimeOfPenetration(const Angel::vec4& origin, const Angel::vec4& direction, const GLfloat maxExitTime)
{
	GLfloat s;

	if (direction[2] == 0) { return 0.0; }
	else {
		s = -origin[2] / direction[2];
		GLfloat x2 = (origin[0] + s*direction[0])*(origin[0] + s*direction[0]);
		GLfloat y2 = (origin[1] + s*direction[1])*(origin[1] + s*direction[1]);
		if ((1 >= x2) && (1 >= y2) && (s >= 1) && (maxExitTime >= s)) { return s; }
		else { return 0.0; }
	}

}

static void mouse(int button, int state, int x, int y)
{
	GLfloat wx = 2.0*x / ww - 1.0;
	GLfloat wy = 1.0 - 2.0*y / wh;

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		GLfloat t = zNear*tan(fovy*DegreesToRadians / 2);
		GLfloat r = t*aspect;
		rayDirectionWorldCoords = lookAtInverse(eye, u, v, n)*vec4(r*wx, t*wy, -zNear, 0.0);
		rayOriginWorldCoords = lookAtInverse(eye, u, v, n)*vec4(0.0, 0.0, 0.0, 1.0);

		rayDirectionObjectCoords = modelingTransformInverse()*rayDirectionWorldCoords;
		rayOriginObjectCoords = modelingTransformInverse()*rayOriginWorldCoords;
		GLfloat s = firstTimeOfPenetration(rayOriginObjectCoords, rayDirectionObjectCoords, zFar / zNear);

		if (s > 0) { pFlag = -pFlag; }

		int window2 = glutCreateWindow("Cylinder");
		m_glewInitAndVersion();

		init();
		glutDisplayFunc(display2);
		glutKeyboardFunc(keyboard2);
		constants();
	}


	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON)
	{
		int window2 = glutCreateWindow("Cylinder");
		m_glewInitAndVersion();

		init();
		glutDisplayFunc(display3);
		glutKeyboardFunc(keyboard3);
		constants();

	}

	if (state == GLUT_DOWN && button == GLUT_MIDDLE_BUTTON)
	{
		int window2 = glutCreateWindow("Cylinder");
		m_glewInitAndVersion();

		init();
		glutDisplayFunc(display4);
		glutKeyboardFunc(keyboard4);
		constants();

	}
}*/


