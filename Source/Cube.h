#ifndef CUBE_H
#define CUBE_H

#include "glm.hpp"

using namespace std;

class Cube{

private:
	glm::vec3 center_position_object;
	float size;
	int type;


public:
	
	Cube();
	Cube(glm::vec3, float, int);

	glm::vec3 getCenter();
	float getSize();
	int getType();

};

#endif