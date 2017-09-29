/*************************************************************
CG: Shaded Scene
Shanan Almario
CSCE385

This code uses the phongTorus code provided by instructor as a base reference.

Camera: Free flying  
	Pilot's interface pitch/roll/yaw, slide forward/backward and strafe left/right

Objects: torus
	Surface of revolution obtained by rotating the circle (x-1)^2 + (z-0)^2 = 0.121, y=0 around the z-axis
	i.e the crossection profile is circle of radios 0.1
	The paraemtric representation of this torus is
			x = (1+0.1*cos(theta))*cos(phi)
			y = (1+0.1*cos(theta))*sin(phi)
			z=  0.1*sin(theta)
	where theta and phi belong to [0, 2Pi].
	
	The formulae in the instructor's nodes say that the normal at a vertex with coordinates
			x = (1+0.1*cos(theta))*cos(phi)
			y = (1+0.1*cos(theta))*sin(phi)
			z=  0.1*sin(theta)
	is the vector N = vec3{cos(theta)*cos(phi), cos(theta)*sin(phi), sin(theta)}


Objects: sphere
	Taken from the shaded sphere provided from instructor. Normals were modified for 
	smoother shading.

Objects: ellipse
	Scaled sphere to look like ellipse

Light: Single distant light, moving light, and stationary light

 
****************************************************************/

#include "Angel.h"
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

int currentHeight = 800, currentWidth = 800;

GLuint buffers[13];
//create a vertex array object
GLuint vao[2];
GLuint program[2];
//Spinning light: orbit parameterization
float spin = 0.0, spin_step = 0.001;

//--------------------------------------
//   Program and Vao Objects 
//--------------------------------------

enum {
	torusNum = 0,
	sphereNum = 1,
	ellipseNum = 2,
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


/////ATTRIBUTES, BUFFERS, ARRAYS///////////////////
GLuint vPositionTorus, vNormalTorus, vPositionSphere, vNormalSphere, vPositionEllipsoid, vNormalEllipsoid;

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
materialData torusMaterial, sphereMaterial, ellipseMaterial;		//material of each object
lightData torusLightData, sphereLightData, ellipseLightData;		//stationary light data
lightData torusSpinData, sphereSpinData, ellipseSpinData;			//spinning light data
lightData torusCameraData, sphereCameraData, ellipseCameraData;		//camera light data


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
void stationaryLightTorus() {
	torusLightData.light_position2 = vec4(1.0, 1.0, -0.5, 0.0);
	torusLightData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	torusLightData.light_diffuse2 = vec4(0.66, 0.66, 0.66, 1.0);
	torusLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[torusNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, torusLightData.light_position2);
}

void stationaryLightSphere() {
	sphereLightData.light_position2 = vec4(1.0, 1.0, 0.5, 0.0);
	sphereLightData.light_ambient2 = vec4(0.85, 0.65, 0.12, 1.0);
	sphereLightData.light_diffuse2 = vec4(0.6, 0.4, 0.08, 1.0);
	sphereLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, sphereLightData.light_position2);
}

void stationaryLightEllipse() {
	ellipseLightData.light_position2 = vec4(1.0, 1.0, 0.5, 0.0);
	ellipseLightData.light_ambient2 = vec4(0.86, 0.08, 0.24, 1.0);
	ellipseLightData.light_diffuse2 = vec4(0.61, 0.07, 0.12, 1.0);
	ellipseLightData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition");
	glUniform4fv(light_pos_loc, 1, ellipseLightData.light_position2);
}

	//spinning light functions
void spinningLightTorus() {
	torusSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	torusSpinData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	torusSpinData.light_diffuse2 = vec4(0.75, 0.75, 0.75, 1.0);
	torusSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[torusNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, torusSpinData.light_position2);
}

void spinningLightSphere() {
	sphereSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	sphereSpinData.light_ambient2 = vec4(0.85, 0.65, 0.12, 1.0);
	sphereSpinData.light_diffuse2 = vec4(0.6, 0.4, 0.08, 1.0);
	sphereSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, sphereSpinData.light_position2);
}

void spinningLightEllipse() {
	ellipseSpinData.light_position2 = vec4(-3 * sin(spin), 4, -3 * cos(spin), 1.0);
	ellipseSpinData.light_ambient2 = vec4(0.86, 0.08, 0.24, 1.0);
	ellipseSpinData.light_diffuse2 = vec4(0.61, 0.07, 0.12, 1.0);
	ellipseSpinData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition2");
	glUniform4fv(light_pos_loc, 1, ellipseSpinData.light_position2);
}

	//camera light functions
void cameraLightTorus() {
	torusCameraData.light_position2 = eye;
	torusCameraData.light_ambient2 = vec4(0.5, 0.5, 0.5, 1.0);
	torusCameraData.light_diffuse2 = vec4(0.75, 0.75, 0.75, 1.0);
	torusCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[torusNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, torusCameraData.light_position2);
}

void cameraLightSphere() {
	sphereCameraData.light_position2 = eye ;
	sphereCameraData.light_ambient2 = vec4(0.85, 0.65, 0.12, 1.0);
	sphereCameraData.light_diffuse2 = vec4(0.6, 0.4, 0.08, 1.0);
	sphereCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, sphereCameraData.light_position2);
}

void cameraLightEllipse() {
	ellipseCameraData.light_position2 = eye;
	ellipseCameraData.light_ambient2 = vec4(0.86, 0.08, 0.24, 1.0);
	ellipseCameraData.light_diffuse2 = vec4(0.61, 0.07, 0.12, 1.0);
	ellipseCameraData.light_specular2 = vec4(1.0, 1.0, 1.0, 1.0);
	GLuint light_pos_loc = glGetUniformLocation(program[sphereNum], "LightPosition3");
	glUniform4fv(light_pos_loc, 1, ellipseCameraData.light_position2);
}



//---------------------------------------------------------
//	Building the Torus
//---------------------------------------------------------
const int numTorus = 2400;
const int baseTorus = 2400;

vec4 torusData[numTorus];
vec3 torusNormal[numTorus];
const int sizeTorus = sizeof(torusData);
const int sizeTorusNormal = sizeof(torusNormal);

void buildTorus() {
	int numc, numt, i, j, k;
	double s, t, x, y, z, xn, yn, zn;
	numc = 30; numt = 39;
	int vertexCounter = 0;
	for (i = 0; i < numc; i++) {
		for (j = 0; j <= numt; j++) {
			for (k = 1; k >= 0; k--) {
				s = (i + k) % numc + 0.5;
				t = j % numt;
				x = (0.5 + .1*cos(s * 2 * PI / numc))*cos(t * 2 * PI / numt);
				y = (0.5 + .1*cos(s * 2 * PI / numc))*sin(t * 2 * PI / numt);
				z = 0.1 * sin(s * 2 * PI / numc);
				xn = cos(s * 2 * PI / numc)*cos(t * 2 * PI / numt);
				yn = cos(s * 2 * PI / numc)*sin(t * 2 * PI / numt);
				zn = sin(s * 2 * PI / numc);
				torusData[vertexCounter] = vec4(x, y, z, 1.0);
				torusNormal[vertexCounter] = vec3(xn, yn, zn);
				vertexCounter++;
			}
		}
	}
}


//---------------------------------------------------------
//	Building the Sphere
//---------------------------------------------------------
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices = 3 * NumTriangles;

point4 points[NumVertices];
vec3   normals[NumVertices];

int Index = 0;

void
triangle(const point4& a, const point4& b, const point4& c)
{
	normals[Index] = normalize(vec3(a[0], a[1], a[2])); points[Index] = a; Index++;
	normals[Index] = normalize(vec3(b[0], b[1], b[2])); points[Index] = b; Index++;
	normals[Index] = normalize(vec3(c[0], c[1], c[2])); points[Index] = c; Index++;
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
void drawTorus()
	{
		stack.push(model_view);
		normalStack.push(normal_view);

		glUseProgram(program[0]);
		glBindVertexArray(vao[0]);
		// Retrieve transformation uniform variable locations
		torusMaterial.material_ambient = vec4(0.2, 0.2, 0.2, 1.0);
		torusMaterial.material_diffuse = vec4(0.6, 0.6, 0.6, 1.0);
		torusMaterial.material_specular = vec4(0.8, 0.8, 0.8, 1.0);
		torusMaterial.material_shininess = 60.0;

		stationaryLightTorus();
		computeStationaryLight(torusLightData, torusMaterial, torusNum);

		spinningLightTorus();
		computeSpinningLight(torusSpinData, torusMaterial, torusNum);

		cameraLightTorus();
		computeCameraLight(torusCameraData, torusMaterial, torusNum);

		ModelView = glGetUniformLocation(program[0], "ModelView");
		NormalView = glGetUniformLocation(program[0], "NormalView");
		Projection = glGetUniformLocation(program[0], "Projection");
		camera_loc = glGetUniformLocation(program[0], "Camera");
		glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		glUniformMatrix4fv(NormalView, 1, GL_TRUE, normal_view);
		glUniformMatrix4fv(camera_loc, 1, GL_TRUE, model_view);
		glUniform1f(glGetUniformLocation(program[0], "Shininess"), torusMaterial.material_shininess);
		glDrawArrays(GL_QUAD_STRIP, 0, 2400);

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
		torus = 0,
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

		m = mat4(1.0)*Translate(1.0, 0.6, 0.0)*RotateX(5);// *RotateZ(10)*RotateX(20)*RotateY(90);;
		nodes[torus] = Node(m, drawTorus, &nodes[sphere], NULL);

		m = mat4(1.0)*Scale(0.5,0.5,0.5);
		nodes[sphere] = Node(m, drawSphere, NULL, &nodes[ellipse]);

		m = mat4(1.0)*Scale(0.45, 0.8, 0.45)*Translate(-3.0, -1.0, 2.0);
		nodes[ellipse] = Node(m, drawEllipse, NULL, NULL);

	}

	void init(void) {
		//----------------------------------------------------------------------
		// initializes the build functions
		//----------------------------------------------------------------------
		buildTorus();
		tetrahedron(NumTimesToSubdivide); //build sphere

		//----------------------------------------------------------------------
		// initializing the buffers
		//----------------------------------------------------------------------
		glGenVertexArrays(4, vao);
		glGenBuffers(2, buffers);

		//torus
		glBindVertexArray(vao[torusNum]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[torusNum]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(torusData) + sizeof(torusNormal), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(torusData), torusData);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(torusData), sizeof(torusNormal), torusNormal);


		program[torusNum] = InitShader("vLIGHTS_v120.glsl", "fLIGHTS_v120.glsl");
		glUseProgram(program[torusNum]);

		vPositionTorus = glGetAttribLocation(program[torusNum], "vPosition");
		glEnableVertexAttribArray(vPositionTorus);
		glVertexAttribPointer(vPositionTorus, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		vNormalTorus = glGetAttribLocation(program[torusNum], "vNormal");
		glEnableVertexAttribArray(vNormalTorus);
		glVertexAttribPointer(vNormalTorus, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(torusData)));

		glBindVertexArray(0);
		glUseProgram(0);

		//sphere
		glBindBuffer(GL_ARRAY_BUFFER, buffers[sphereNum]);
		glBindVertexArray(vao[sphereNum]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);

		program[sphereNum] = InitShader("vLIGHTS_v120.glsl", "fLIGHTS_v120.glsl");
		glUseProgram(program[sphereNum]);

		vPositionSphere = glGetAttribLocation(program[sphereNum], "vPosition");
		glEnableVertexAttribArray(vPositionSphere);
		glVertexAttribPointer(vPositionSphere, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		vNormalSphere = glGetAttribLocation(program[sphereNum], "vNormal");
		glEnableVertexAttribArray(vNormalSphere);
		glVertexAttribPointer(vNormalSphere, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

		glBindVertexArray(0);
		glUseProgram(0);

		initScene();

		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClearDepth(1.0);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

		traverse(&nodes[torus]);

		glutSwapBuffers();
	}

	//---------------------------------------------------------
	//		Main Function
	//---------------------------------------------------------
	int main(int argc, char **argv) {
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
		glutInitWindowSize(currentWidth, currentHeight);
		glutCreateWindow("Shaded Stationary Torus, moving camera, single distant light");
		m_glewInitAndVersion();

		init();

		glutMouseFunc(mouse);
		glutMotionFunc(motion);

		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutSpecialFunc(specKey);
		glutTimerFunc(20, animate, 1);
		glutIdleFunc(idle);

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

	void keyboard(unsigned char key, int x, int y) {
		GLfloat xTemp, yTemp, zTemp;


		switch (key)
		{
		case 'z': //z key, roll CW
			uref = u; vref = v;
			u = cos(-0.01)*uref - sin(-0.01)*vref;
			v = sin(-0.01)*uref + cos(-0.01)*vref;
			break;
		case 'Z': //uppercase Z key, roll CCW
			uref = u; vref = v;
			u = cos(0.01)*uref - sin(0.01)*vref;
			v = sin(0.01)*uref + cos(0.01)*vref;

			break;
		case 'x': //x key, pitch up
			vref = v; nref = n;
			v = cos(-0.01)*vref - sin(-0.01)*nref;
			n = sin(-0.01)*vref + cos(-0.01)*nref;
			break;
		case 'X': //uppercase X key, pitch down
			vref = v; nref = n;
			v = cos(0.01)*vref - sin(0.01)*nref;
			n = sin(0.01)*vref + cos(0.01)*nref;
			break;

		case 'c': //c key, yaw CW
			uref = u; nref = n;
			u = cos(-0.01)*uref - sin(-0.01)*nref;
			n = sin(-0.01)*uref + cos(-0.01)*nref;

			break;
		case 'C': //uppercase C key, yaw CCW
			uref = u; nref = n;
			u = cos(0.01)*uref - sin(0.01)*nref;
			n = sin(0.01)*uref + cos(0.01)*nref;
			break;

		}

		glutPostRedisplay(); //redisplay with the new view
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

	static void mouse(int button, int state, int x, int y)
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

	static void motion(int x, int y) {
		if (moving) {
			thetaY = thetaY + (x - startx);
			theta = theta + (y - starty);
			startx = x;
			starty = y;
			glutPostRedisplay();
		}
	}


	