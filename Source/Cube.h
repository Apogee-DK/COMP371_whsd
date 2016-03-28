#ifndef CUBE_H
#define CUBE_H

#include "glm.hpp"

using namespace std;

class Cube{

private:
	glm::vec3 center_position_object;
	float width, length, depth;


public:
	
	Cube();
	Cube(glm::vec3, float);

	glm::vec3 getCenter();
	float getWidth();
	float getLength();
	float getDepth();

};

#endif