#ifndef CHARACTER_H
#define CHARACTER_H

#include "glm.hpp"
#include "glfw3.h"
#include "Cube.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Character{
private:
	
	string name;

	int currentTypeCube; //the type of cube the character is holding
	int currentTypeTool; //If we decide to add tool/weapons

	vector<Cube> inventory_cube[20]; //assuming there's only 10 types of cubes at the moment


public:

	Character();

	Character(string);

	//set the cube which the character is holding
	void setHoldingTypeCube(int);

	//set the tool which the character is holding
	void setHoldingTypeTool(int);

	//add a cube to a vector
	void addCubeToInventory(Cube);

	void removeCubeFromInventory(int);

	//get the quantity of cube for each type from the vector
	int getCubeQuantity(int);

	int getHoldingTypeCube();

	int getHoldingTypeTool();
};
#endif