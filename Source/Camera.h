#ifndef CAMERA_H
#define CAMERA_H

#include "glfw3.h"       // include GLFW helper library
#include "glm.hpp"

class Camera{
private:

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	GLfloat yaw;
	GLfloat pitch;
	double radius;
	

public:
	Camera();
	Camera(glm::vec3, glm::vec3, glm::vec3, GLfloat, GLfloat, double);

	void setCameraPosition(glm::vec3);
	void setCameraFront(glm::vec3);
	void setCameraUp(glm::vec3);

	void setYaw(GLfloat);
	void setPitch(GLfloat);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraFront();
	glm::vec3 getCameraUp();

	void setRadius(double);
	double getRadius();

	GLfloat getYaw();
	GLfloat getPitch();

	void update(glm::vec3, glm::vec3, glm::vec3, GLfloat, GLfloat);
};
#endif