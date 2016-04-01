#include "Character.h"

Character::Character() {
	
}

Character::Character(string n) {
	name = n;
}

void Character::setHoldingTypeCube(int c){
	currentTypeCube = c;
}

void Character::setHoldingTypeTool(int t){
	currentTypeTool = t;
}

void Character::addCubeToInventory(Cube new_cube){
	int typeofobject = new_cube.getType();

	inventory_cube[typeofobject].push_back(new_cube);
}

int Character::getCubeQuantity(int index){

	return inventory_cube[index].size();
}

int Character::getHoldingTypeCube(){
	return currentTypeCube;
}

int Character::getHoldingTypeTool(){
	return currentTypeTool;
}