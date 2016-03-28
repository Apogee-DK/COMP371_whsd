#include "Cube.h"

Cube::Cube() {}

Cube::Cube(glm::vec3 center, float s, string t) 
	: center_position_object(center), size(s), type(t) {}


float Cube::getSize(){
	return size;
}

glm::vec3 Cube::getCenter(){
	return center_position_object;
}

string Cube::getType(){
	return type;
}