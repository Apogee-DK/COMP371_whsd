#include "Cube.h"

Cube::Cube() {}

Cube::Cube(glm::vec3 center, float s) 
	: center_position_object(center), width(s), length(s), depth(s) {}

float Cube::getWidth(){
	return width;
}

float Cube::getLength(){
	return length;
}

float Cube::getDepth(){
	return depth;
}

glm::vec3 Cube::getCenter(){
	return center_position_object;
}