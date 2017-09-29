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
	q/Q: hides the current window being viewed
	ESC: closes all windows

Objects: Brick Cylinder
	Referenced from the cannon assignment and modified to have the brick texture on it.

Objects: gold sphere
	Taken from the shaded sphere provided from instructor. Normals were modified for 
	smoother shading. Modified to have gold texture on it.

Objects: copper ellipse
	Scaled sphere to look like ellipse. Modified to have copper texture on it. 

Light: Single distant light, moving light, and stationary light

Windows:
	Pressing the left mouse button opens a window for just the cylinder.
	Pressing the right mouse button opens a window for just the sphere.
	Pressing the middle mouse button opens a window for just the ellipse.

 
****************************************************************/

#include "Angel.h"
#include "glm.h"
#include <assert.h>

///////CONSTANT/////////////////////
#ifndef PI
#define PI 3.14159265358979323846
#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0
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

int currentHeight = 700, currentWidth = 800;

//sphere constants
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices = 3 * NumTriangles;

GLuint buffers[2];
int  TextureWidth[3];
int  TextureHeight[3];
GLuint textures[3];
vec2 cylinder_tex_coords[600];
vec2 sphere_tex_coords[NumVertices];

GLubyte* image0;
GLubyte* image1;
GLubyte* image2;
//create a vertex array object
GLuint vao[2];
GLuint program[2];
//Spinning light: orbit parameterization
float spin = 0.0, spin_step = 0.001;

//--------------------------------------
//   Program and Vao Objects 
//--------------------------------------

enum {
	cylinderNum = 0,
	sphereNum = 1,
	nodesnum
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

#include<iostream>
using namespace std;

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

vec4 n = normalize(vec4(0.0, 0.0, 1.0, 0.0));
vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
vec4 eye = vec4(0.0, 0.0, 2.0, 1.0);
vec4 nref = n, vref = v, uref = u;

///////LIGHTING and SHADING////////////////////////


//---------------------------------------------------------
//	lightData and materialData structs
//			Used so that there aren't giant list of 
//			the same variable for different objects
//---------------------------------------------------------
typedef struct lightData {
	//fixed distant light
	point4 light_position2;
	color4 light_ambient2;
	color4 light_diffuse2;
	color4 light_specular2;

	color4 ambient_product2;
	color4 diffuse_product2;
	color4 specular_product2;

}lightData;

typedef struct materialData {
	//materials
	color4 material_ambient;
	color4 material_diffuse;
	color4 material_specular;
	GLfloat material_shininess;

}materialData;

//---------------------------------------------------------
//	Lists for the different objects
//---------------------------------------------------------
materialData cylinderMaterial, sphereMaterial, ellipseMaterial;		//material of each object
lightData cylinderLightData, sphereLightData, ellipseLightData;		//stationary light data
lightData cylinderSpinData, sphereSpinData, ellipseSpinData;			//spinning light data
lightData cylinderCameraData, sphereCameraData, ellipseCameraData;		//camera light data


//---------------------------------------------------------
//	compute functions
//			compute the variables for the shaders
//---------------------------------------------------------

void computeStationaryLight(lightData theData, materialData material, int num) {

	theData.ambient_product2 = theData.light_ambient2 * material.material_ambient;
	theData.diffuse_product2 = theData.light_diffuse2 * material.material_diffuse;
	theData.specular_product2 = theData.light_specular2 * material.material_specular;

	glUniform4fv(glGetUniformLocation(program[num], "AmbientProduct2"), 1, theData.ambient_product2);
	glUniform4fv(glGetUniformLocation(program[num], "DiffuseProduct2"), 1, theData.diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[num], "SpecularProduct2"), 1, theData.specular_product2);

}

void computeSpinningLight(lightData theData, materialData material, int num) {

	theData.ambient_product2 = theData.light_ambient2 * material.material_ambient;
	theData.diffuse_product2 = theData.light_diffuse2 * material.material_diffuse;
	theData.specular_product2 = theData.light_specular2 * material.material_specular;

	glUniform4fv(glGetUniformLocation(program[num], "AmbientProduct3"), 1, theData.ambient_product2);
	glUniform4fv(glGetUniformLocation(program[num], "DiffuseProduct3"), 1, theData.diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[num], "SpecularProduct3"), 1, theData.specular_product2);

}

void computeCameraLight(lightData theData, materialData material, int num) {

	theData.ambient_product2 = theData.light_ambient2 * material.material_ambient;
	theData.diffuse_product2 = theData.light_diffuse2 * material.material_diffuse;
	theData.specular_product2 = theData.light_specular2 * material.material_specular;

	glUniform4fv(glGetUniformLocation(program[num], "AmbientProduct4"), 1, theData.ambient_product2);
	glUniform4fv(glGetUniformLocation(program[num], "DiffuseProduct4"), 1, theData.diffuse_product2);
	glUniform4fv(glGetUniformLocation(program[num], "SpecularProduct4"), 1, theData.specular_product2);

}

//---------------------------------------------------------
//	initialize functions
//			initialize the variables and the colors of the
//			objects
//---------------------------------------------------------
	//stationary light functions
void stationaryLightCylinder() {
	cylinderLightData.light_position2 = vec4(1.0, 1.0, -0.5, 0.0);
	cylinderLightData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	cylinderLightData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	cylinderLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[cylinderNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, cylinderLightData.light_position2);
}

void stationaryLightSphere() {
	sphereLightData.light_position2 = vec4(1.0, 1.0, 0.5, 0.0);
	sphereLightData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	sphereLightData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	sphereLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, sphereLightData.light_position2);
}

void stationaryLightEllipse() {
	ellipseLightData.light_position2 = vec4(1.0, 1.0, 0.5, 0.0);
	ellipseLightData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	ellipseLightData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	ellipseLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, ellipseLightData.light_position2);
}

	//spinning light functions
void spinningLightCylinder() {
	cylinderSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	cylinderSpinData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	cylinderSpinData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	cylinderSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[cylinderNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, cylinderSpinData.light_position2);
}

void spinningLightSphere() {
	sphereSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	sphereSpinData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	sphereSpinData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	sphereSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, sphereSpinData.light_position2);
}

void spinningLightEllipse() {
	ellipseSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	ellipseSpinData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	ellipseSpinData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	ellipseSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, ellipseSpinData.light_position2);
}

	//camera light functions
void cameraLightCylinder() {
	cylinderCameraData.light_position2 = eye;
	cylinderCameraData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	cylinderCameraData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	cylinderCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[cylinderNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, cylinderCameraData.light_position2);
}

void cameraLightSphere() {
	sphereCameraData.light_position2 = eye ;
	sphereCameraData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	sphereCameraData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	sphereCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, sphereCameraData.light_position2);
}

void cameraLightEllipse() {
	ellipseCameraData.light_position2 = eye;
	ellipseCameraData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	ellipseCameraData.light_diffuse2 = vec4(0.8, 0.8, 0.8, 1.0);
	ellipseCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, ellipseCameraData.light_position2);
}



//---------------------------------------------------------
//	Building the Cylinder
//---------------------------------------------------------

vec4 cylinderData[600];
vec3 cylinderNormal[600];
int iMax = 50, vertexCounter = 0;
GLfloat x, z, thetaC;

void buildCylinder() {
	//top
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(0, 1.0, 0, 1.0); 
		cylinderNormal[vertexCounter] = vec3(0, 1.0, 0); 
		cylinder_tex_coords[vertexCounter] = vec2(0, 0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 1.0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 1.0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, z);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 1.0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 1.0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, z);
		vertexCounter++;
	}
	//bottom
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(0, 0, 0, 1.0); 
		cylinderNormal[vertexCounter] = vec3(0, 0, 0); 
		cylinder_tex_coords[vertexCounter] = vec2(0, 0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, z);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, z);
		vertexCounter++;
	}
	//cylinder
	for (int i = 0; i<iMax; i++) {
		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC);
		cylinderData[vertexCounter] = vec4(x, 1.0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 1.0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC) ; z = sin(thetaC) ;
		cylinderData[vertexCounter] = vec4(x, 1.0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 1.0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		cylinderData[vertexCounter] = vec4(x, 1.0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 1.0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 1.0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*i; x = cos(thetaC); z = sin(thetaC) ;
		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;

		thetaC = (2 * PI / (float)iMax)*(i + 1.0); x = cos(thetaC) ; z = sin(thetaC) ;
		cylinderData[vertexCounter] = vec4(x, 0, z, 1.0); 
		cylinderNormal[vertexCounter] = vec3(x, 0, z); 
		cylinder_tex_coords[vertexCounter] = vec2(x, 0);
		vertexCounter++;
	}
}


//---------------------------------------------------------
//	Building the Sphere
//---------------------------------------------------------
/*const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices = 3 * NumTriangles;*/

point4 points[NumVertices];
vec3   normals[NumVertices];

int Index = 0;

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
void drawCylinder()
	{
		stack.push(model_view);
		normalStack.push(normal_view);

		glUseProgram(program[cylinderNum]);
		glBindVertexArray(vao[cylinderNum]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		if (!GLEW_ARB_vertex_array_object)
			std::cerr << "ARB_vertex_array_object not available." << std::endl;

		// Retrieve transformation uniform variable locations
		cylinderMaterial.material_ambient = vec4(0.1, 0.1, 0.1, 1.0);
		cylinderMaterial.material_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
		cylinderMaterial.material_specular = vec4(1.0, 1.0, 1.0, 1.0);
		cylinderMaterial.material_shininess = 100.0;

		stationaryLightCylinder();
		computeStationaryLight(cylinderLightData, cylinderMaterial, cylinderNum);

		spinningLightCylinder();
		computeSpinningLight(cylinderSpinData, cylinderMaterial, cylinderNum);

		cameraLightCylinder();
		computeCameraLight(cylinderCameraData, cylinderMaterial, cylinderNum);

		ModelView = glGetUniformLocation(program[0], "ModelView");
		NormalView = glGetUniformLocation(program[0], "NormalView");
		Projection = glGetUniformLocation(program[0], "Projection");
		camera_loc = glGetUniformLocation(program[0], "Camera");
		glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
		glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
		glUniform1f(glGetUniformLocation(program[0], "Shininess"), cylinderMaterial.material_shininess);
		glDrawArrays(GL_TRIANGLES, 0, 600);

		model_view = stack.pop();
		normal_view = normalStack.pop();

		glutPostRedisplay();
	}

void drawSphere()
{
	stack.push(model_view);
	normalStack.push(normal_view);

	glUseProgram(program[1]);
	glBindVertexArray(vao[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	if (!GLEW_ARB_vertex_array_object)
		std::cerr << "ARB_vertex_array_object not available." << std::endl;

	// Retrieve transformation uniform variable locations
	sphereMaterial.material_ambient = vec4(0.2, 0.2, 0.2, 1.0);
	sphereMaterial.material_diffuse = vec4(0.6, 0.6, 0.6, 1.0);
	sphereMaterial.material_specular = vec4(0.8, 0.8, 0.8, 1.0);
	sphereMaterial.material_shininess = 20.0;

	stationaryLightSphere();
	computeStationaryLight(sphereLightData, sphereMaterial, sphereNum);

	spinningLightSphere();
	computeSpinningLight(sphereSpinData, sphereMaterial, sphereNum);

	cameraLightSphere();
	computeCameraLight(sphereCameraData, sphereMaterial, sphereNum);

	ModelView = glGetUniformLocation(program[sphereNum], "ModelView");
	NormalView = glGetUniformLocation(program[sphereNum], "NormalView");
	Projection = glGetUniformLocation(program[sphereNum], "Projection");
	camera_loc = glGetUniformLocation(program[sphereNum], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[1], "Shininess"), sphereMaterial.material_shininess);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = stack.pop();
	normal_view = normalStack.pop();

	glutPostRedisplay();

}

void drawEllipse()
{
	stack.push(model_view);
	normalStack.push(normal_view);

	glUseProgram(program[sphereNum]);
	glBindVertexArray(vao[sphereNum]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	if (!GLEW_ARB_vertex_array_object)
		std::cerr << "ARB_vertex_array_object not available." << std::endl;

	// Retrieve transformation uniform variable locations

	ellipseMaterial.material_ambient = vec4(0.2, 0.2, 0.2, 1.0);
	ellipseMaterial.material_diffuse = vec4(0.6, 0.6, 0.6, 1.0);
	ellipseMaterial.material_specular = vec4(0.8, 0.8, 0.8, 1.0);
	ellipseMaterial.material_shininess = 20.0;

	stationaryLightEllipse();
	computeStationaryLight(ellipseLightData, ellipseMaterial, sphereNum);

	spinningLightEllipse();
	computeSpinningLight(ellipseSpinData,ellipseMaterial, sphereNum);

	cameraLightEllipse();
	computeCameraLight(ellipseCameraData, ellipseMaterial, sphereNum);

	ModelView = glGetUniformLocation(program[sphereNum], "ModelView");
	NormalView = glGetUniformLocation(program[sphereNum], "NormalView");
	Projection = glGetUniformLocation(program[sphereNum], "Projection");
	camera_loc = glGetUniformLocation(program[sphereNum], "Camera");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
	glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
	glUniform1f(glGetUniformLocation(program[sphereNum], "Shininess"), ellipseMaterial.material_shininess);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = stack.pop();
	normal_view = normalStack.pop();

	glutPostRedisplay();

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
		cylinder = 0,
		sphere = 1,
		ellipse = 2,
		NumNodes
	};
	Node  nodes[NumNodes];

	//---------------------------------------------------------
	//	Tree Scene
	//---------------------------------------------------------
	void initScene() {
		mat4  m;

		m = mat4(1.0)*Translate(1.2, -1.0, 0.0)*Scale(0.4,1.0, 0.4);// *RotateZ(10)*RotateX(20)*RotateY(90);
		nodes[cylinder] = Node(m, drawCylinder, &nodes[sphere], NULL);

		m = mat4(1.0)*Scale(0.5,0.5,0.5);
		nodes[sphere] = Node(m, drawSphere, NULL, &nodes[ellipse]);

		m = mat4(1.0)*Scale(0.45, 0.8, 0.45)*Translate(-3.0, -1.0, 2.0);
		nodes[ellipse] = Node(m, drawEllipse, NULL, NULL);
	}

	void initScene2() {
		mat4  m;

		m = mat4(1.0)*Translate(0.0,-0.5,0.0)*Scale(0.4, 1.0, 0.4);// *RotateZ(10)*RotateX(20)*RotateY(90);
		Node justCylinder = Node(m, drawCylinder, NULL, NULL);
	}
	
	void buildInit() {
		//----------------------------------------------------------------------
		// initializes the build functions
		//----------------------------------------------------------------------
		buildCylinder();
		tetrahedron(NumTimesToSubdivide);
	}

	void init(void) {

		image0 = glmReadPPM("copper_texture256by256.ppm", &TextureWidth[0], &TextureHeight[0]);
		image1 = glmReadPPM("brks.ppm", &TextureWidth[1], &TextureHeight[1]);
		image2 = glmReadPPM("gold.ppm", &TextureWidth[2], &TextureHeight[2]);

		// Initialize texture objects
		glGenTextures(3, textures);

		glActiveTexture(GL_TEXTURE0); 

		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[0], TextureHeight[0], 0,
			GL_RGB, GL_UNSIGNED_BYTE, image0);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[1], TextureHeight[1], 0,
			GL_RGB, GL_UNSIGNED_BYTE, image1);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[2], TextureHeight[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, image2);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//----------------------------------------------------------------------
		// initializing the buffers
		//----------------------------------------------------------------------
		glGenVertexArrays(2, vao);
		glGenBuffers(2, buffers);

		//cylinder
		glBindVertexArray(vao[cylinderNum]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[cylinderNum]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderData) + sizeof(cylinderNormal) + sizeof(cylinder_tex_coords), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cylinderData), cylinderData);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(cylinderData), sizeof(cylinderNormal), cylinderNormal);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(cylinderData) + sizeof(cylinderNormal), sizeof(cylinder_tex_coords), cylinder_tex_coords);

		program[cylinderNum] = InitShader("vLIGHTS_v120.glsl", "fLIGHTS_v120.glsl");
		glUseProgram(program[cylinderNum]);


		vPositionCylinder = glGetAttribLocation(program[cylinderNum], "vPosition");
		glEnableVertexAttribArray(vPositionCylinder);
		glVertexAttribPointer(vPositionCylinder, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		vNormalCylinder = glGetAttribLocation(program[cylinderNum], "vNormal");
		glEnableVertexAttribArray(vNormalCylinder);
		glVertexAttribPointer(vNormalCylinder, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderData)));

		GLuint vTexCoordCyl = glGetAttribLocation(program[cylinderNum], "vTexCoord");
		glEnableVertexAttribArray(vTexCoordCyl);
		glVertexAttribPointer(vTexCoordCyl, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(cylinderData) + sizeof(cylinderNormal)));

		glUniform1i(glGetUniformLocation(program[cylinderNum], "texture"), 0);

		glBindVertexArray(0);
		glUseProgram(0);

		//sphere
		glBindBuffer(GL_ARRAY_BUFFER, buffers[sphereNum]);
		glBindVertexArray(vao[sphereNum]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(sphere_tex_coords), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(sphere_tex_coords), sphere_tex_coords);

		program[sphereNum] = InitShader("vLIGHTS_v120.glsl", "fLIGHTS_v120.glsl");
		glUseProgram(program[sphereNum]);

		vPositionSphere = glGetAttribLocation(program[sphereNum], "vPosition");
		glEnableVertexAttribArray(vPositionSphere);
		glVertexAttribPointer(vPositionSphere, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		vNormalSphere = glGetAttribLocation(program[sphereNum], "vNormal");
		glEnableVertexAttribArray(vNormalSphere);
		glVertexAttribPointer(vNormalSphere, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

		vTexCoord = glGetAttribLocation(program[sphereNum], "vTexCoord");
		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(points)+sizeof(normals)));

		glUniform1i(glGetUniformLocation(program[sphereNum], "texture"), 0);

		glBindVertexArray(0);
		glUseProgram(0);

		initScene();

		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.5, 0.5, 0.5, 1.0);
	}

	//---------------------------------------------------------
	//		Display Function
	//---------------------------------------------------------
	void display(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		projection = Perspective(90, 1.0, 0.1, 10.0);

		model_view = LookAt(eye, eye - n, v);
		model_view = model_view*RotateX(theta);
		model_view = model_view*RotateY(thetaY);

		normal_view = mat4(1.0);

		traverse(&nodes[cylinder]);

		glutSwapBuffers();
	}

	void display2(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		projection = Perspective(90, 1.0, 0.1, 10.0);

		model_view = LookAt(eye, eye - n, v);
		model_view = model_view*RotateX(theta);
		model_view = model_view*RotateY(thetaY);

		normal_view = mat4(1.0);

		mat4  m;
		m = mat4(1.0)*Translate(0.0, -0.5, 0.0)*Scale(0.4, 1.0, 0.4);// *RotateZ(10)*RotateX(20)*RotateY(90);
		Node justCylinder = Node(m, drawCylinder, NULL, NULL);
		traverse(&justCylinder);

		glutSwapBuffers();
	}

	void display3(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		projection = Perspective(90, 1.0, 0.1, 10.0);

		model_view = LookAt(eye, eye - n, v);
		model_view = model_view*RotateX(theta);
		model_view = model_view*RotateY(thetaY);

		normal_view = mat4(1.0);

		mat4  m;

		m = mat4(1.0)*Scale(0.5, 0.5, 0.5);
		Node justSphere = Node(m, drawSphere, NULL, NULL);

		traverse(&justSphere);

		glutSwapBuffers();
	}

	void display4(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		projection = Perspective(90, 1.0, 0.1, 10.0);

		model_view = LookAt(eye, eye - n, v);
		model_view = model_view*RotateX(theta);
		model_view = model_view*RotateY(thetaY);

		normal_view = mat4(1.0);

		mat4  m;

		m = mat4(1.0)*Scale(0.5, 0.5, 0.5);
		nodes[sphere] = Node(m, drawSphere, NULL, &nodes[ellipse]);

		m = mat4(1.0)*Scale(0.45, 0.8, 0.45);
		Node justEllipse = Node(m, drawEllipse, NULL, NULL);

		traverse(&justEllipse);

		glutSwapBuffers();
	}

	//---------------------------------------------------------
	//		Main Function
	//---------------------------------------------------------
	void constants() {
		glutMouseFunc(mouse);

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
			u[Y] = yt*cosine - n[Y] * sine;
			u[Z] = zt*cosine - n[Z] * sine;
			n[X] = xt*sine + n[X] * cosine;
			n[Y] = yt*sine + n[Y] * cosine;
			n[Z] = zt*sine + n[Z] * cosine;
			break;
		case 033:  // Escape key
			exit(EXIT_SUCCESS);
		case 'q': case 'Q':
			glutHideWindow();
		}

		glutPostRedisplay();
	}

	void keyboard2(unsigned char key, int x, int y)
	{
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
			u[Y] = yt*cosine - n[Y] * sine;
			u[Z] = zt*cosine - n[Z] * sine;
			n[X] = xt*sine + n[X] * cosine;
			n[Y] = yt*sine + n[Y] * cosine;
			n[Z] = zt*sine + n[Z] * cosine;
			break;
		case 033:  // Escape key
			exit(EXIT_SUCCESS);
		case 'q': case 'Q':
			glutHideWindow();
		}

		glutPostRedisplay();
	}

	void keyboard3(unsigned char key, int x, int y)
	{
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
			u[Y] = yt*cosine - n[Y] * sine;
			u[Z] = zt*cosine - n[Z] * sine;
			n[X] = xt*sine + n[X] * cosine;
			n[Y] = yt*sine + n[Y] * cosine;
			n[Z] = zt*sine + n[Z] * cosine;
			break;
		case 033:  // Escape key
			exit(EXIT_SUCCESS);
		case 'q': case 'Q':
			glutHideWindow();
		}

		glutPostRedisplay();
	}

	void keyboard4(unsigned char key, int x, int y)
	{
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
			u[Y] = yt*cosine - n[Y] * sine;
			u[Z] = zt*cosine - n[Z] * sine;
			n[X] = xt*sine + n[X] * cosine;
			n[Y] = yt*sine + n[Y] * cosine;
			n[Z] = zt*sine + n[Z] * cosine;
			break;
		case 033:  // Escape key
			exit(EXIT_SUCCESS);
		case 'q': case 'Q':
			glutHideWindow();
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

	Angel::vec4 rayOriginWorldCoords;
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
	}


	