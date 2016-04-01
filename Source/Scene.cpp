#include "Scene.h"
#include <algorithm>

Scene::Scene(){
	length_map = 0;
	width_map = 0;
	size_cube = 0;
}

//Set the size of map and the size of the cubes
Scene::Scene(Camera c, Character ch, int l, int w, float s) 
	: scene_camera(c), scene_character(ch), length_map(l), width_map(w), size_cube(s) {

	max_x = l*s + min_x;
	max_y = max(l*s/2, w*s/2) + min_y;
	min_z = -1*w*s + max_z; //negative number

}

void Scene::update(){

	if (size_cube > 0 && length_map > 0 && width_map > 0){
		max_x = length_map*size_cube + min_x;
		max_y = max(length_map*size_cube, width_map*size_cube) + min_y;
		min_z = -1 * width_map*size_cube + max_z; //negative number
	}

}

void Scene::setSceneWidth(int w){
	if (width_map == 0)
		width_map = w;
}

void Scene::setSceneLength(int l){
	if (length_map == 0)
		length_map = l;
}

void Scene::setSceneCubeSize(float s){
	if (size_cube == 0)
		 size_cube = s;
}

//Set objects functions
void Scene::setNumberOfTrees(int n){
	numOfTrees = n;
}

void Scene::setNumberOfHouses(int n){
	numOfHouses = n;
}

void Scene::setNumberOfHills(int n){
	numOfHills = n;
}

void Scene::setSceneCamera(Camera c){
	scene_camera = c;
}

void Scene::setPreviousCameraPosition(glm::vec3 v){
	previous_camera_position = v;
}

glm::vec3 Scene::getMinBoundaryMap(){
	return glm::vec3(min_x, min_y, max_z);
}

glm::vec3 Scene::getMaxBoundaryMap(){
	return glm::vec3(max_x, max_y, min_z);
}

int Scene::getSceneWidth(){
	return width_map;
}

int Scene::getSceneLength(){
	return length_map;
}

Camera& Scene::getSceneCamera(){
	return scene_camera;
}

Character& Scene::getSceneCharacter(){
	return scene_character;
}

glm::vec3 Scene::getPreviousSceneCameraPosition(){
	return previous_camera_position;
}