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


#include "Cube.h"

using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

GLFWwindow* window = 0x00;

GLuint shader_program = 0;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;


///Transformations
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 model_matrix;	

GLuint VBO, VAO, EBO;

GLfloat point_size = 3.0f;

//for the points
static vector<GLfloat> inputPoints_1;

//for the points
static vector<GLfloat> inputPoints_2;

//for the points
static vector<GLfloat> g_vertex_buffer_data;

//for the ebo
static vector<GLuint> indicesOfPoints;

int width, height;

void key_callback(GLFWwindow*, int, int, int, int);

///Handle the keyboard input
void keyPressed(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	switch (key) {
	default: break;
	}
	return;
}

bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(1000, 1000, "COMP371: Assignment 1", NULL, NULL);
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

//v1 & v2 ARE VECTORS THAT WERE TAKEN FROM THE INPUT FILES
//bufferData IS THE VECTOR THAT HOLDS THE RESULTING TRANSLATION USING v1 & v2
//indicesOfPoints IS THE MATRIX OF indicesOfPoints, EACH POINT HAS AN INDEX
void translationSweepMatrix(vector<GLfloat> v1, vector<GLfloat> v2, vector<GLfloat> *bufferData, vector<GLuint> *indicesOfPoints) {

	//KEEP TRACK OF THE NUMBER OF POINTS
	int numOfVertices = v1.size() / 3;

	//FOR EACH POINT IN THE SECOND VECTOR, ADD THE FIRST VECTOR TO IT --> x1, y1, y1 + x2, y2, z2 --> New point --> STORE NEW POINT IN bufferData
	for (int i = 0; i < v2.size() / 3; i++) {
		for (int j = 0; j < v1.size(); j++) {
			bufferData->push_back(v1[j] + v2[(i * 3 + j % 3)]);
		}
	}

	for (int i = 0; i < bufferData->size(); i++) {
		if (i%numOfVertices == numOfVertices - 1)
			continue;

		//connecting the top face and bottom face
		if (i == 0){
			//top face
			indicesOfPoints->push_back(i);
			indicesOfPoints->push_back(i + 4);
			indicesOfPoints->push_back(i + 2);
			
			indicesOfPoints->push_back(i);
			indicesOfPoints->push_back(i + 6);			
			indicesOfPoints->push_back(i + 4);

			//bottom face
			indicesOfPoints->push_back(i + 1);
			indicesOfPoints->push_back(i + 5);
			indicesOfPoints->push_back(i + 3);

			indicesOfPoints->push_back(i + 1);
			indicesOfPoints->push_back(i + 7);
			indicesOfPoints->push_back(i + 5);
			
		}

		//PUSHING ALL THE INDEXES INTO THE VECTOR
		//FOLLOWING THIS PATTERN
		//
		//0  1  2  3   4   5
		//6  7  8  9  10  11
		//
		//CONNECTING 0 > 1 > 7, 0 > 6 > 7, 1 > 2 > 8, 1 > 7 > 8
		indicesOfPoints->push_back(i);
		indicesOfPoints->push_back(i + 1);
		indicesOfPoints->push_back(i + numOfVertices + 1);

		indicesOfPoints->push_back(i);
		indicesOfPoints->push_back(i + numOfVertices);
		indicesOfPoints->push_back(i + numOfVertices + 1);

		//STOP WHEN THE CONNECTION WITH THE LAST POINT IN THE VECTOR IS MADE
		if (i + numOfVertices + 1 == bufferData->size() / 3 - 1) {
			break;
		}
	}



}

//create a square
void createSquare() {

	//from the assignment to create a hollow box
	inputPoints_1.push_back(-0.5);
	inputPoints_1.push_back(0.3);
	inputPoints_1.push_back(0.0);

	inputPoints_1.push_back(-0.5);
	inputPoints_1.push_back(-0.1);
	inputPoints_1.push_back(0.0);

	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);

	inputPoints_2.push_back(0.4);
	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);

	inputPoints_2.push_back(0.4);
	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.4);

	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.4);

	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);
	inputPoints_2.push_back(0.0);


	//CALL THE TRANSLATIONAL SWEEP
	translationSweepMatrix(inputPoints_1, inputPoints_2, &g_vertex_buffer_data, &indicesOfPoints);
}

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
}

int main() {

	initialize();

	createSquare();

	///Load the shaders
	shader_program = loadShaders("../Source/minecraft.vs", "../Source/minecraft.fss");

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
