#include "room.h"

room::room()
{
	xMax = 0.0;
	xMin = 0.0;
	yMax = 0.0;
	yMin = 0.0;
	edge1 = false;
	edge2 = false;
	edge3 = false;
	edge4 = false;
	numID = 0;
}

room::room(int numin, double xmax, double xmin, double ymax, double ymin, bool e1, bool e2, bool e3, bool e4)
{
	numID = numin;
	xMax = xmax;
	xMin = xmin;
	yMax = ymax;
	yMin = ymin;
	edge1 = e1;
	edge2 = e2;
	edge3 = e3;
	edge4 = e4;
}

double room::getXmax(){
	return xMax;
}

double room::getXmin() {
	return xMin;
}

double room::getYmax() {
	return yMax;
}

double room::getYmin() {
	return yMin;
}

bool room::getE1() {
	return edge1;
}

bool room::getE2() {
	return edge2;
}

bool room::getE3() {
	return edge3;
}

bool room::getE4() {
	return edge4;
}

void room::setE1(bool inBool) {
	edge1 = inBool;
}

void room::setE2(bool inBool) {
	edge2 = inBool;
}

void room::setE3(bool inBool) {
	edge3 = inBool;
}

void room::setE4(bool inBool) {
	edge4 = inBool;
}

int room::getNumID()
{
	return numID;
}


room::~room()
{
}
