#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <ctime>
#include "Angel.h"

using namespace std;

#define PI 3.14159265
#define MIN_X 0
#define MIN_Y 1
#define MAX_X 2
#define MAX_Y 3
#define MAX_Z 4
#define MIN_Z 5

GLfloat width;
GLfloat height;
//GLfloat length = 0;

GLuint buffers[10];
GLuint vPosition;
//GLuint program; 
GLuint projmat_loc;
GLuint modelview_loc;
GLuint draw_color_loc;
Angel::vec4 blue_trajectory =    Angel::vec4( 0.0, 0.0, 1.0, 1.0 );
Angel::vec4 green_start_marker = Angel::vec4( 0.0, 1.0, 0.0, 1.0 );
Angel::vec4 red_exit_marker =    Angel::vec4( 1.0, 0.0, 0.0, 1.0 );
Angel::vec4 yelow_box_edge =     Angel::vec4( 1.0, 1.0, 0.0, 1.0 );


//Angel::mat4 projmat;
Angel::mat4 modelview;

GLfloat minX = -0.5, minY = -0.5, maxX = 0.5, maxY = 0.5, minZ = -0.5, maxZ = 0.5;


Angel::vec2 bBox[4]={
	Angel::vec2(minX, minY),
	Angel::vec2(maxX, minY),
	Angel::vec2(maxX, maxY),
	Angel::vec2(minX, maxY)
};


int count;
struct pointNode* head = NULL;
struct pointNode* curr = NULL;
bool bPaused = true;
bool bComplete = false;

/*void m_glewInitAndVersion(void)
{
   fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
   fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
   fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }
   fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}*/


void randomDisplacement(GLfloat magnitude, GLfloat &x, GLfloat &y, GLfloat &z);
int pointCount(struct pointNode* head);
void printData(struct pointNode* head);
pointNode* getRandomStart(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax);
pointNode* AddNode(struct pointNode* node, GLfloat x, GLfloat y, GLfloat z);
GLfloat calcDisplacement(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax);
bool checkNode(struct pointNode * curr, GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax);
void init ();
void display();
GLfloat* copyToArray(struct pointNode * head);
void animate(int i);
//void keyboard (unsigned char key, int x, int y);
void findExitPoint(struct pointNode * prev, struct pointNode * last);
void buildRectangle();

//This will be the basis of linked list to hold pointer data. 
struct pointNode
{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	pointNode *next;
};

void setMinMax(double maxx, double minx, double maxy, double miny, double maxz, double minz)
{
	GLfloat minX = (GLfloat)minx, minY = (GLfloat)miny, maxX = (GLfloat)maxx, maxY = (GLfloat)maxy, minZ = (GLfloat)minz, maxZ = (GLfloat)maxz;
};


