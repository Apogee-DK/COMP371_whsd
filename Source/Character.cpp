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

void Character::removeCubeFromInventory(int type){
	//remove the first cube in the vector
	inventory_cube[type].erase(inventory_cube[type].begin());
}

int Character::getCubeQuantity(int type){

	return inventory_cube[type].size();
}

int Character::getHoldingTypeCube(){
	return currentTypeCube;
}

int Character::getHoldingTypeTool(){
	return currentTypeTool;
}