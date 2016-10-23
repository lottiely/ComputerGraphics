//CG George Kamberov
//
//A simple 3D scene:
//			A cube mesh and a wireframe line strip built using the pointers-to-vertices DS and 
//			the Draw Elements mechanism.
//		
//
//
//
// Interaction: 
//     
//      
//      Use the up/down direction (arrow)  keys to Move the camera back/forward  
//      (along the fixed viewing axis). 
//
// We use perspective projection.
//
// Shaders used: 
//  vshader30_TwoCubes_FullPipe.glsl
//  vshader6_2.glsl
//  fshader30_TwoCubes.glsl
//  fshader6_2.glsl
//

#include "hwk1.h"
#include "Angel.h"
#include<assert.h>
#include <cstdlib>
#include <ctime>
#include <math.h>


///////CONSTANT/////////////////////
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0


void m_glewInitAndVersion(void);
void reshape(int width, int height);

GLuint program[3]; 
GLuint VAO[3];
int collisionArea=-1;

enum {cube_vertices, trajectory, cube_indeces, wire_indeces};

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



//----------------------------------------------------------------------------

/////Camera unv basis///////

Angel::vec4 n = vec4(0.0, 0.0, 1.0, 0.0);
Angel::vec4 v = vec4(0.0, 1.0, 0.0, 00);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.1, 0.0, 2.0, 1.0);

GLfloat theta = -45;
GLfloat thetaY = 0;
GLfloat thetaZ = 0;

int startx;
int starty;
int moving;

GLfloat step = 20;
GLfloat theta_y = 0;
int spinToggle = 0; 
GLfloat d=0.5;
int w; int h;


// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[16] = {
    point4(-0.125, 0.5,  0.5, 1.0),		
	point4(0.125, 0.5,  0.5, 1.0),	
	point4(-0.5, 0.125,  0.5, 1.0),
	point4(0.5, 0.125,  0.5, 1.0),
	point4(-0.5, -0.125,  0.5, 1.0),
	point4(0.5, -0.125,  0.5, 1.0),
	point4(-0.125, -0.5, 0.5, 1.0),
	point4(0.125, -0.5,  0.5, 1.0),
	point4(-0.125, 0.5,  -0.5, 1.0),
	point4(0.125, 0.5,  -0.5, 1.0),
	point4(-0.5, 0.125,  -0.5, 1.0),
	point4(0.5, 0.125,  -0.5, 1.0),
	point4(-0.5, -0.125,  -0.5, 1.0),
	point4(0.5, -0.125,  -0.5, 1.0),
	point4(-0.125, -0.5,  -0.5, 1.0),
	point4(0.125, -0.5,  -0.5, 1.0)
};

GLubyte indices[]= {
	0,1,2,
	2,4,6,
	6,7,5,
	5,3,1,
	1,2,6,
	1,5,6,
	8,9,10,
	10,12,14,
	14,15,13,
	13,11,9,
	9,10,14,
	9,13,14,
	8,9,0,
	9,0,1,
	9,11,1,
	11,1,3,
	11,13,3,
	13,3,5,
	13,15,7,
	13,7,5,
	14,15,6,
	15,6,7,
	12,14,4,
	14,4,6,
	10,12,2,
	12,2,4,
	10,8,2,
	8,2,0
};

GLubyte indices_wire[]= {
	0,1,
	1,3,
	3,5,
	5,7,
	7,6,
	6,4,
	4,2,
	0,2,
	8,9,
	9,11,
	11,13,
	13,15,
	14,15,
	12,14,
	10,12,
	10,8,
	2,10,
	0,8,
	1,9,
	3,11,
	5,13,
	7,15,
	6,14,
	4,12
};

// RGBA colors
color4 opaqueT = color4( 0.0, 0.0, 0.0, 1.0 ); 
color4 transparent = color4(0.2, 0.5, 1.0, 0.1);
color4 trajectoryColor = color4(0.0, 0.0, 1.0, 1.0);

//uniform variables locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;


//make sure you start with the default coordinate system
mat4 projmat = mat4(1.0);

mat4 modelviewStackTop = mat4(1.0);
mat4 modelviewStackBottom=  mat4(1.0);

////////////BROWNIAN FUNCTIONS////////////////
void randomDisplacement(GLfloat magnitude, GLfloat &side1, GLfloat &side2, GLfloat &side3)
{
	GLfloat angle = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	GLfloat angle2 = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	side1 = magnitude * cos(angle)* cos(angle2);
	side2 = magnitude * sin(angle)* cos(angle2);
	side3 = magnitude * sin(angle2);
}

//this will count all the nodes after head;
int pointCount(struct pointNode* head)
{
	pointNode* tmp;
	tmp = head;
	int count_l = 0;

	while (tmp != NULL)
	{
		count_l++;
		tmp = tmp->next;
	}
	return count_l;
}


pointNode* getRandomStart(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax)
{
	struct pointNode* retVal;
	GLfloat xLen = xMax - xMin;
	GLfloat yLen = yMax - yMin;
	GLfloat zLen = zMax - zMin;

	GLfloat startX = ((GLfloat)rand() / (GLfloat)RAND_MAX) * xLen + xMin;
	GLfloat startY = ((GLfloat)rand() / (GLfloat)RAND_MAX) * yLen + yMin;
	GLfloat startZ = ((GLfloat)rand() / (GLfloat)RAND_MAX) * zLen + zMin;

	retVal = (pointNode*)malloc(sizeof(pointNode));
	retVal->x = startX;
	retVal->y = startY;
	retVal->z = startZ;
	retVal->next = NULL;

	return retVal;
}

pointNode* AddNode(struct pointNode* node, GLfloat x, GLfloat y, GLfloat z)
{
	struct pointNode* tmp = NULL;
	while (node->next != NULL)
	{
		node = node->next;
	}

	tmp = (pointNode *)malloc(sizeof(pointNode));
	tmp->x = x;
	tmp->y = y;
	tmp->z = z;

	tmp->next = NULL;
	node->next = tmp;

	return tmp;
}

//this function will determine the length of the displacement vectors
//it will be 1/50 the shortest side.
GLfloat calcDisplacement(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax)
{
	GLfloat lenX = xMax - xMin;
	GLfloat lenY = yMax - yMin;
	GLfloat lenZ = zMax - zMin;

	if (lenX < lenY)
	{
		if (lenX < lenZ)
			return lenX / 50.0;
		else
			return lenZ / 50.0;
	}
	else
	{
		if (lenY < lenZ)
			return lenY / 50.0;
		else
			return lenZ / 50.0;
	}
}

//this function will check to see if the node is withing the box
bool checkNode(struct pointNode * curr, GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax)
{
	GLfloat topLeftSlope = ((GLfloat) 0.5 - (GLfloat) 0.125) / ((GLfloat)-0.125 + (GLfloat) 0.5);
	GLfloat topRightSlope = ((GLfloat) 0.5 - (GLfloat) 0.125) / ((GLfloat)0.125 - (GLfloat) 0.5);
	GLfloat bottomLeftSlope = ((GLfloat) -0.125 + (GLfloat) 0.5) / ((GLfloat)-0.5 + (GLfloat) 0.125);
	GLfloat bottomRightSlope = ((GLfloat) -0.125 + (GLfloat) 0.5) / ((GLfloat)0.5 - (GLfloat) 0.125);

	GLfloat b;

	//Checks to see if it gets near the top left of the polygon
	if (curr->x < xMin)
	{
		//collision with side 7
		collisionArea = 7;
		return false;
	}
	else if (curr->y < yMin)
	{
		//collision with side 5
		collisionArea = 5;
		return false;
	}
	else if (curr->z < zMin)
	{
		//collision with bottom 10
		collisionArea = 10;
		return false;
	}
	else if (curr->x > xMax)
	{
		//collision with side 3
		collisionArea = 3;
		return false;
	}
	else if (curr->y > yMax)
	{
		//collision with side 1
		collisionArea = 1;
		return false;
	}
	else if (curr->z > zMax)
	{
		//collision with side 9
		collisionArea = 9;
		return false;
	}
	else if (curr->x < -0.125 && curr->y >0.125)
	{
		b = 0.5 - (topLeftSlope*-0.125); //Finding the intercept for the y = mx +b equation
		if (curr->y > topLeftSlope*curr->x + b) //Checking to see if the y value is past the sloped line
		{
			//collision with side 8
			collisionArea = 8;
			return false;
		}
		else
			return true;
	}
	//Checks to see if it gets near the bottom left of the polygon
	else if (curr->x < -0.125 && curr->y < -0.125)
	{
		b = -0.125 - (bottomLeftSlope*-0.5);
		if (curr->y < bottomLeftSlope*curr->x + b)
		{
			//collision with side 6
			collisionArea = 6;
			return false;
		}
		else
			return true;
	}
	//Checks to see if it gets near the top right of the polygon
	else if (curr->x > 0.125 && curr->y > 0.125 )
	{
		b = 0.5 - (topRightSlope*0.125);
		if (curr->y > topRightSlope*curr->x + b)
		{
			//collision with side 2
			collisionArea = 2;
			return false;
		}
		else
			return true;
	}
	//Checks to see if it gets near the bottom right of the polygon
	else if (curr->x > 0.125 && curr->y < -0.125)
	{
		b = -0.125 - (bottomRightSlope*0.5);
		if (curr->y < bottomRightSlope*curr->x + b)
		{
			//collision with side 4
			collisionArea = 4;
			return false;
		}
		else
			return true;
	}
	else
	{
		return true;
	}
}
//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	
/**
***  Create and initialize buffer objects
**/
	//////////////BROWNIAN INIT////////////////
	//Starte the Brownian motion
	head = getRandomStart(minX, maxX, minY, maxY, minZ, maxZ);
	curr = head;
	
	glGenBuffers(4, buffers);
	
	//Vertex buffer for the vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

	//Elements buffer for the pointers
	 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces] ); 
	 glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[wire_indeces] ); 
	 glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_wire), indices_wire, GL_STATIC_DRAW); 

	 glGenVertexArrays(3, VAO);
	

// Load shaders and use the resulting shader programs
    program[0] = InitShader( "vshader30_TwoCubes_FullPipe.glsl", "fshader30_TwoCubes.glsl" ); 
	program[1] = InitShader( "vshader30_TwoCubes_FullPipe.glsl", "fshader6_2.glsl" ); 
	////////BROWNIAN SHADERS////////////////////////
	program[2] = InitShader("vshader23_v110.glsl", "fshader23_v110.glsl");

	//projmat_loc = glGetUniformLocation(program[2], "projmat");
	//modelview_loc = glGetUniformLocation(program[2], "modelview");
	draw_color_loc = glGetUniformLocation(program[2], "vColor");
    
	// set up vertex attributes arrays
	//glUseProgram(program[2]);
	glBindVertexArray(VAO[2]);
	GLuint vPosition = glGetAttribLocation(program[2], "vPosition");
	glEnableVertexAttribArray(vPosition);
	//glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//done with this packet
	glBindVertexArray(0);

	///////////////////////////////////////////////////

   //VAO[0] the Cube 
	glUseProgram( program[0] );
	glBindVertexArray(VAO[0]);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces]);
	GLuint vPosition_Cube = glGetAttribLocation( program[0], "vPosition" );
	glEnableVertexAttribArray( vPosition_Cube );
	glVertexAttribPointer( vPosition_Cube, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
   	//done with this packet
    glBindVertexArray(0); 

	//VAO[1] the wireframe
	glUseProgram(program[1]);
	glBindVertexArray(VAO[1]);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	GLuint vPosition_Wire = glGetAttribLocation( program[1], "vPosition" ); 
	glEnableVertexAttribArray( vPosition_Wire );
	glVertexAttribPointer( vPosition_Wire, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[wire_indeces]);
	//done with this packet
    glBindVertexArray(0);

	glUseProgram(0);

	glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------

void
display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 	//Make sure you start with the Default Coordinate Systems
	projmat=mat4(1.0);
	modelviewStackTop = mat4(1.0);
	mat4 modelviewStackBottom =  modelviewStackTop;
	/////
	glUniformMatrix4fv(projmat_loc, 1, GL_TRUE, projmat);
	////
	//Set up the camera optics
	projmat = projmat*Perspective(30,1.0,0.1,20.0); 
	//Position and orient the camera
	modelviewStackTop = modelviewStackTop*LookAt(eye, eye-n, v);
    modelviewStackTop = modelviewStackTop*Translate(0.15, 0.15, -2.5);
	//Spin the whole scene by pressing the X key
	modelviewStackTop = modelviewStackTop*RotateX(theta);
	modelviewStackTop = modelviewStackTop*RotateY(thetaY);
	modelviewStackTop = modelviewStackTop*RotateZ(thetaZ);
	
	modelviewStackBottom =  modelviewStackTop; //mvstack.push
	if (bComplete == true)
	{
		if (collisionArea == 1)
		{
			//36, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(36));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 2)
		{
			//42, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(42));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 3)
		{
			//48, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(48));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 4)
		{
			//54,6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(54));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 5)
		{
			//60, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(60));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 6)
		{
			//66, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(66));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 7)
		{
			// 72, 6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(72));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 8)
		{
			//78,6
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(78));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if (collisionArea == 9)
		{
			//top 18, 0
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
		else if(collisionArea == 10)
		{
			//bottom 18, 18
			glUseProgram(program[0]);
			color_loc = glGetUniformLocation(program[0], "color");
			proj_loc = glGetUniformLocation(program[0], "projection");
			model_view_loc = glGetUniformLocation(program[0], "modelview");
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
			glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
			glUniform4fv(color_loc, 1, opaqueT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glBindVertexArray(VAO[0]);
			glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_BYTE, BUFFER_OFFSET(18));
			glDisable(GL_POLYGON_OFFSET_FILL);

			modelviewStackTop = modelviewStackBottom; //mvstack.pop
		}
	}
	
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//Render wireframe
	glUseProgram(program[1]);
	proj_loc       = glGetUniformLocation(program[1], "projection");
	model_view_loc = glGetUniformLocation(program[1], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_LINES, 48, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	
	modelviewStackTop = modelviewStackBottom; //mvstack.pop

	//////////////////////////////////////
	//Draw trajectory
	glUseProgram(program[2]);
	color_loc = glGetUniformLocation(program[2], "color");
	model_view_loc = glGetUniformLocation(program[2], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);

	//copy the trajectory into a buffer 
	GLfloat * trajectoryBuffer = copyToArray(head);
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[trajectory]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * pointCount(head), trajectoryBuffer, GL_STREAM_DRAW);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniform4fv(draw_color_loc, 1, trajectoryColor);
	glDrawArrays(GL_LINE_STRIP, 0, pointCount(head));

	modelviewStackTop = modelviewStackBottom;
	delete[] trajectoryBuffer;
	/////////////////////////////////////////

	//Render surface mesh
	glUseProgram(program[0]);
	color_loc = glGetUniformLocation(program[0], "color");
	proj_loc = glGetUniformLocation(program[0], "projection");
	model_view_loc = glGetUniformLocation(program[0], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
	glUniform4fv(color_loc, 1, transparent);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glBindVertexArray(VAO[0]);
	glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	glDisable(GL_POLYGON_OFFSET_FILL);

	modelviewStackTop = modelviewStackBottom; //mvstack.pop
	glDepthMask(GL_TRUE);

	glutSwapBuffers();

}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 'x': {	theta += step;
		if (theta > 360.0) {
			theta -= 360.0;
		}
	}
			  break;
	case 'X': {	theta -= step;
		if (theta < -360.0) {
			theta += 360.0;
		}
	}
			  break;
	case 'Z': {	thetaZ += step;
		if (thetaZ > 360.0) {
			thetaZ -= 360.0;
		}
	}
			  break;
	case 'z': {	thetaZ += step;
		if (thetaZ > 360.0) {
			thetaZ -= 360.0;
		}
	}
			  break;
	case 'y': {	thetaY += step;
		if (thetaY > 360.0) {
			thetaY -= 360.0;
		}
	}
			  break;
	case 'Y': {	thetaY += step;
		if (thetaY > 360.0) {
			thetaY -= 360.0;
		}
	}
			  break;
	case 'i': case 'I':{
		bPaused = !bPaused;
		if (!bPaused)
			animate(0);
		break;
	}
	case 's': case 'S': {
		bPaused = !bPaused;
		if(!bPaused)
			animate(0);
		break;
	}
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
	}
}
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
///////////////////BROWNIAN STUFF//////////////////////
GLfloat* copyToArray(struct pointNode * head)
{
	GLfloat * retVal;
	pointNode * tmp;
	int count_l;
	count_l = pointCount(head);
	int i = 0;

	count_l *= 3;

	tmp = head;
	if (count_l > 0)
	{

		retVal = new GLfloat[count_l];
	}
	else
	{
		return NULL;
	}

	while (i < count_l)
	{
		retVal[i] = tmp->x;
		retVal[i + 1] = tmp->y;
		retVal[i + 2] = tmp->z;
		tmp = tmp->next;
		i += 3;
	}
	return retVal;
}

void animate(int i)
{
	GLfloat x, y, z;
	pointNode * last;
	if (!bPaused && !bComplete)
	{
		GLfloat displacement = calcDisplacement(minX, maxX, minY, maxY, minZ, maxZ);

		last = curr;
		randomDisplacement(displacement, x, y,z);
		curr = AddNode(curr, curr->x + x, curr->y + y, curr->z + z);
		int count_l;
		count_l = pointCount(head);

		glutPostRedisplay();
		//We only want to keep going if 
		if (checkNode(curr, minX, maxX, minY, maxY, minZ, maxZ))
		{
			//keep a roughly constat fps
			glutTimerFunc(17, animate, 0);
		}
		else
		{
			findExitPoint(last, curr);
			bComplete = true;
		}
	}
}
void findExitPoint(struct pointNode * prev, struct pointNode * last)
{
	GLfloat slope;
	GLfloat b; // y -intercept of  the line y = slope*x + b

	GLfloat yExit;
	GLfloat xExit;
	//GLfloat zExit;

	if (checkNode(prev, minX, maxX, minY, maxY, minZ, maxZ) && !checkNode(last, minX, maxX, minY, maxY, minZ, maxZ))
	{
		slope = (last->y - prev->y) / (last->x - prev->x);
		b = prev->y - slope*prev->x;

		if (last->x > maxX)
		{
			yExit = slope * maxX + b;
			xExit = maxX;
			if (yExit < minY)
			{
				yExit = minY;
				xExit = (yExit - b) / slope;
			}
			if (yExit > maxY)
			{
				yExit = maxY;
				xExit = (yExit - b) / slope;
			}
		}
		else if (last->x < minX)
		{
			yExit = slope * minX + b;
			xExit = minX;
			if (yExit < minY)
			{
				yExit = minY;
				xExit = (yExit - b) / slope;
			}
			if (yExit > maxY)
			{
				yExit = maxY;
				xExit = (yExit - b) / slope;
			}
		}
		else if (last->y > maxY)
		{
			yExit = maxY;
			xExit = (yExit - b) / slope;
		}
		else if (last->y < minY)
		{
			yExit = minY;
			xExit = (yExit - b) / slope;
		}
	}
}
//////////////////////////////////////////////////////

void specKey(int key, int x, int y)
{ 
switch( key ) {

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
  }
}

void idle(void){
  glutPostRedisplay();
}
//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	struct pointNode* tmp = NULL;

	srand(time(NULL));
	rand();

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Pointers-to-Vertex, VAOs, Multiple Pipelines" );
	m_glewInitAndVersion();
	
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

    init();

	glutReshapeFunc(reshape);
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);
	//wait a bit before we start
	glutTimerFunc(1000, animate, 0);
	/////

    glutMainLoop();
    return 0;
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

void reshape( int width, int height )
//the same objects are shown possibly distorted  
//original viewport is a square
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

}
