#ifndef CUBE_H
#define CUBE_H

#include "glm.hpp"
#include <string>

using namespace std;

class Cube{

private:
	glm::vec3 center_position_object;
	float size;
	string type;


public:
	
	Cube();
	Cube(glm::vec3, float, string);

	glm::vec3 getCenter();
	float getSize();
	string getType();

};

#endif