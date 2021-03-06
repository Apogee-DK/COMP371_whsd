#include "Camera.h"

Camera::Camera() {
	cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	yaw = -90.0f;
	pitch = 0.0f;
	radius = 0.05;

}

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, GLfloat y, GLfloat p, double r) 
	: cameraPos(position), cameraFront(front), cameraUp(up), yaw(y), pitch(p), radius(r) {}

void Camera::setCameraPosition(glm::vec3 v){
	cameraPos = v;
}

void Camera::setCameraFront(glm::vec3 v){
	cameraFront = v;
}

void Camera::setCameraUp(glm::vec3 v){
	cameraUp = v;
}

void Camera::setYaw(GLfloat y){
	yaw = y;
}

void Camera::setPitch(GLfloat p){
	pitch = p;
}

void Camera::setRadius(double r){
	radius = r;
}

glm::vec3 Camera::getCameraPosition(){
	return cameraPos;
}

glm::vec3 Camera::getCameraFront(){
	return cameraFront;
}

glm::vec3 Camera::getCameraUp(){
	return cameraUp;
}

GLfloat Camera::getYaw(){
	return yaw;
}

GLfloat Camera::getPitch(){
	return pitch;
}

double Camera::getRadius(){
	return radius;
}

void Camera::update(glm::vec3 cam_front, GLfloat y, GLfloat p){

	cameraFront = cam_front;
	
	yaw = y;
	
	pitch = p;

}