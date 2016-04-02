#ifndef SCENE_H
#define SCENE_H

#include "glm.hpp"
#include "glfw3.h" 
#include "Camera.h"
#include "Character.h"
#include "Cursor.h"
using namespace std;

class Scene{
private:
	//Setting up the map dimensions
	
	Camera scene_camera;
	Character scene_character;
	Cursor scene_cursor;
	glm::vec3 previous_camera_position;

	int length_map;
	int width_map;		
	int numOfTrees;
	int numOfHouses;
	int numOfHills;
	float size_cube;

	//Boundaries of the map
	float min_x = -0.1f;
	float min_y = -0.1f;
	float max_z = 0.1f;
	
	float max_x;
	float max_y;
	float min_z;


public:

	Scene();

	//Set the size of map and the size of the cubes
	Scene(Camera, Character, Cursor, int, int, float);

	void update();

	void setSceneWidth(int);

	void setSceneLength(int);

	void setSceneCubeSize(float);

	//Set objects functions
	void setNumberOfTrees(int);

	void setNumberOfHouses(int);

	void setNumberOfHills(int);

	void setSceneCamera(Camera);

	void setPreviousCameraPosition(glm::vec3);
	
	glm::vec3 getMinBoundaryMap();

	glm::vec3 getMaxBoundaryMap();

	int getSceneWidth();

	int getSceneLength();

	Camera& getSceneCamera();

	Character& getSceneCharacter();

	Cursor& getSceneCursor();

	glm::vec3 Scene::getPreviousSceneCameraPosition();


};
#endif