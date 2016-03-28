#include "Cube.h"

Cube::Cube() {}

Cube::Cube(glm::vec3 center, float s, int t) 
	: center_position_object(center), size(s), type(t) {}


float Cube::getSize(){
	return size;
}

glm::vec3 Cube::getCenter(){
	return center_position_object;
}

int Cube::getType(){
	return type;
}