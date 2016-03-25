#include "glew.h"		// include GL Extension Wrangler

#include "glfw3.h"  // include GLFW helper library

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include "SOIL.h"
using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

//--------------------------------------------------------------------------------------------------------------------------------------------
//PROTOTYPE FUNCTIONS AND VARIABLES
//--------------------------------------------------------------------------------------------------------------------------------------------
GLfloat skyboxVertices[] = {
	// Positions          
	-3.0f, 3.0f, -3.0f,
	-3.0f, -3.0f, -3.0f,
	3.0f, -3.0f, -3.0f,
	3.0f, -3.0f, -3.0f,
	3.0f, 3.0f, -3.0f,
	-3.0f, 3.0f, -3.0f,

	-3.0f, -3.0f, 3.0f,
	-3.0f, -3.0f, -3.0f,
	-3.0f, 3.0f, -3.0f,
	-3.0f, 3.0f, -3.0f,
	-3.0f, 3.0f, 3.0f,
	-3.0f, -3.0f, 3.0f,

	3.0f, -3.0f, -3.0f,
	3.0f, -3.0f, 3.0f,
	3.0f, 3.0f, 3.0f,
	3.0f, 3.0f, 3.0f,
	3.0f, 3.0f, -3.0f,
	3.0f, -3.0f, -3.0f,

	-3.0f, -3.0f, 3.0f,
	-3.0f, 3.0f, 3.0f,
	3.0f, 3.0f, 3.0f,
	3.0f, 3.0f, 3.0f,
	3.0f, -3.0f, 3.0f,
	-3.0f, -3.0f, 3.0f,

	-3.0f, 3.0f, -3.0f,
	3.0f, 3.0f, -3.0f,
	3.0f, 3.0f, 3.0f,
	3.0f, 3.0f, 3.0f,
	-3.0f, 3.0f, 3.0f,
	-3.0f, 3.0f, -3.0f,

	-3.0f, -3.0f, -3.0f,
	-3.0f, -3.0f, 3.0f,
	3.0f, -3.0f, -3.0f,
	3.0f, -3.0f, -3.0f,
	-3.0f, -3.0f, 3.0f,
	3.0f, -3.0f, 3.0f
};
GLFWwindow* window = 0x00;

GLuint shader_program = 0;
GLuint skyboxShader_program = 0;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;

///Transformations
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 model_matrix;

GLuint VBO, VAO, EBO;
GLuint skyboxVAO, skyboxVBO;
GLfloat point_size = 3.0f;

//for the points
vector<float> dir_translation = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.1f, 0.0f
	};

//for the points
vector<GLfloat> inputPoints_2;

//for the coordinates of objects in our scene
static vector<GLfloat> g_vertex_buffer_data;

//for the ebo, in order to draw the shap
static vector<GLuint> indicesOfPoints;

//window size
int width, height;

//Prototype function for key inputs
void key_callback(GLFWwindow*, int, int, int, int);

//--------------------------------------------------------------------------------------------------------------------------------------------
//INITIALIZATION AND DELETION
//--------------------------------------------------------------------------------------------------------------------------------------------

bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(1000, 1000, "COMP371: Minecraft", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	
	glfwGetWindowSize(window, &width, &height);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);

	/// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	///Needed to get the latest version of OpenGL
	glewInit();

	/// Get the current OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	/// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); /// Enable depth-testing
	glDepthFunc(GL_LESS);	/// The type of testing i.e. a smaller value as "closer"

	//INITIALIZING THE PROJECTION MATRIX AND THE VIEW MATRIX
	proj_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f); //0.1 units <-> 100 units, clipping
	//SETTING THE VIEW MATRIX
	view_matrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f),		//Position of the camera
		glm::vec3(0.0f, 0.0f, 0.0f),		//Target of the camera
		glm::vec3(0.0f, 1.0f, 0.0f)		//Direction of the camera
		);

	return true;
}

bool cleanUp() {
	glDisableVertexAttribArray(0);
	//Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//SETUP SHADERS
//--------------------------------------------------------------------------------------------------------------------------------------------

GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path)	{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	//appearing in the vertex shader.
	glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	view_matrix_id = glGetUniformLocation(ProgramID, "view_matrix");
	model_matrix_id = glGetUniformLocation(ProgramID, "model_matrix");
	proj_matrix_id = glGetUniformLocation(ProgramID, "proj_matrix");

	return ProgramID;
}
//load a cubeMap (for example skybox)
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}
//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATING CUBES
//--------------------------------------------------------------------------------------------------------------------------------------------

//v1 & v2 ARE VECTORS THAT WERE TAKEN FROM THE INPUT FILES
//bufferData IS THE VECTOR THAT HOLDS THE RESULTING TRANSLATION USING v1 & v2
//indicesOfPoints IS THE MATRIX OF indicesOfPoints, EACH POINT HAS AN INDEX
void translationSweepMatrix(vector<GLfloat> v1, vector<GLfloat> v2, vector<GLfloat> *bufferData, vector<GLuint> *indicesOfPoints) {

	//FOR EACH POINT IN THE SECOND VECTOR, ADD THE FIRST VECTOR TO IT --> x1, y1, y1 + x2, y2, z2 --> New point --> STORE NEW POINT IN bufferData
	for (int i = 0; i < v2.size() / 3; i++) {
		for (int j = 0; j < v1.size(); j++) {
			bufferData->push_back(v1[j] + v2[(i * 3 + j % 3)]);
		}
	}

	//Setting up each cube
	for (int i = 0; i < bufferData->size()/3; i+=8) {

		//0 1 3
		indicesOfPoints->push_back(i);
		indicesOfPoints->push_back(i + 1);
		indicesOfPoints->push_back(i + 3);

		//0 3 2
		indicesOfPoints->push_back(i);
		indicesOfPoints->push_back(i + 3);
		indicesOfPoints->push_back(i + 2);


		//2 3 7
		indicesOfPoints->push_back(i + 2);
		indicesOfPoints->push_back(i + 3);
		indicesOfPoints->push_back(i + 7);

		//2 7 6
		indicesOfPoints->push_back(i + 2);
		indicesOfPoints->push_back(i + 7);
		indicesOfPoints->push_back(i + 6);


		//6 7 5
		indicesOfPoints->push_back(i + 6);
		indicesOfPoints->push_back(i + 7);
		indicesOfPoints->push_back(i + 5);


		//6 5 4
		indicesOfPoints->push_back(i + 6);
		indicesOfPoints->push_back(i + 5);
		indicesOfPoints->push_back(i + 4);

		//4 5 1
		indicesOfPoints->push_back(i + 4);
		indicesOfPoints->push_back(i + 5);
		indicesOfPoints->push_back(i + 1);
		
		//4 1 0
		indicesOfPoints->push_back(i + 4);
		indicesOfPoints->push_back(i + 1);
		indicesOfPoints->push_back(i);

		//0 2 4
		indicesOfPoints->push_back(i);
		indicesOfPoints->push_back(i + 2);
		indicesOfPoints->push_back(i + 4);

		//4 0 2
		indicesOfPoints->push_back(i + 4);
		indicesOfPoints->push_back(i + 0);
		indicesOfPoints->push_back(i + 2);

		//4 2 6
		indicesOfPoints->push_back(i + 4);
		indicesOfPoints->push_back(i + 2);
		indicesOfPoints->push_back(i + 6);

		//4 2 6
		indicesOfPoints->push_back(i + 4);
		indicesOfPoints->push_back(i + 2);
		indicesOfPoints->push_back(i + 6);

		//1 5 7
		indicesOfPoints->push_back(i + 1);
		indicesOfPoints->push_back(i + 5);
		indicesOfPoints->push_back(i + 7);

		//1 7 3
		indicesOfPoints->push_back(i + 1);
		indicesOfPoints->push_back(i + 7);
		indicesOfPoints->push_back(i + 3);		
	}



}

void setTranslationDirection(float size){
	dir_translation[4] = size;
}

//create a square
//The parameter x, y, z is the starting coordinate of the square
void createCube(float x, float y, float z, float size) {

	setTranslationDirection(size);

	//The shape being translated
	inputPoints_2.push_back(x + size);
	inputPoints_2.push_back(y);
	inputPoints_2.push_back(z);	
	
	inputPoints_2.push_back(x + size);
	inputPoints_2.push_back(y);
	inputPoints_2.push_back(z + size);	
	
	inputPoints_2.push_back(x);
	inputPoints_2.push_back(y);
	inputPoints_2.push_back(z);

	inputPoints_2.push_back(x);
	inputPoints_2.push_back(y);
	inputPoints_2.push_back(z + size);


	//--------------------------------------------------------------------------------------------------------------------------------------------
	//ADD COLOR/TEXTURE HERE
	//--------------------------------------------------------------------------------------------------------------------------------------------





}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE SCENE OBJECTS
//--------------------------------------------------------------------------------------------------------------------------------------------

//Add functions to create trees, water, grass, ...

void createLeaves(glm::vec3 location, int height, float size){

	glm::vec3 obj_coordinate = location;

	//Stacking the cube onto of each other
	for (int i = 0; i < height + 1; i++){
		if (i % 2 == 0){
			createCube(obj_coordinate[0], obj_coordinate[1], obj_coordinate[2], size);
			createCube(obj_coordinate[0] + size, obj_coordinate[1], obj_coordinate[2], size);
			createCube(obj_coordinate[0] - size, obj_coordinate[1], obj_coordinate[2], size);
			createCube(obj_coordinate[0], obj_coordinate[1], obj_coordinate[2] + size, size);
			createCube(obj_coordinate[0], obj_coordinate[1], obj_coordinate[2] - size, size); //problem with negative
		}

		else{
			createCube(obj_coordinate[0], obj_coordinate[1], obj_coordinate[2], size);
			createCube(obj_coordinate[0] + size, obj_coordinate[1], obj_coordinate[2] + size, size);
			createCube(obj_coordinate[0] + size, obj_coordinate[1], obj_coordinate[2] - size, size);
			createCube(obj_coordinate[0] - size, obj_coordinate[1], obj_coordinate[2] + size, size);
			createCube(obj_coordinate[0] - size, obj_coordinate[1], obj_coordinate[2] - size, size);
		}

		//Stack leaves ontop of each other
		obj_coordinate[1] += size;
	}
}

void createTrees(glm::vec3 location, int height, float size){

	glm::vec3 obj_coordinate = location;

	//Stacking the cube onto of each other
	for (int i = 0; i < height; i++){
		createCube(obj_coordinate[0], obj_coordinate[1], obj_coordinate[2], size);

		//Stack cube ontop of each other
		obj_coordinate[1] += size;
	}

	//Create the leaves
	createLeaves(obj_coordinate, height, size);

	//CALL THE TRANSLATIONAL SWEEP
	translationSweepMatrix(dir_translation, inputPoints_2, &g_vertex_buffer_data, &indicesOfPoints);
}

void createWater(){

	


}

void createGrass(){



}


//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE MAP
//--------------------------------------------------------------------------------------------------------------------------------------------


//Add function to generate the map procedurally
void createMap(int numOfTrees, int sizeOfWater, int sizeOfMap, float sizeOfCubes){
	
	//coordinate and height will be random
	//coordinate, height, size of cubes
	createTrees(glm::vec3(0, -1, 0), 2, sizeOfCubes);


	createWater();


	createGrass();
}



//--------------------------------------------------------------------------------------------------------------------------------------------
//SETUP VERTEX OBJECTS
//--------------------------------------------------------------------------------------------------------------------------------------------

//METHOD TO GENERATE AND BIND ALL THE VERTEX AND ELEMENT OBJECTS
void setupVertexObjects() {

	//GENERATE THE ARRAY OBJECTS AND BUFFER OBJECTS
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO); //USED TO HOLD MY indicesOfPoints

	//Binding the Vertex Array Object, then we bind and set the vertex buffers
	glBindVertexArray(VAO);

	//COPYING OUR VERTICES ARRAY IN A VERTEX BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, g_vertex_buffer_data.size() * sizeof(GLfloat), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	//SETTING ATTRIBUTE POINTERS - LINKING VERTEX ATTRIBUTES
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);
	glEnableVertexAttribArray(0); //ENABLING IT

	//COPYING OUR INDEX ARRAY IN AN ELEMENT BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesOfPoints.size() * sizeof(unsigned int), &indicesOfPoints[0], GL_STATIC_DRAW);

	//UNBINDING THE VAO NOT THE EBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind VAO to prevent bugs
	// Setup skybox VAO
	
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
}



//--------------------------------------------------------------------------------------------------------------------------------------------
//MAIN FUNCTION
//--------------------------------------------------------------------------------------------------------------------------------------------

int main() {

	initialize();

	createTrees(glm::vec3(0, -1, 0), 2, 0.1);
	vector<const GLchar*> faces;
	faces.push_back("../Source/images/right.jpg");
	faces.push_back("../Source/images/left.jpg");
	faces.push_back("../Source/images/top.jpg");
	faces.push_back("../Source/images/bottom.jpg");
	faces.push_back("../Source/images/back.jpg");
	faces.push_back("../Source/images/front.jpg");
	GLuint cubemapTexture = loadCubemap(faces);
	///Load the shaders
	shader_program = loadShaders("../Source/minecraft.vs", "../Source/minecraft.fss");
	skyboxShader_program = loadShaders("../Source/skybox.vs", "../Source/skybox.fss");
	//SETUP THE VERTEX AND ELEMENT OBJECTS FOR FIRST USE
	setupVertexObjects();

	while (!glfwWindowShouldClose(window)) {

		// update other events like input handling
		glfwPollEvents();

		//FOR RESIZING THE WINDOW
		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);
		
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);
		// Draw skybox first
		glDepthMask(GL_FALSE);// Remember to turn depth writing off
		glUseProgram(skyboxShader_program);

		glUniformMatrix4fv(glGetUniformLocation(view_matrix_id, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(proj_matrix_id, "projection"), 1, GL_FALSE, glm::value_ptr(proj_matrix));
		
		//Pass the values of the three matrices to the shaders
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(glGetUniformLocation(shader_program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		glUseProgram(shader_program);

		//Pass the values of the three matrices to the shaders
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

		glBindVertexArray(VAO);

		//DRAW THE TRIANGLES USING THE EBO (ELEMENTS)
		glDrawElements(GL_TRIANGLES, indicesOfPoints.size(), GL_UNSIGNED_INT, (void*)0);

		glBindVertexArray(0);


		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();
	return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------
//KEYBOARD INPUTS
//--------------------------------------------------------------------------------------------------------------------------------------------

//HANDLES THE KEY INPUTS
void key_callback(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	switch (key) {
		//THE FOLLOWING CASES ROTATES THE MODEL (OBJECT) DEPENDING ON CERTAIN KEY PRESSES
	case GLFW_KEY_LEFT:
		if (action != GLFW_RELEASE) {
			model_matrix = glm::rotate(model_matrix, 0.1f, glm::vec3(0, -0.5, 0));
		}
		break;
	case GLFW_KEY_RIGHT:
		if (action != GLFW_RELEASE) {
			model_matrix = glm::rotate(model_matrix, 0.1f, glm::vec3(0, 0.5, 0));
		}
		break;
	case GLFW_KEY_UP:
		if (action != GLFW_RELEASE) {
			model_matrix = glm::rotate(model_matrix, 0.1f, glm::vec3(-0.5, 0, 0));
		}
		break;
	case GLFW_KEY_DOWN:
		if (action != GLFW_RELEASE) {
			model_matrix = glm::rotate(model_matrix, 0.1f, glm::vec3(0.5, 0, 0));
		}
		break;

	case GLFW_KEY_P:
		if (action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //DRAWING MY FIGURE AS POINTS
		}
		break;

	case GLFW_KEY_W:
		if (action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //REPRESENTING MY FIGURE AS LINES (I DREW THEM AS TRIANGLES FIRST)
		}
		break;

	case GLFW_KEY_T:
		if (action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //REPRESENTING MY FIGURE AS A SOLID
		}
		break;

		//SPACE WILL ALLOW YOU TO SWITCH BETWEEN FILES - MAKE SURE THAT THE FILES ARE LOCATED IN THE SAME FOLDER
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS) {
			cleanUp();
		}
		break;

	default:
		break;
	}
	return;
}
