#include "hwk1.h"
#include "room.h"
#include "Angel.h"
#include <cstdlib>
#include <ctime>

typedef Angel::vec4  color4;

// The colors 
color4 boxColor = color4(0.0, 0.5, 0.5, 1.0);
color4 trajectory = color4(0.0, 0.20, 0.5, 1.0);
color4 startPoint = color4(0.5, 0.5, 0.5, 1.0);
color4 background = color4(1.0, 1.0, 1.0, 1.0);

void m_glewInitAndVersion(void);

typedef Angel::vec4  point;

GLuint vao;

//variables for organization purposes
int exit1, exit2, pickRoom;
double xCoord, yCoord;
room roomData[40];
room currentRoom;

//Edge initialization for all the edges on the grid
point wall1[2] = {
	point(-0.5, 0.5, 0.0, 1.0),
	point(-0.25, 0.5, 0.0, 1.0)
};
point wall2[2] = {
	point(-0.25, 0.5, 0.0, 1.0),
	point(0.0, 0.5, 0.0, 1.0)
};
point wall3[2] = {
	point(0.0, 0.5, 0.0, 1.0),
	point(0.25,0.5, 0.0, 1.0)
};
point wall4[2] = {
	point(0.25,0.5, 0.0, 1.0),
	point(0.5, 0.5, 0.0, 1.0)
};
point wall5[2] = {
	point(0.5, 0.5, 0.0, 1.0),
	point(0.5, 0.25, 0.0, 1.0)
};
point wall6[2] = {
	point(0.5, 0.25, 0.0, 1.0),
	point(0.5, 0.0, 0.0, 1.0)
};
point wall7[2] = {
	point(0.5, 0.0, 0.0, 1.0),
	point(0.5, -0.25, 0.0, 1.0)
};
point wall8[2] = {
	point(0.5, -0.25, 0.0, 1.0),
	point(0.5, -0.5, 0.0, 1.0)
};
point wall9[2] = {
	point(0.5, -0.5, 0.0, 1.0),
	point(0.25, -0.5, 0.0, 1.0)
};
point wall10[2] = {
	point(0.25, -0.5, 0.0, 1.0),
	point(0.0, -0.5, 0.0, 1.0)
};
point wall11[2] = {
	point(0.0, -0.5, 0.0, 1.0),
	point(-0.25, -0.5, 0.0, 1.0)
};
point wall12[2] = {
	point(-0.25, -0.5, 0.0, 1.0),
	point(-0.5, -0.5, 0.0, 1.0)
};
point wall13[2] = {
	point(-0.5, -0.5, 0.0, 1.0),
	point(-0.5, -0.25, 0.0, 1.0)
};
point wall14[2] = {
	point(-0.5, -0.25, 0.0, 1.0),
	point(-0.5, 0.0, 0.0, 1.0)
};
point wall15[2] = {
	point(-0.5, 0.0, 0.0, 1.0),
	point(-0.5, 0.25, 0.0, 1.0)
};
point wall16[2] = {
	point(-0.5, 0.25, 0.0, 1.0),
	point(-0.5, 0.5, 0.0, 1.0)
};
point wall17[2]{
	point(-0.25, 0.5, 0.0, 1.0),
	point(-0.25, 0.25, 0.0, 1.0)
};
point wall18[2]{
	point(0.0, 0.5, 0.0, 1.0),
	point(0.0, 0.25, 0.0, 1.0)
};
point wall19[2]{
	point(0.25, 0.5, 0.0, 1.0),
	point(0.25, 0.25, 0.0, 1.0)
};
point wall20[2]{
	point(-0.5, 0.25, 0.0, 1.0),
	point(-0.25, 0.25, 0.0, 1.0)
};
point wall21[2]{
	point(-0.25, 0.25, 0.0, 1.0),
	point(0.0, 0.25, 0.0, 1.0)
};
point wall22[2]{
	point(0.0, 0.25, 0.0, 1.0),
	point(0.25, 0.25, 0.0, 1.0)
};
point wall23[2]{
	point(0.25, 0.25, 0.0, 1.0),
	point(0.5, 0.25, 0.0, 1.0)
};
point wall24[2]{
	point(-0.25, 0.25, 0.0, 1.0),
	point(-0.25, 0.0, 0.0, 1.0)
};
point wall25[2]{
	point(0.0, 0.25, 0.0, 1.0),
	point(0.0, 0.0, 0.0, 1.0)
};
point wall26[2]{
	point(0.25, 0.25, 0.0, 1.0),
	point(0.25, 0.0, 0.0, 1.0)
};
point wall27[2]{
	point(-0.5, 0.0, 0.0, 1.0),
	point(-0.25, 0.0, 0.0, 1.0)
};
point wall28[2]{
	point(-0.25, 0.0, 0.0, 1.0),
	point(0.0, 0.0, 0.0, 1.0)
};
point wall29[2]{
	point(0.0, 0.0, 0.0, 1.0),
	point(0.25, 0.0, 0.0, 1.0)
};
point wall30[2]{
	point(0.25, 0.0, 0.0, 1.0),
	point(0.5, 0.0, 0.0, 1.0)
};
point wall31[2]{
	point(-0.25, 0.0, 0.0, 1.0),
	point(-0.25, -0.25, 0.0, 1.0)
};
point wall32[2]{
	point(0.0, 0.0, 0.0, 1.0),
	point(0.0, -0.25, 0.0, 1.0)
};
point wall33[2]{
	point(0.25, 0.0, 0.0, 1.0),
	point(0.25, -0.25, 0.0, 1.0)
};
point wall34[2]{
	point(-0.5, -0.25, 0.0, 1.0),
	point(-0.25, -0.25, 0.0, 1.0)
};
point wall35[2]{
	point(-0.25, -0.25, 0.0, 1.0),
	point(0.0, -0.25, 0.0, 1.0)
};
point wall36[2]{
	point(0.0, -0.25, -0.25, 1.0),
	point(0.25, -0.25, 0.0, 1.0)
};
point wall37[2]{
	point(0.25, -0.25, 0.0, 1.0),
	point(0.5, -0.25, 0.0, 1.0)
};
point wall38[2]{
	point(-0.25, -0.25, 0.0, 1.0),
	point(-0.25, -0.5, 0.0, 1.0)
};
point wall39[2]{
	point(0.0, -0.25, 0.0, 1.0),
	point(0.0, -0.5, 0.0, 1.0)
};
point wall40[2]{
	point(0.25, -0.25, 0.0, 1.0),
	point(0.25, -0.5, 0.0, 1.0)
};

//uniform variable locations
GLuint color_loc;
GLuint model_view_loc;

mat4 ctmat = Angel::mat4(1.0);


void randomDisplacement(GLfloat magnitude, GLfloat &side1, GLfloat &side2)
{
	GLfloat angle = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	side1 = magnitude * cos(angle);
	side2 = magnitude * sin(angle);
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


/********************************************************************************
*							getRandomStart Function
*	Modified version of instructor's getRandomStart function. This function 
*	will pick a random point on the grid by picking one of the squares on the 
*	grid and choosing a random point in it.
*
**********************************************************************************/
pointNode* getRandomStart(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{
	struct pointNode* retVal;

	GLfloat xLen = roomData[pickRoom].getXmax() - roomData[pickRoom].getXmin();
	GLfloat yLen = roomData[pickRoom].getYmax() - roomData[pickRoom].getYmin();
	currentRoom = roomData[pickRoom];
	GLfloat startX;
	GLfloat startY;

	srand(time(NULL));
	pickRoom = rand() % 16;

	//All these if statements help unify the coordinate system so that the 
	//walls can match the drawn walls 
	if (currentRoom.getXmin() < 0)
	{
		if (currentRoom.getYmin() < 0)
		{
			if (abs(currentRoom.getXmax()) == 0.5)
			{
				if (abs(currentRoom.getYmax()) == 0.5) 
				{
					startX = 0.25 + (rand() % 25) / -100.0;
					startY = 0.25+ (rand() % 25) / -100.0;
				}
				else {
					startX = 0.25+ (rand() % 25) / -100.0;
					startY = (rand() % 25) / -100.0;
				}
			}
			else
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = (rand() % 25) / -100.0;
					startY = 0.25+ (rand() % 25) / -100.0;
				}
				else {
					startX = (rand() % 25) / -100.0;
					startY = (rand() % 25) / -100.0;
				}
			}
		}
		else
		{
			if (abs(currentRoom.getXmax()) == 0.5)
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = 0.25+(rand() % 25) / -100.0;
					startY = 0.25+(rand() % 25) / 100.0;
				}
				else {
					startX = 0.25+(rand() % 25) / -100.0;
					startY = (rand() % 25) / 100.0;
				}
			}
			else
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = (rand() % 25) / -100.0;
					startY = 0.25+(rand() % 25) / 100.0;
				}
				else {
					startX = (rand() % 25) / -100.0;
					startY = (rand() % 25) / 100.0;
				}
			}
		}
	}
	else
	{
		if (currentRoom.getYmin() < 0)
		{
			if (abs(currentRoom.getXmax()) == 0.5)
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = 0.25+(rand() % 25) / 100.0;
					startY = 0.25+(rand() % 25) / -100.0;
				}
				else {
					startX = 0.25+(rand() % 25) / 100.0;
					startY = (rand() % 25) / -100.0;
				}
			}
			else
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = (rand() % 25) / 100.0;
					startY = 0.25+(rand() % 25) / -100.0;
				}
				else {
					startX = (rand() % 25) / 100.0;
					startY = (rand() % 25) / -100.0;
				}
			}
		}
		else
		{
			if (abs(currentRoom.getXmax()) == 0.5)
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = 0.25+(rand() % 25) / 100.0;
					startY = 0.25+(rand() % 25) / 100.0;
				}
				else {
					startX = 0.25+(rand() % 25) / 100.0;
					startY = (rand() % 25) / 100.0;
				}
			}
			else
			{
				if (abs(currentRoom.getYmax()) == 0.5)
				{
					startX = (rand() % 25) / 100.0;
					startY = 0.25+(rand() % 25) / 100.0;
				}
				else {
					startX = (rand() % 25) / 100.0;
					startY = (rand() % 25) / 100.0;
				}
			}
		}
	}

	retVal = (pointNode*)malloc(sizeof(pointNode));
	retVal->x = startX;
	retVal->y = startY;
	retVal->next = NULL;

	return retVal;
}

//wasn't changed...
pointNode* AddNode(struct pointNode* node, GLfloat x, GLfloat y)
{
	struct pointNode* tmp = NULL;
	while (node->next != NULL)
	{
		node = node->next;
	}

	tmp = (pointNode *)malloc(sizeof(pointNode));
	tmp->x = x;
	tmp->y = y;

	tmp->next = NULL;
	node->next = tmp;

	return tmp;
}

//this function will determine the length of the displacement vectors
//it will be 1/50 the shortest side.
GLfloat calcDisplacement(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{
	GLfloat lenX = (currentRoom.getXmax() - currentRoom.getXmin())*4;
	GLfloat lenY = (currentRoom.getYmax() - currentRoom.getYmin())*16;

	if (lenX < lenY)
	{
		return lenX / 50.0;
	}
	else
	{
		return lenY / 50.0;
	}
}

//this function will check to see if the node is withing the box

/********************************************************************************
*								checkNode Function
*	Modified version of instructor's getRandomStart function. This function
*	will check the boundary and see if there is a wall there or if the fly 
*	will go into another square on the grid. When it does, the coordinate system 
*	is changed to fit accordingly.
*
**********************************************************************************/
bool checkNode(struct pointNode * curr, GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{
	int num;
	if (curr->x > xMax)
	{
		if (currentRoom.getE2() == true)
		{
			return false;
		}
		else
		{
			if (currentRoom.getNumID() == 4 || currentRoom.getNumID() == 8 || currentRoom.getNumID() == 12 || currentRoom.getNumID() == 15)
			{
				return false;
			}

			xMin = xMax;
			xMax = xMax + 0.25;
			num = currentRoom.getNumID() +1;
			currentRoom = roomData[num-1];
			return true;
		}
	}
	else if (curr->x < xMin ) {
		if (currentRoom.getE4() == true)
		{
			return false;
		}
		else {
			if (currentRoom.getNumID() == 1 || currentRoom.getNumID() == 5 || currentRoom.getNumID() == 9 || currentRoom.getNumID() == 13)
			{
				return false;
			}
			xMax = xMin;
			xMin = xMin - 0.25;
			num = currentRoom.getNumID();
			num = num-1;
			currentRoom = roomData[num-1];
			return true;
		}
	}
	else if (curr->y > yMax)
	{
		if (currentRoom.getE1() == true)
		{
			return false;
		}
		else
		{
			if (currentRoom.getNumID() == 1 || currentRoom.getNumID() == 2 || currentRoom.getNumID() == 3 || currentRoom.getNumID() == 4)
			{
				return false;
			}
			yMin = yMax;
			yMax = yMax + 0.25;
			num = currentRoom.getNumID() - 4;
			currentRoom = roomData[num-1];
			return true;
		}
	}
	else if (curr->y < yMin)
	{
		if (currentRoom.getE3() == true)
		{
			return false;
		}
		else
		{
			if (currentRoom.getNumID() == 13 || currentRoom.getNumID() == 14 || currentRoom.getNumID() == 15 || currentRoom.getNumID() == 16)
			{
				return false;
			}
			yMax = yMin;
			yMin = yMin - 0.25;
			num = currentRoom.getNumID() + 4;
			currentRoom = roomData[num - 1];
			return true;
		}
	}
	else
		return true;
}


/********************************************************************************
*								moreinit Function
*		Initializations that I want to have done before anything else. 
*
**********************************************************************************/
void moreinit()
{
	srand(time(NULL));
	pickRoom = rand() % 16;
	exit1 = rand() % 16;
	do {
		exit2 = rand() % 16;
	} while (exit2 == exit1 || exit2 + 1 == exit1 || exit2 - 1 == exit1 || (exit2 == 16 && exit1 == 1) || (exit1 == 16 && exit2 == 1));

	room room1;
	if (exit1 == 1 || exit2 == 1)
		room1 = room(1, -0.25, -0.5, 0.5, 0.25, false, true, false, true);
	else if (exit1 == 16 || exit2 == 16)
		room1 = room(1, -0.25, -0.5, 0.5, 0.25, true, true, false, false);
	else
		room1 = room(1, -0.25, -0.5, 0.5, 0.25, true, true, false, true);
	roomData[0] = room1;

	room room2;
	if (exit1 == 2 || exit2 == 2)
		room2 = room(2, 0.0, -0.25, 0.5, 0.25, false, false, false, true);
	else
		room2 = room(2, 0.0, -0.25, 0.5, 0.25, true, false, false, true);
	roomData[1] = room2;

	room room3;
	if (exit1 == 3 || exit2 == 3)
		room3 = room(3, 0.25, 0.0, 0.5, 0.25, false, true, false, false);
	else
		room3 = room(3, 0.25, 0.0, 0.5, 0.25, true, true, false, false);
	roomData[2] = room3;

	room room4;
	if (exit1 == 4 || exit2 == 4)
		room4 = room(4, 0.5, 0.25, 0.5, 0.25, false, true, false, true);
	else if (exit1 == 5 || exit2 == 5)
		room4 = room(4, 0.5, 0.25, 0.5, 0.25, true, false, false, true);
	else
		room4 = room(4, 0.5, 0.25, 0.5, 0.25, true, true, false, true);
	roomData[3] = room4;

	room room5;
	if (exit1 == 15 || exit2 == 15)
		room5 = room(5, -0.25, -0.5, 0.25, 0.0, false, true, false, false);
	else
		room5 = room(5, -0.25, -0.5, 0.25, 0.0, false, true, false, true);
	roomData[4] = room5;

	room room6 = room(6, 0.0, -0.25, 0.25, 0.0, false, false, false, true);
	roomData[5] = room6;
	room room7 = room(7, 0.25, 0.0, 0.25, 0.0, false, true, true, false);
	roomData[6] = room7;

	room room8;
	if (exit1 == 6 || exit2 == 6)
		room8 = room(8, 0.5, 0.25, 0.25, 0.0, false, false, false, true);
	else
		room8 = room(8, 0.5, 0.25, 0.25, 0.0, false, true, false, true);
	roomData[7] = room8;

	room room9;
	if (exit1 == 14 || exit2 == 14)
		room9 = room(9, -0.25, -0.5, 0.0, -0.25, false, false, true, false);
	else
		room9 = room(9, -0.25, -0.5, 0.0, -0.25, false, false, true, true);
	roomData[8] = room9;

	room room10 = room(10, 0.0, -0.25, 0.0, -0.25, false, false, true, false);
	roomData[9] = room10;
	room room11 = room(11, 0.25, 0.0, 0.0, -0.25, true, false, false, false);
	roomData[10] = room11;

	room room12;
	if (exit1 == 7 || exit2 == 7)
		room12 = room(12, 0.5, 0.25, 0.0, -0.25, false, false, true, false);
	else
		room12 = room(12, 0.5, 0.25, 0.0, -0.25, false, true, true, false);
	roomData[11] = room12;

	room room13;
	if (exit1 == 12 || exit2 == 12)
		room13 = room(13, -0.25, -0.5, -0.25, -0.5, true, false, false, true);
	else if (exit1 == 13 || exit2 == 13)
		room13 = room(13, -0.25, -0.5, -0.25, -0.5, true, false, true, false);
	else
		room13 = room(13, -0.25, -0.5, -0.25, -0.5, true, false, true, true);
	roomData[12] = room13;

	room room14;
	if (exit1 == 11 || exit2 == 11)
		room14 = room(14, 0.0, -0.25, -0.25, -0.5, true, false, false, true);
	else
		room14 = room(14, 0.0, -0.25, -0.25, -0.5, true, false, true, false);
	roomData[13] = room14;

	room room15;
	if (exit1 == 10 || exit2 == 10)
		room15 = room(15, 0.25, 0.0, -0.25, -0.5, false, false, false, false);
	else
		room15 = room(15, 0.25, 0.0, -0.25, -0.5, false, false, true, false);
	roomData[14] = room15;

	room room16;
	if (exit1 == 8 || exit2 == 8)
		room16 = room(16, 0.5, 0.25, -0.25, -0.5, true, false, true, false);
	else if (exit1 == 9 || exit2 == 9)
		room16 = room(16, 0.5, 0.25, -0.25, -0.5, true, true, false, false);
	else
		room16 = room(16, 0.5, 0.25, -0.25, -0.5, true, true, true, false);
	roomData[15] = room16;
	width = currentRoom.getXmax() - currentRoom.getXmin();
	height = currentRoom.getYmax() - currentRoom.getYmin();
}

//the other initializations
void init()
{

	glGenBuffers(44, &buffers[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[41]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bBox), bBox, GL_STATIC_DRAW);

	// Load shaders and use the resulting shader program
	program = InitShader("hwk1/vshader23_v110.glsl", "hwk1/fshader23_v110.glsl");
	glUseProgram(program);
	projmat_loc = glGetUniformLocation(program, "projmat");
	modelview_loc = glGetUniformLocation(program, "modelview");
	draw_color_loc = glGetUniformLocation(program, "vColor");


	// set up vertex attributes arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//THE STUFF FROM LABYRINTH

	// Create a vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall1), wall1, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall2), wall2, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall3), wall3, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall4), wall4, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall5), wall5, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall6), wall6, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall7), wall7, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall8), wall8, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall9), wall9, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall10), wall10, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[10]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall11), wall11, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[11]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall12), wall12, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[12]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall13), wall13, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[13]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall14), wall14, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[14]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall15), wall15, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[15]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall16), wall16, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[16]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall17), wall17, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[17]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall18), wall18, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[18]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall19), wall19, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[19]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall20), wall20, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[20]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall21), wall21, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[21]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall22), wall22, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[22]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall23), wall23, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[23]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall24), wall24, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[24]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall25), wall25, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[25]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall26), wall26, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[26]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall27), wall27, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[27]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall28), wall28, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[28]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall29), wall29, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[29]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall30), wall30, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[30]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall31), wall31, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[31]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall32), wall32, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[32]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall33), wall33, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[33]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall34), wall34, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[34]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall35), wall35, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[35]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall36), wall36, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[36]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall37), wall37, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[37]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall38), wall38, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[38]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall39), wall39, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[39]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall40), wall40, GL_STATIC_DRAW);


	//Starte the Brownian motion

	head = getRandomStart(currentRoom.getXmin(), currentRoom.getXmax(), currentRoom.getYmin(), currentRoom.getYmax());
	curr = head;
	
	vPosition = glGetAttribLocation(program, "vPosition");

	glEnableVertexAttribArray(vPosition);


	color_loc = glGetUniformLocation(program, "color");
	model_view_loc = glGetUniformLocation(program, "modelview"); 

	glClearColor(1.0, 1.0, 1.0, 1.0);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	projmat = Angel::mat4(1.0);
	glUniformMatrix4fv(projmat_loc, 1, GL_TRUE, projmat);
	glLineWidth(2.0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[44]);

	//Mark initial location
	glBindBuffer(GL_ARRAY_BUFFER, buffers[41]);
	modelview = Angel::mat4(1.0)*Angel::Translate(vec4(head->x, head->y, 0.0, 0.0))*Angel::Scale((width / 16.0), (height / 16.0), 1.0);
	glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniform4fv(draw_color_loc, 1, startPoint);
	glDrawArrays(GL_QUADS, 0, 4);

	//loop to draw each line
	for (int i = 0; i < 40; i++)
	{
		if (i != exit1 &&  i != exit2)
		{
			if (i<16 || i == 16 || i == 18 || i == 23 || i == 25 || i == 28 || i == 33 || i == 34 || i == 36)
			{
				modelview = Angel::mat4(1.0);
				glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
				glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glUniform4fv(draw_color_loc, 1, boxColor);
				glDrawArrays(GL_LINES, 0, 2);
			}
			else
			{
				modelview = Angel::mat4(1.0);
				glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
				glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glUniform4fv(draw_color_loc, 1, background);
				glDrawArrays(GL_LINES, 0, 2);
			}
		}
	}

	//Draw trajectory
	modelview = Angel::mat4(1.0);
	glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
	//copy the trajectory into a buffer 
	GLfloat * trajectoryBuffer = copyToArray(head);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[42]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * pointCount(head), trajectoryBuffer, GL_STREAM_DRAW);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniform4fv(draw_color_loc, 1, trajectory);
	glDrawArrays(GL_LINE_STRIP, 0, pointCount(head));

	GLfloat linewidth = width;

	//finishing marker
	if (bComplete)
	{ 
		GLfloat linewidth = 0.125;
		glLineWidth(2.5);
		glUniform4fv(draw_color_loc, 1, red_exit_marker);
		glBegin(GL_LINES);
		glVertex3f(xCoord - linewidth / 2, yCoord - linewidth / 2, 0.0);
		glVertex3f(xCoord + linewidth / 2, yCoord + linewidth / 2, 0.0);
		glVertex3f(xCoord - linewidth / 2, yCoord + linewidth / 2, 0.0);
		glVertex3f(xCoord + linewidth / 2, yCoord - linewidth / 2, 0.0);
		glEnd();
	}
	glutSwapBuffers();
	delete[] trajectoryBuffer;
	glFlush();
}

GLfloat* copyToArray(struct pointNode * head)
{
	GLfloat * retVal;
	pointNode * tmp;
	int count_l;
	count_l = pointCount(head);
	int i = 0;

	count_l *= 2;

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
		tmp = tmp->next;
		i += 2;
	}
	return retVal;
}

void animate(int i)
{
	GLfloat x, y;
	pointNode * last;
	if (!bPaused && !bComplete)
	{
		GLfloat displacement = calcDisplacement(currentRoom.getXmin(), currentRoom.getXmax(), currentRoom.getYmin(), currentRoom.getYmax());

		last = curr;
		randomDisplacement(displacement, x, y);
		curr = AddNode(curr, curr->x + x, curr->y + y);
		int count_l;
		count_l = pointCount(head);

		glutPostRedisplay();
		//We only want to keep going if 
		if (checkNode(curr, currentRoom.getXmin(), currentRoom.getXmax(), currentRoom.getYmin(), currentRoom.getYmax()))
		{
			//keep a roughly constat fps
			glutTimerFunc(17, animate, 0);
		}
		else
		{
			findExitPoint(last, curr);
			xCoord = last->x;
			yCoord = last->y;
			bComplete = true;
		}
	}
}

//pauses and quits with the push of the right buttons
void keyboard(unsigned char key, int x, int y)
{
	if (key == 'p')
	{
		bPaused = !bPaused;
		if (!bPaused)
			animate(0);
	}
	if (key == 'q' || 'Q')
	{
		exit(0);
	}
}

void findExitPoint(struct pointNode * prev, struct pointNode * last)
{
	GLfloat slope;
	GLfloat b; // y -intercept of  the line y = slope*x + b

	GLfloat yExit;
	GLfloat xExit;

	//this will make sure one of the nodes is inside the other is outside
	if (checkNode(prev, currentRoom.getXmin(), currentRoom.getXmax(), currentRoom.getYmin(), currentRoom.getYmax()) && !checkNode(last, currentRoom.getXmin(), currentRoom.getXmax(), currentRoom.getYmin(), currentRoom.getYmax()))
	{
		slope = (last->y - prev->y) / (last->x - prev->x);
		b = prev->y - slope*prev->x;

		if (last->x > currentRoom.getXmax())
		{
			yExit = slope * currentRoom.getXmax() + b;
			xExit = currentRoom.getXmax();
			if (yExit < currentRoom.getYmin())
			{
				yExit = currentRoom.getYmin();
				xExit = (yExit - b) / slope;
			}
			if (yExit > currentRoom.getXmax())
			{
				yExit = currentRoom.getXmax();
				xExit = (yExit - b) / slope;
			}
		}
		else if (last->x < currentRoom.getXmin())
		{
			yExit = slope * currentRoom.getXmin() + b;
			xExit = currentRoom.getXmin();
			if (yExit < currentRoom.getYmin())
			{
				yExit = currentRoom.getYmin();
				xExit = (yExit - b) / slope;
			}
			if (yExit > currentRoom.getYmax())
			{
				yExit = currentRoom.getYmax();
				xExit = (yExit - b) / slope;
			}
		}
		else if (last->y > currentRoom.getYmax())
		{
			yExit = currentRoom.getYmax();
			xExit = (yExit - b) / slope;
		}
		else if (last->y < currentRoom.getYmin())
		{
			yExit = currentRoom.getYmin();
			xExit = (yExit - b) / slope;
		}
	}
}



int main(int argc, char** argv)
{
	moreinit();
	struct pointNode* tmp = NULL;

	srand(time(NULL));
	rand();

	height = (maxX - minY)*6;
	width = (maxX - minY)*6;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(int(width * 400), int(height * 400));
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Flying Around a Labyrinth");
	m_glewInitAndVersion();
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	//wait a bit before we start
	glutTimerFunc(1000, animate, 0);
	glutMainLoop();
	return 0;
}
