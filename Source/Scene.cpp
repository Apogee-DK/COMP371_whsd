#include "Scene.h"
#include <algorithm>

Scene::Scene(){}

//Set the size of map and the size of the cubes
Scene::Scene(int l, int w, float s) 
	: length_map(l), width_map(w), size_cube(s) {

	max_x = l*s + min_x;
	max_y = max(l*s, w*s) + min_y;
	min_z = -1*w*s + max_z; //negative number

}

//Set objects functions
void Scene::setNumberOfTrees(int n){
	numOfTrees = n;
}

void Scene::setNumberOfHouses(int n){
	numOfHouses = n;
}

glm::vec3 Scene::getMinBoundaryMap(){
	return glm::vec3(min_x, min_y, max_z);
}

glm::vec3 Scene::getMaxBoundaryMap(){
	return glm::vec3(max_x, max_y, min_z);
}