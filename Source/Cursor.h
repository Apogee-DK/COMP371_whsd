#ifndef CURSOR_H
#define CURSOR_H

#include "glm.hpp"
#include "glfw3.h"
#include <vector>
#include <iostream>
using namespace std;

class Cursor{
private:

	glm::vec3 center_cursor_position;

public:

	Cursor();

	Cursor(glm::vec3);

	//set the cube which the character is holding
	void updateCenterPositionCursor(glm::vec3);

	//set the tool which the character is holding
	glm::vec3 getCenterPositionCursor();

};
#endif