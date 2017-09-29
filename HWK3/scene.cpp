//Shanan Almario
//CSCE 385
//
//Assignment3 Problem 1
//	 This code should show a little town made up of trees, houses, roads, and a random spot of water with a mountain in the back
//	 Mouse interaction and keyboard interaction has been implemented for debugging purposes
//
//Interaction:
//	 Keyboard:
//		z/Z = roll the camera 
//		y/Y yaw the camera  
//		x/X pitch the camera 
//	 
//		up   Special Key = slide forward -- implemented
//		down Special Key = slide backward -- you must implement
//		SPACEBAR  return to default (initial) camera position and oreintation 
//
//	 Mouse:
//		 If the mouse is moved up, then the object is rotated around the x axis, clockwise in the y?z coordinate plane.
//		 If the mouse is moved horizontally to the right/left, then the object is rotated around the y axis, clockwise in the z ? x coordinate plane.
//
//Compiling:
//Shaders Used:
//  vshaderPerVertColor.glsl
//	vshader_Inst.glsl
//  fshaderStock.glsl
//


#include "Angel.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>



//------------------------------------------------------------------
//functions
//------------------------------------------------------------------
void m_glewInitAndVersion(void);
void reshape(int width, int height);
void specKey(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);

void drawGround(void);//ground
void drawHouse1(void);//house1
void drawHouse2(void);//house2
void drawTree(void);//drawtree
void drawMountain(void);//drawMountain
void drawRoad(void); //drawRoad and water

					//  Define PI in the case it's not defined in the math header file


typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0

/////Camera unv basis///////

Angel::vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 n = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 eye = vec4(5.0, 1.0, 25.0, 1.0);

int startx;
int starty;
int moving;

GLfloat theta = 0;
GLfloat thetaY = 0;

GLfloat step = 20;
GLfloat theta_y = 0;
int spinToggle = 0;
GLfloat d = 0.5;
int w; int h;

//-------------------------------
// initialize
//----------------------------------

const int treeGrid = 80;
const int NumInstances = treeGrid*treeGrid;

#ifndef PI
#  define PI  3.14159265358979323846
#endif

GLuint buffers[13];

// Create a vertex array object
GLuint vao[6];

GLuint program[6];

GLuint vPosition;
GLuint vColor;
GLuint vShift;


//------------------------------------
//uniform variables locations

GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;

mat4 modelviewStackTop = mat4(1.0);
mat4 modelviewStackBottom = mat4(1.0);

//------------------------------------


//-----------------------------------------------
// Vertices and color initialize 
//-----------------------------------------------

int house1points = 66;
int house2points = 42;
int groundpoints = 6;
int treepoints = 627;
int mountainpoints = 9;
int roadpoints = 42;

point4 house1Data[66];
point4 house2Data[42];
point4 groundData[6];
point4 treeData[627];
point4 mountainData[9];
point4 roadData[42];
color4 color[66];
color4 house2Color[42];
color4 groundColor[6];
color4 treeColor[627];
color4 mountainColor[9];
color4 roadColor[42];

vec4 shiftATTR[NumInstances];

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
mat4         model_view = mat4(1.0);
mat4         projmat = mat4(1.0);

//--------------------------------------
//   Objects 
//--------------------------------------

enum {
	house1 = 0,
	house4 = 1,
	ground = 2,
	tree = 3,
	mountain = 4,
	road = 5,
	house2 = 6,
	house3 = 7,
	mountain2 = 8,
	mountain3 = 9,
	NumNodes
};

//-------SG DS-------------------------------------------------------------

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

Node  nodes[NumNodes];


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

//--------------------------------
// Builds House Type 1
//
//	This house is the one with a flat roof
//	colored with a lighter green and brown
//

void buildHouse1() {
	int vertexCounter = 0;
	color4 housecolor = vec4(0.804, 0.522, 0.247, 1.0);
	color4 roofColor = vec4(0.2, 0.7, 0.4, 1.0);
	color4 roofhighlight = vec4(0.22, 0.72, 0.42, 1.0);

		//Wall 1
		house1Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0,1.0,0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0,1.0,0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0,0,0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0,0,0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;

		//Wall 2
		house1Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;

		//Wall 3
		house1Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;

		//Wall 4
		house1Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 0.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); color[vertexCounter] = housecolor;
		vertexCounter++;

		//trapezoid roof front
		house1Data[vertexCounter] = vec4(0,1.0,0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.0, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.0, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;

		//trapezoid roof back
		house1Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;

		//roof rectangle on one side
		house1Data[vertexCounter] = vec4(0.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 1.0,0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 1.0, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;

		//roof rectangle on other side 
		house1Data[vertexCounter] = vec4(1.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); color[vertexCounter] = roofColor;
		vertexCounter++;

		//roof rectangle top 
			//color is lighter to show the sun shines down
		house1Data[vertexCounter] = vec4(0.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(0.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 0.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
		house1Data[vertexCounter] = vec4(1.5, 1.5, 2.0, 1.0); color[vertexCounter] = roofhighlight;
		vertexCounter++;
}

//--------------------------------
// Builds House Type 2
//
//	This house is the one with a triangular roof
//	colored with a bluish roof and a darker brown 
//
void buildHouse2() {
	int vertexCounter = 0;
	color4 housecolor = vec4(0.7, 0.4, 0.15, 1.0);
	color4 roofColor = vec4(0.000, 0.545, 0.645, 1.0);

	//Wall 1
	house2Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 0, 1.0); house2Color[vertexCounter] = housecolor;//vec4(0.96, 0.64, 0.37, 0.5);
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 0, 0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;

	//wall 2
	house2Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;

	//wall 3
	house2Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;

	//wall 4
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 0.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = housecolor;
	vertexCounter++;

	//triangle roof front
	house2Data[vertexCounter] = vec4(0, 1.0, 0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;

	//triangle roof back
	house2Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;

	//roof rectangle on one side
	house2Data[vertexCounter] = vec4(1.0, 1.5, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;

	//roof rectangle on the other side
	house2Data[vertexCounter] = vec4(1.0, 1.5, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 0.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(2.0, 1.0, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
	house2Data[vertexCounter] = vec4(1.0, 1.5, 2.0, 1.0); house2Color[vertexCounter] = roofColor;
	vertexCounter++;
}

//--------------------------------
// Builds the plane
//
//	This is just a giant sqare whose with a large x and z
//	and a small y
//	This will appear to be the land where the town is on
//
void buildGround() {
	int vertexCounter = 0;
	color4 groundcolor = vec4(0.000, 0.502, 0.000, 1.0);

	groundData[vertexCounter] = vec4(-77, 0.0, -77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	groundData[vertexCounter] = vec4(77, 0.0, 77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	groundData[vertexCounter] = vec4(-77, 0.0, 77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	groundData[vertexCounter] = vec4(-77, 0.0, -77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	groundData[vertexCounter] = vec4(77, 0.0, -77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	groundData[vertexCounter] = vec4(77, 0.0, 77, 1.0); groundColor[vertexCounter] = groundcolor;
	vertexCounter++;
	
}

//--------------------------------
// Builds Mountain
//
//	Mountain is just three triangles used to form a pyramid
//	colored with a darker blue for the bottom two vertex
//	and a lighter blue for the top vertex
//

void buildMountain() {
	int vertexCounter = 0;
	color4 mountaincolor = vec4(0.255, 0.412, 0.882, 1.0);
	color4 mountainHighlight = vec4(0.690, 0.878, 0.902, 1.0);

	//side 1
	mountainData[vertexCounter] = vec4(-10.0, 0.0, 5.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(10.0, 0.0, 5.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(0.0, 10.0, 0.0, 1.0); mountainColor[vertexCounter] = mountainHighlight;
	vertexCounter++;
	//side 2
	mountainData[vertexCounter] = vec4(-10.0, 0.0, 5.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(0.0, 0.0, -10.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(0.0, 10.0, 0.0, 1.0); mountainColor[vertexCounter] = mountainHighlight;
	vertexCounter++;
	//side 3
	mountainData[vertexCounter] = vec4(10.0, 0.0, 5.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(0.0, 0.0, -10.0, 1.0); mountainColor[vertexCounter] = mountaincolor;
	vertexCounter++;
	mountainData[vertexCounter] = vec4(0.0, 10.0, 0.0, 1.0); mountainColor[vertexCounter] = mountainHighlight;
	vertexCounter++;
}

//--------------------------------
// Builds Road
//
//	The roads and stretch of water are all rectangles
//	Roads are constructed as rectangles of the same color 
//	placed next to each other
//

void buildRoad() {
	int vertexCounter = 0;
	color4 roadcolor = vec4(0.957, 0.643, 0.376, 1.0);
	color4 watercolor = vec4(0.0, 0.3, 0.706, 1.0);

	//roads
	//each block are patches of rectangles placed next to each other
	
	//rectangle1
	roadData[vertexCounter] = vec4(-3.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(-3.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(14.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(14.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(-3.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(14.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//rectangle 2
	roadData[vertexCounter] = vec4(3.0, 0.01, -6.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(3.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(7.0, 0.01, -6.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(7.0, 0.01, -6.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(3.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(7.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//rectangle3
	roadData[vertexCounter] = vec4(3.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(3.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(3.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//rectangle 4
	roadData[vertexCounter] = vec4(11.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(13.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(11.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(13.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(11.0, 0.01, -1.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(13.0, 0.01, -3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//rectangle 6
	roadData[vertexCounter] = vec4(11.0, 0.01, 5.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 5.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(11.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(11.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 5.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 6.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//rectangle 7
	roadData[vertexCounter] = vec4(5.0, 0.01, 3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 4.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(1.0, 0.01, 4.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(1.0, 0.01, 4.5, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(1.0, 0.01, 3.0, 1.0);; roadColor[vertexCounter] = roadcolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(5.0, 0.01, 3.0, 1.0); roadColor[vertexCounter] = roadcolor;
	vertexCounter++;

	//Rectangle 8
	//patch of water on the left
	roadData[vertexCounter] = vec4(15, 0.01, -3, 1.0); roadColor[vertexCounter] = watercolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(30, 0.01, -3, 1.0); roadColor[vertexCounter] = watercolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(30, 0.01, -1, 1.0); roadColor[vertexCounter] = watercolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(30, 0.01, -1, 1.0); roadColor[vertexCounter] = watercolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(15, 0.01, -3, 1.0); roadColor[vertexCounter] = watercolor;
	vertexCounter++;
	roadData[vertexCounter] = vec4(15, 0.01, -1, 1.0); roadColor[vertexCounter] = watercolor;
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

void buildTree() {
	GLfloat x, z, theta;
	int iMax = 50, vertexCounter = 0;
	color4 treeTrunk = vec4(0.545, 0.271, 0.075, 0.5);
	color4 treeHighlight = vec4(0.604, 0.804, 0.196, 1.0);
	color4 treeLeaves = vec4(0.133, 0.545, 0.133, 1.0);

	//build brown cylinder sides
	//the for loop is the cylinder creation
	for (int i = 0; i<iMax; i++) {
		theta = (2 * PI / (float)iMax)*i; x = cos(theta)/5; z = sin(theta)/5;
		treeData[vertexCounter] = vec4(x, 1.0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;
		treeData[vertexCounter] = vec4(x, 0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta) / 5; z = sin(theta) / 5;
		treeData[vertexCounter] = vec4(x, 1.0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;
		treeData[vertexCounter] = vec4(x, 1.0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*i; x = cos(theta) / 5; z = sin(theta) / 5;
		treeData[vertexCounter] = vec4(x, 0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta) / 5; z = sin(theta) / 5;
		treeData[vertexCounter] = vec4(x, 0, z, 1.0); treeColor[vertexCounter] = treeTrunk;
		vertexCounter++;
	}

	//bottomPyramid
			//side 1
	treeData[vertexCounter] = vec4(-1.0, 0.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(1.0, 0.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.0, 0.0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 2
	treeData[vertexCounter] = vec4(-1.0, 0.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 0.5, -1.0, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.0, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 3
	treeData[vertexCounter] = vec4(1.0, 0.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 0.5, -1.0, 1.0);  treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.0, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;

	//middle pyramid
			//side 1
	treeData[vertexCounter] = vec4(-1.0, 1.0, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(1.0, 1.0, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.5, 0.0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 2
	treeData[vertexCounter] = vec4(-1.0, 1.0, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 1.0, -1.0, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.5, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 3
	treeData[vertexCounter] = vec4(1.0, 1.0, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 1.0, -1.0, 1.0);  treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 2.5, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;

	//top pyramid
			//side 1
	treeData[vertexCounter] = vec4(-1.0, 1.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(1.0, 1.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 3.0, 0.0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 2
	treeData[vertexCounter] = vec4(-1.0, 1.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 1.5, -1.0, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 3.0, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
			//side 3
	treeData[vertexCounter] = vec4(1.0, 1.5, 0.5, 1.0); treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 1.5, -1.0, 1.0);  treeColor[vertexCounter] = treeLeaves;
	vertexCounter++;
	treeData[vertexCounter] = vec4(0.0, 3.0, 0, 1.0); treeColor[vertexCounter] = treeHighlight;
	vertexCounter++;
}

/////////////////////////////////////
///Where the objects are scaled and rotated
/////////////////////////////////////
void initNodes(void)
{
	mat4  m;

	m = mat4(1.0);
	nodes[house1] = Node(m, drawHouse1, &nodes[ground], &nodes[house2]);

	m = mat4(1.0)*Translate(8.0, 0.0, 2.0); 
	nodes[house2] = Node(m, drawHouse1, &nodes[house3], NULL);

	m = mat4(1.0)*Scale(2.0, 2.0, 2.0)*Translate(1.5, 0.0, -5.5); 
	nodes[house3] = Node(m, drawHouse1, NULL, NULL);

	m = mat4(1.0);
	nodes[ground] = Node(m, drawGround, &nodes[house4], NULL);

	m = mat4(1.0)*Scale(1.5,1.5,1.5)*Translate(7.0,0.0,-5.0);
	nodes[house4] = Node(m, drawHouse2, &nodes[tree], NULL);

	m = mat4(1.0)*Translate(0.0,0.0,-6.0);
	nodes[tree] = Node(m, drawTree, &nodes[mountain],NULL);

	m = mat4(1.0)*Translate(20.0, 0.0, -40.0);
	nodes[mountain] = Node(m, drawMountain, &nodes[road], &nodes[mountain2]);

	m = mat4(1.0)*Translate(10.0, 0.0, -4.0)*Scale(1.5,0.7,1.0)*RotateY(90);
	nodes[mountain2] = Node(m, drawMountain, NULL, NULL);

	m = mat4(1.0);
	nodes[road] = Node(m, drawRoad, NULL, NULL);
}

void init() {

	// Instanced Arrays used to position each 
	// tree instance 
	srand((unsigned)time(0));
	int counter = 0;
	double prev;
	double current;

	for (int i = 0; i < treeGrid; i++) {
		for (int j = 0; j < treeGrid; j++) {
			counter++;

			//this is that forest randomly placed in a small area to the lower left of the town
			if (counter > 60)
			{
				shiftATTR[i*treeGrid + j] = vec4(((float)rand() / RAND_MAX)*15 +15, 0, ((float)rand() / RAND_MAX)*15 +15, 0);
			}
			//drawing line by line 
			//trees that surround the little town
			else if (counter > 58)
				shiftATTR[i*treeGrid + j] = vec4(2.0*i - 4.0, 0, -2.0*j + 118, 0);
			else if (counter > 53)
				shiftATTR[i*treeGrid + j] = vec4(2.0*i - 2.0, 0, -2.0*j + 108, 0);
			else if (counter > 52)
				shiftATTR[i*treeGrid + j] = vec4(-2.0*j + 100.0, 0, 2.0*i + 6.0, 0);
			else if (counter > 50)
				shiftATTR[i*treeGrid + j] = vec4(2.0*i -2.0, 0, -2.0*j + 108, 0);
			else if (counter > 49)
				shiftATTR[i*treeGrid + j] = vec4(-2.0*j + 96.0, 0, 2.0*i + 10.0, 0);
			else if (counter > 48)
				shiftATTR[i*treeGrid + j] = vec4(2.0*i , 0, -2.0*j + 106, 0);
			else if (counter > 47)
			{
				shiftATTR[i*treeGrid + j] = vec4(-2.0*j + 94.0, 0, 2.0*i + 12.0, 0);
			}
			else if (counter > 46) {
				shiftATTR[i*treeGrid + j] = vec4(2.0*i +2.0, 0, -2.0*j +104.0, 0);
			}
			else if (counter > 40) {
				shiftATTR[i*treeGrid + j] = vec4(-2.0*j + 92.0, 0, 2.0*i + 14.0, 0);
			}
			else if (counter > 36)
			{
				shiftATTR[i*treeGrid + j] = vec4(2.0*i +14.0, 0, 2.0*j - 64.0, 0);
			}
			else if (counter > 27)
			{
				shiftATTR[i*treeGrid + j] = vec4(-2.0*j + 84.0, 0, 2.0*i +6.0, 0);
			}
			else if (counter > 24)
			{
				shiftATTR[i*treeGrid + j] = vec4(2.0*i + 30.0, 0, 2.0*j - 46.0, 0);
			}
			else if (counter > 17) {
				shiftATTR[i*treeGrid + j] = vec4(2.0*j - 16.0, 0, 2.0*i + 2.0, 0);
			}
			else if (counter > 15) {
				shiftATTR[i*treeGrid + j] = vec4(2.0*i + 16.0, 0, 2.0*j - 30.0, 0);
			}
			else if (counter > 11) {
				shiftATTR[i*treeGrid + j] = vec4(2.0*j - 12.0, 0, 2.0*i -2.0, 0);
			}
			else if (counter > 9)
			{
				shiftATTR[i*treeGrid + j] = vec4(2.0*i+10.0, 0, 2.0*j -22.0 , 0);
			}
			else if (counter > 4) {
				shiftATTR[i*treeGrid + j] = vec4(2.0*j - 6.0, 0, 2.0*i- 6.0, 0);
			}
			else
			{
				shiftATTR[i*treeGrid + j] = vec4(2.0*i, 0, -2.0*j, 0);
			}
		}
	}

	//----------------------------------------------------------------------
	// initializes the build functions
	//----------------------------------------------------------------------

	buildHouse1();
	buildHouse2();
	buildGround();
	buildTree();
	buildMountain();
	buildRoad();

	//----------------------------------------------------------------------
	// initializing the buffers
	//----------------------------------------------------------------------
	glGenBuffers(13, buffers);
	glGenVertexArrays(6, vao);

	//house1	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(house1Data), house1Data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

	//ground
	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundData), groundData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundColor), groundColor, GL_STATIC_DRAW);

	//house2	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(house2Data), house2Data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(house2Color), house2Color, GL_STATIC_DRAW);

	//tree
	glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(treeData), treeData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(treeColor), treeColor, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[12]); //shift attribute for instancing
	glBufferData(GL_ARRAY_BUFFER, sizeof(shiftATTR), shiftATTR, GL_STATIC_DRAW);

	//mounatain
	glBindBuffer(GL_ARRAY_BUFFER, buffers[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mountainData), mountainData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mountainColor), mountainColor, GL_STATIC_DRAW);

	//road and water
	glBindBuffer(GL_ARRAY_BUFFER, buffers[10]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(roadData), roadData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[11]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(roadColor), roadColor, GL_STATIC_DRAW);


	// Load shaders and use the resulting shader programs
	program[house1] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");
	program[house4] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl"); 
	program[ground] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");
	program[tree] = InitShader("vshader_Inst.glsl", "fshaderStock.glsl");
	program[mountain] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");
	program[road] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");

	//house1
	glBindVertexArray(vao[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	vPosition = glGetAttribLocation(program[house1], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	vColor = glGetAttribLocation(program[house1], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	//house2
	glBindVertexArray(vao[2]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	vPosition = glGetAttribLocation(program[house4], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	vColor = glGetAttribLocation(program[house4], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	//ground
	glBindVertexArray(vao[1]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	vPosition = glGetAttribLocation(program[ground], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	vColor = glGetAttribLocation(program[ground], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	//tree
	glBindVertexArray(vao[3]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
	vPosition = glGetAttribLocation(program[tree], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[7]);
	vColor = glGetAttribLocation(program[tree], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

			//instancing for the tree
	glBindBuffer(GL_ARRAY_BUFFER, buffers[12]);
	vShift = glGetAttribLocation(program[tree], "vShift");
	glEnableVertexAttribArray(vShift);
	glVertexAttribPointer(vShift, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glVertexAttribDivisor(vShift, 1);

	glBindVertexArray(0);

	//mounatain
	glBindVertexArray(vao[4]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[8]);
	vPosition = glGetAttribLocation(program[mountain], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffers[9]);
	vColor = glGetAttribLocation(program[mountain], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	//road
	glBindVertexArray(vao[5]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[10]);
	vPosition = glGetAttribLocation(program[road], "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffers[11]);
	vColor = glGetAttribLocation(program[road], "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);


	// Initialize tree
	initNodes();

	glClearColor(0.118, 0.565, 1.000, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT, GL_FILL);

}

//-------------------------------------------------------------------------------
// 
//		Draw functions
//
//-------------------------------------------------------------------------------

void drawHouse1() {

	glUseProgram(program[house1]);

	proj_loc = glGetUniformLocation(program[house1], "Projection");
	model_view_loc = glGetUniformLocation(program[house1], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, house1points);

	glUseProgram(0);
	glBindVertexArray(0);

}

void drawHouse2() {

	glUseProgram(program[house4]);

	proj_loc = glGetUniformLocation(program[house4], "Projection");
	model_view_loc = glGetUniformLocation(program[house4], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, house2points);

	glUseProgram(0);
	glBindVertexArray(0);

}

void drawGround() {

	glUseProgram(program[ground]);

	proj_loc = glGetUniformLocation(program[ground], "Projection");
	model_view_loc = glGetUniformLocation(program[ground], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, groundpoints);

	glUseProgram(0);
	glBindVertexArray(0);

}

void drawTree() {

	glUseProgram(program[tree]);

	proj_loc = glGetUniformLocation(program[tree], "Projection");
	model_view_loc = glGetUniformLocation(program[tree], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[3]);
	glDrawArrays(GL_TRIANGLES, 0, treepoints);
	//glDrawArraysInstanced(GL_TRIANGLES, 0, treepoints, treeGrid);

	glUseProgram(0);
	glBindVertexArray(0);

}

void drawMountain() {

	glUseProgram(program[mountain]);

	proj_loc = glGetUniformLocation(program[mountain], "Projection");
	model_view_loc = glGetUniformLocation(program[mountain], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[4]);
	glDrawArrays(GL_TRIANGLES, 0, mountainpoints);

	glUseProgram(0);
	glBindVertexArray(0);

}

void drawRoad() {

	glUseProgram(program[road]);

	proj_loc = glGetUniformLocation(program[road], "Projection");
	model_view_loc = glGetUniformLocation(program[road], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[5]);
	glDrawArrays(GL_TRIANGLES, 0, roadpoints);

	glUseProgram(0);
	glBindVertexArray(0);

}

//----------------------------------------------------------------------
// display function
//----------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projmat = Perspective(90, 1.0, 0.1, 10.0);

	model_view = LookAt(eye, eye - n, v);
	model_view = model_view*RotateX(theta);
	model_view = model_view*RotateY(thetaY);

	traverse(&nodes[house1]);

	glutSwapBuffers();
}


void idle(void)
{
	glutPostRedisplay();
}

//////////////////////////////
//Mouse Functions
//////////////////////////////
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

//----------------------------------------------------------------
//
//			Keyboard Functions
//
//-----------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	GLfloat xt, yt, zt;
	GLfloat cosine, sine;

	//prees  spacebar to  return to default (initial) camera position and oreintation 
	if (key == 32)
	{
		v = vec4(0.0, 1.0, 0.0, 0.0);
		u = vec4(1.0, 0.0, 0.0, 0.0);
		n = vec4(0.0, 0.0, 1.0, 1.0);
		eye = vec4(5.0, 1.0, 25.0, 1.0);
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
		u[Y] = yt*cosine - n[Y] * sine;
		u[Z] = zt*cosine - n[Z] * sine;
		n[X] = xt*sine + n[X] * cosine;
		n[Y] = yt*sine + n[Y] * cosine;
		n[Z] = zt*sine + n[Z] * cosine;
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
	}

	glutPostRedisplay();
}

void specKey(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP: // MOVE FORWARD
		eye[0] -= SCALE_VECTOR * n[0];
		eye[1] -= SCALE_VECTOR * n[1];
		eye[2] -= SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_DOWN: // MOVE BACKWARD
		eye[0] += SCALE_VECTOR * n[0];
		eye[1] += SCALE_VECTOR * n[1];
		eye[2] += SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_LEFT: // MOVE LEFT
		eye[0] -= SCALE_VECTOR * u[0];
		eye[1] -= SCALE_VECTOR * u[1];
		eye[2] -= SCALE_VECTOR * u[2];
		break;
	case GLUT_KEY_RIGHT: // MOVE RIGHT
		eye[0] += SCALE_VECTOR * u[0];
		eye[1] += SCALE_VECTOR * u[1];
		eye[2] += SCALE_VECTOR * u[2];
		break;
	default:
		break;
	}
	glutPostRedisplay();
}


void reshape(int width, int height)
//the same objects are shown (possibly scaled) w/o shape distortion 
//original viewport is a square
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Homework3: New Bark Town with a Mountain");

	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specKey);
	glutIdleFunc(idle);

	glewInit();

	init();

	glutMainLoop();
	return 0;
}
