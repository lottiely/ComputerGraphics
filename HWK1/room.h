#pragma once
class room
{
private:
	double xMax;
	double xMin;
	double yMax;
	double yMin;
	bool edge1;
	bool edge2;
	bool edge3;
	bool edge4;
	int numID;

public:
	room();
	room(int num, double xmax, double xmin, double ymax, double ymin, bool e1, bool e2, bool e3, bool e4);
	double getXmax();
	double getXmin();
	double getYmax();
	double getYmin();
	bool getE1();
	bool getE2();
	bool getE3();
	bool getE4();
	void setE1(bool inBool);
	void setE2(bool inBool);
	void setE3(bool inBool);
	void setE4(bool inBool);
	int getNumID();
	~room();
};

