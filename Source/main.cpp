/*
OPTIMIZATION

For createSceneCube function - add the following property:

SEPARATE THE MAP INTO SECTIONS --> PLACE THE CUBES IN EACH SECTION DEPENDING ON THEIR X and Z COORDINATES
This process will optimize the collision test since it would not have to check EVERY cube in the scene

CREATE A ARRAY OF SECTIONS OF MAPS DEPENDING ON SIZE
Each section will be 100x100 (since it seems that 100x100 map runs smoothly)

Cube* map_sections = new Cube[width_map/100][length_map/100];

map_sections[i][j] --> where "i", "j" will be the index of the section
Each section will contain cubes... 
Determine i by using the coordinates x and z


For instance, consider a 200x200 map, then we should have 4 sections of 100 x 100

1,0	|   1,1
____________

0,0	|   0,1

A cube placed at (10, 0, 10) would be in section 0,0 and a cube place at (10, 0, 110) would be in section 0,1.
We can use integers of i = x / 11 and j = z / 11 to figure out the section which corresponds to cube

Therefore, for collisionFunction, we would use the camera position to determine which section we should be looking at...

*/

/*
INTERACTION

Deletion of cubes would require us to find the "starting point" of the cube --> get the index --> erase the range of elements with the buffer vector

Addition of cubes would only require the addition of the "center" of the cube and call the createSceneCube class with its predefined type... then add it to the buffer vector.
*Must also add the cube object to the requested section

*/

/*
SHADOWS
Must include a light position

Must create Z-buffer, in order to determine if an object is in shadow

Use light ray and shadow ray to determine if the object will be lit

*/

#include "glew.h"		// include GL Extension Wrangler
#include "glfw3.h"       // include GLFW helper library
#include "SOIL.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>

#include <time.h>
#include <unordered_map>

#include "Scene.h"
#include "Cube.h"
#include "Camera.h"
#include "Character.h"

using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

static const size_t InitialFNV = 2166136261U;
static const size_t FNVMultiple = 16777619;

/* Index for the texture of cubes
-1 water
0 empty
1 ground
2 tree
3 leaf
4 house
5 roof
6 character hand
7 hill
*/

//--------------------------------------------------------------------------------------------------------------------------------------------
//HASH FUNCTION - using for other map coordinates
//--------------------------------------------------------------------------------------------------------------------------------------------

struct intHasher {

	size_t operator()(const int& i) const {

		size_t hash = InitialFNV;

		hash = (hash * FNVMultiple) ^ i;

		return hash;
	}
};

int coordinatetoint(float _x, float _y, float _z){

	int x = _x * 10.0f;
	int y = _y * 10.0f;
	int z = _z * 10.0f;

	int first_hash = ((((x + 31) * 37) + y) * 41) + z;

	return first_hash;
}

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
GLuint textureShader_program = 0;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;

///Transformations
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 character_view_matrix;
glm::mat4 model_matrix;
glm::mat4 character_model_matrix;

GLuint VBO, char_VBO, VAO, char_VAO, EBO, char_EBO;
GLuint skyboxVAO, skyboxVBO;
GLfloat point_size = 3.0f;

//for the coordinates of objects in our scene
vector<GLfloat> g_vertex_buffer_data;

//for the ebo, in order to draw the shap
vector<GLuint> obj_indicesOfPoints;

//for the points on a square in order to draw the cube
vector<GLfloat>* obj_coordinates = new vector<GLfloat>();

//for the coordinates of character
vector<GLfloat> character_vertex_buffer_data;

//for the ebo, in order to draw the character
vector<GLuint> character_indicesOfPoints;

//for the points on a square in order to draw the cube
vector<GLfloat>* character_coordinates = new vector<GLfloat>();

//for the Camera
bool freeRoam = false;
bool jumped = false;

//Certain camera properties that must be global
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
GLfloat yaw = -90.0f;	// Magnitude of how much we're looking to the left or to the right
GLfloat pitch = 0.0f;	// How much we are looking up or down

//Mouse positions
float lastY = 0, lastX = 0;

//translate all the points in obj_coordinates to get a cube
vector<float> dir_translation = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.1f, 0.0f
};

//for placement of objects in a scene
//needed for generate and create functions
unordered_map<int, int, intHasher>* map_of_coordinates = new unordered_map<int, int, intHasher>();

//window size
int width_window, height_window;

//For the key inputs
bool keys[1024];

//Mouse position
bool firstMouse = true;

//Mouse click
bool leftclick = false;
bool rightclick = false;

//Cursor
bool cursor_hidden = false;

//To smoothe out camera movement
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//Prototype function for key inputs
void key_callback(GLFWwindow*, int, int, int, int);
void mouse_callback(GLFWwindow*, double, double);
void windowResize(GLFWwindow*, int, int);
void character_actions(Scene&, vector<Cube>***, double, float);
void mouse_click(GLFWwindow*, int, int, int);

//--------------------------------------------------------------------------------------------------------------------------------------------
//MATHEMATICAL OPERATIONS
//--------------------------------------------------------------------------------------------------------------------------------------------

//Magnitude of vector
float findMagnitude(glm::vec3 firstV, glm::vec3 secondV) {

	glm::vec3 resultV = secondV - firstV;
	return sqrt(pow(resultV[0], 2) + pow(resultV[1], 2) + pow(resultV[2], 2));
}

float dotProduct(glm::vec3 v1, glm::vec3 v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
//Determine the angle between two vectors
float theta(glm::vec3 v1, glm::vec3 v2) {

	float dotProd = dotProduct(v1, v2);
	double magnitude = findMagnitude(glm::vec3(0, 0, 0), v1) * findMagnitude(glm::vec3(0, 0, 0), v2);

	return acos(dotProd / magnitude);

}

glm::vec3 crossProduct(glm::vec3 v1, glm::vec3 v2){

	return glm::vec3(v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]);

}
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


	glfwGetWindowSize(window, &width_window, &height_window);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, key_callback);
	//Register the mouse cursor position
	glfwSetCursorPosCallback(window, mouse_callback);
	//Register mouse click
	glfwSetMouseButtonCallback(window, mouse_click);

	//Hide the cursor in the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Set the cursor to the middle
	glfwSetCursorPos(window, width_window / 2, height_window / 2);

	//Resizing window effect
	glfwSetWindowSizeCallback(window, windowResize);

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

	//INITIALIZING THE PROJECTION MATRIX
	proj_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.05f, 75.0f); //0.1 units <-> 100 units, clipping

	glEnable(GL_CULL_FACE);

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

	//Might need to add my own character matrix id here

	return ProgramID;
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//LOAD CUBE MAP FOR THE SKYBOX
//--------------------------------------------------------------------------------------------------------------------------------------------

//load a cubeMap (for example skybox)
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width_window, height_window;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width_window, &height_window, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width_window, height_window, 0, GL_RGB, GL_UNSIGNED_BYTE, image
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

//coordinate is the vector which contains all the points
//dir_translation is the direction of translation
//bufferData IS THE VECTOR THAT HOLDS THE RESULTING TRANSLATION USING coordinate & dir_translation
//obj_indicesOfPoints IS THE MATRIX OF obj_indicesOfPoints, EACH POINT HAS AN INDEX
void translationSweepMatrix(vector<GLfloat> dir_trans, vector<GLfloat>* obj_coord, vector<GLfloat> *bufferData, vector<GLuint> *obj_indicesOfPoints) {

	//FOR EACH POINT IN THE SECOND VECTOR, ADD THE FIRST VECTOR TO IT --> x1, y1, y1 + x2, y2, z2 --> New point --> STORE NEW POINT IN bufferData
	for (int i = 0; i < obj_coord->size() / 3; i++) {
		for (int j = 0; j < dir_trans.size(); j++) {
			bufferData->push_back(dir_trans[j] + (*obj_coord)[(i * 3 + j % 3)]); //8 points for the cube
		}
	}

	//Relation between type cube
	//For the first 4 points to make the cube, we added the type
	//--------------------------------------------------------------------------------------------------------------------------------------------
	//ADD COLOR/TEXTURE HERE
	//--------------------------------------------------------------------------------------------------------------------------------------------

	//Every shape will be associated with a color, add the color "coordinate" to the inputPoints
	/*

	Steps for reading coordinates

	For the first cube
	-------------------------------------------------------
	typeOfObjects <-- water (index 0)

	0 0 0 
	0 1 0	Coordinates will go into bufferData (our VBO) ---> translate all these points to create the cube --> insert translated points into VBO (in total you would get 8 points (8 (x, y, z) coordinates)
	0 0 1
	1 0 0
	-------------------------------------------------------

	This is our first square which will be transformed into a cube
	The same process will happen for all other cubes...

	typeOfObjects (index 1) <-- ground (index 1)

	What we should do is...

	Loop through the coordinates in the buffer data to determine and set the texture of the cubes 
	
	--> index(0) was water --> the first 8 coordinates will be used to create the water texture

	func(....){

	//Adding 8 to i each time to get to the other cube
	for(int i = 0; i < bufferData.size(); i += 8){
		
		//8 * 3 
		
		//Face 1

		//Find the points that make up the face
		bufferData[i] --> do your thing;
		bufferData[i + something] --> do your thing;
		bufferData[i + ...] --> do your thing;
		bufferData[i + ...] --> do your thing;

		set your image

		//Face 2


		//Face 3



		//Face 4

		...

		//Face 6

		...

	}

	--> index(1) was ground --> the next 8 coordinates will be used to create the water texture (by calling the func(...))


	The pattern should be: 
	*/


	//Setting up each cube
	//Starting at i = 1 because i = 0 is the type of cube
	for (int i = 0; i < bufferData->size() / 3; i += 8) {

		//0 1 3
		obj_indicesOfPoints->push_back(i);
		obj_indicesOfPoints->push_back(i + 1);
		obj_indicesOfPoints->push_back(i + 3);

		//0 3 2
		obj_indicesOfPoints->push_back(i);
		obj_indicesOfPoints->push_back(i + 3);
		obj_indicesOfPoints->push_back(i + 2);


		//2 3 7
		obj_indicesOfPoints->push_back(i + 2);
		obj_indicesOfPoints->push_back(i + 3);
		obj_indicesOfPoints->push_back(i + 7);

		//2 7 6
		obj_indicesOfPoints->push_back(i + 2);
		obj_indicesOfPoints->push_back(i + 7);
		obj_indicesOfPoints->push_back(i + 6);


		//6 7 5
		obj_indicesOfPoints->push_back(i + 6);
		obj_indicesOfPoints->push_back(i + 7);
		obj_indicesOfPoints->push_back(i + 5);


		//6 5 4
		obj_indicesOfPoints->push_back(i + 6);
		obj_indicesOfPoints->push_back(i + 5);
		obj_indicesOfPoints->push_back(i + 4);

		//4 5 1
		obj_indicesOfPoints->push_back(i + 4);
		obj_indicesOfPoints->push_back(i + 5);
		obj_indicesOfPoints->push_back(i + 1);

		//4 1 0
		obj_indicesOfPoints->push_back(i + 4);
		obj_indicesOfPoints->push_back(i + 1);
		obj_indicesOfPoints->push_back(i);

		//0 2 4
		obj_indicesOfPoints->push_back(i);
		obj_indicesOfPoints->push_back(i + 2);
		obj_indicesOfPoints->push_back(i + 4);

		//4 0 2
		obj_indicesOfPoints->push_back(i + 4);
		obj_indicesOfPoints->push_back(i + 0);
		obj_indicesOfPoints->push_back(i + 2);

		//4 2 6
		obj_indicesOfPoints->push_back(i + 4);
		obj_indicesOfPoints->push_back(i + 2);
		obj_indicesOfPoints->push_back(i + 6);

		//4 2 6
		obj_indicesOfPoints->push_back(i + 4);
		obj_indicesOfPoints->push_back(i + 2);
		obj_indicesOfPoints->push_back(i + 6);

		//1 5 7
		obj_indicesOfPoints->push_back(i + 1);
		obj_indicesOfPoints->push_back(i + 5);
		obj_indicesOfPoints->push_back(i + 7);

		//1 7 3
		obj_indicesOfPoints->push_back(i + 1);
		obj_indicesOfPoints->push_back(i + 7);
		obj_indicesOfPoints->push_back(i + 3);
	}
}

//Set the translating vector
void setTranslationDirection(float sizeOfCube){
	dir_translation[4] = sizeOfCube;
}

//create a square
//Parameter x, y, z is the starting coordinate of the square
//Type is for water (-1) or ground/objects(1)
void createSceneCube(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, float x, float y, float z, float sizeOfCube, int type) {

	//Needed for collisions
	//scene_cube_objects.push_back(Cube(glm::vec3(x, y, z), sizeOfCube, type)); //create anonymous cube objecct, push cube into vector

	int coord_x = x / 11;
	int coord_z = -1*(z) / 11;

	//Pushing the objects into different sections
	map_section[coord_x][coord_z]->push_back(Cube(glm::vec3(x, y, z), sizeOfCube, type)); //assigning the cube to the map_section


	//Each coordinate will be a key to a type
	//Using this because it is much faster to hash the string coordinate and access it later (to check if an object can be placed in that location)
	
	if (type == -1)
		(*map_of_coordinates)[coordinatetoint(x, y, z)] = type;

	setTranslationDirection(sizeOfCube); //Depending on the size of the cube, we will need to change the translating vector

	//Store the points in obj_coordinates which will be used to create cubes
	obj_coordinates->push_back(x + sizeOfCube / 2);
	obj_coordinates->push_back(y - sizeOfCube / 2);
	obj_coordinates->push_back(z - sizeOfCube / 2);
	
	obj_coordinates->push_back(x + sizeOfCube / 2);
	obj_coordinates->push_back(y - sizeOfCube / 2);
	obj_coordinates->push_back(z + sizeOfCube / 2);

	obj_coordinates->push_back(x - sizeOfCube / 2);
	obj_coordinates->push_back(y - sizeOfCube / 2);
	obj_coordinates->push_back(z - sizeOfCube / 2);

	obj_coordinates->push_back(x - sizeOfCube / 2);
	obj_coordinates->push_back(y - sizeOfCube / 2);
	obj_coordinates->push_back(z + sizeOfCube / 2);

}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE LAND
//--------------------------------------------------------------------------------------------------------------------------------------------

/**
In case we need a function to create a body of water...

Function to create a body water in a scene
- location_water: starting coordinate of the ground (first cube being placed in the scene)
- width_water: width dimension of our viewable scene
- length_water: length dimension of our viewable scene
- sizeOfCube: fixed dimension of a single cube, value provided by createTree
*/
void createWater(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_water, int width_water, int length_water, float sizeOfCube){

	//Fill up map space with water depending on the width_water and the length_ground
	for (int i = 0; i < width_water; i++){
		for (int j = 0; j < length_water; j++){
			//add another parameter for color
			createSceneCube(obj_coordinates, map_section, location_water[0] + i*sizeOfCube, location_water[1], location_water[2] - j*sizeOfCube, sizeOfCube, -1); //-1 is for water, needed to see if an object can be placed on top --> if it's water, then you can't
		}
	}
}

/**
Function to create the ground in a scene
- location_ground: starting coordinate of the ground (first cube being placed in the scene)
- width_ground: width dimension of our viewable scene
- length_ground: length dimension of our viewable scene
- sizeOfCube: fixed dimension of a single cube
- _hasWater: boolean value which tells us if there's an area within the map which is covered by water
*/
void createGround(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_ground, int width_ground, int length_ground, float sizeOfCube, bool _hasWater){

	int water_width = 0, water_length = 0;
	int water_StartPosX = 0; int water_StartPosZ = 0;

	//Randomize the dimension of the body of water and the starting position of the water
	if (_hasWater){
		//Scene will contain water
		//Dimensions
		water_width = (rand() % width_ground) / 10;
		water_length = (rand() % length_ground) / 10;
		//Starting position
		water_StartPosX = (rand() % (length_ground - water_length)) / 10;
		water_StartPosZ = (rand() % (width_ground - water_width)) / 10;
	}

	//Set a temp length 
	int temp_w_length = water_length; //must decrease it in the loop and get the previous value

	//Boolean values which will be used to know when to start adding the region with water
	bool atZ = false;
	bool atX = false;


	//Fill up map space with ground depending on the width_ground and the length_ground
	for (int i = 0; i < width_ground; i++){

		//Check if we arrived at the z position for the body of water
		if (water_StartPosZ == i){
			atZ = true;
		}

		temp_w_length = water_length; //reset the temp_w_length to the value which we had before

		for (int j = 0; j < length_ground; j++){

			//Check if we arrived at the x position for the body of water
			if (water_StartPosX == i){
				atX = true;
			}

			//Add the water cube if we arrived at the starting location of our body of water and we have not reached the maximum amount of water cubes
			if (atZ && atX && water_width > 0 && temp_w_length > 0){
				createSceneCube(obj_coordinates, map_section, location_ground[0] + i*sizeOfCube, location_ground[1], location_ground[2] - j*sizeOfCube, sizeOfCube, -1); //-1 for water
				temp_w_length--; //decrease the length
			}

			//Generate a ground cube if it is not a water cube
			else{
				createSceneCube(obj_coordinates, map_section, location_ground[0] + i*sizeOfCube, location_ground[1], location_ground[2] - j*sizeOfCube, sizeOfCube, 1); //1 for ground
			}
		}
		water_width--; //decrease the width of the body of water
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE HILLS
//--------------------------------------------------------------------------------------------------------------------------------------------
void createHill(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_hill, int height_hill, int width_hill, int length_hill, float sizeOfCube){

	glm::vec3 coordinate = location_hill;
	int type = 7;

	for (int h = 0; h < height_hill; h++){

		if (width_hill <= 2 || length_hill <= 2){
			break;
		}

		if (h == 0){
			//Setting the base area which is taken up by the hill
			for (int w1 = 0; w1 < width_hill + 2; w1++){
				for (int l1 = 0; l1 < length_hill + 2; l1++){
					(*map_of_coordinates)[coordinatetoint(coordinate[0] + l1*sizeOfCube, coordinate[1], coordinate[2] - w1*sizeOfCube)] = type;
				}
			}
		}

		//Assign the position of the cubes
		for (int w = 0; w < width_hill; w++){
			for (int l = 0; l < length_hill; l++){
				createSceneCube(obj_coordinates, map_section, coordinate[0] + l*sizeOfCube, coordinate[1] + h*sizeOfCube, coordinate[2] - w*sizeOfCube, sizeOfCube, type);
			}
		}

		coordinate[0] += sizeOfCube;
		coordinate[2] -= sizeOfCube;

		//I could random this
		width_hill -= 3;
		length_hill -= 3;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE TREE FUNCTIONS
//--------------------------------------------------------------------------------------------------------------------------------------------

/**
Function to create a leaves on a tree
- location_leaf: starting coordinate of the leaf (right on top of the tree cube)
- height_leaf: height dimension of the leaves (which is equal to the height of the tree), value provided by createTree
- sizeOfCube: fixed dimension of a single cube, value provided by createTree
*/
void createLeaves(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_leaf, int height_leaf, float sizeOfCube){

	glm::vec3 coordinate = location_leaf;
	int type = 3;

	//Stacking the cube onto of each other

	for (int i = 0; i < height_leaf + 1; i++){

		//Have an alternating effect for the leaves

		// X
		//XXX
		// X
		if (i % 2 == 0){
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2] + sizeOfCube, sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] - sizeOfCube, coordinate[1], coordinate[2], sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2], sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] + sizeOfCube, coordinate[1], coordinate[2], sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2] - sizeOfCube, sizeOfCube, type);
		}

		//X X
		// X
		//X X
		else{
			createSceneCube(obj_coordinates, map_section, coordinate[0] - sizeOfCube, coordinate[1], coordinate[2] + sizeOfCube, sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] + sizeOfCube, coordinate[1], coordinate[2] + sizeOfCube, sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2], sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] - sizeOfCube, coordinate[1], coordinate[2] - sizeOfCube, sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] + sizeOfCube, coordinate[1], coordinate[2] - sizeOfCube, sizeOfCube, type);
		}

		//Stack leaves ontop of each other
		coordinate[1] += sizeOfCube;

		//Add an additional leaf (cube) at the top
		if (i == height_leaf){
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2], sizeOfCube, type);
		}
	}
}

/**
Function to create a tree on the map
- location_tree: starting coordinate of the tree (where the first cube is placed)
- height_tree: height dimension of the roof, value provided by the function which called it
- sizeOfCube: fixed dimension of a single cube, value provided by the function which called it

A tree takes up a 3 x 3 square --> make sure other trees do not overlap
*/
void createTrees(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_tree, int height_tree, float sizeOfCube){

	glm::vec3 coordinate = location_tree;
	int type = 2;
	
	//Only consider base of the tree which takes up the area
	(*map_of_coordinates)[coordinatetoint(coordinate[0] - sizeOfCube, coordinate[1], coordinate[2] + sizeOfCube)] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0], coordinate[1], coordinate[2] + sizeOfCube)] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0] + sizeOfCube, coordinate[1], coordinate[2] + sizeOfCube)] = type;

	(*map_of_coordinates)[coordinatetoint(coordinate[0] - sizeOfCube, coordinate[1], coordinate[2])] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0], coordinate[1], coordinate[2])] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0] + sizeOfCube, coordinate[1], coordinate[2])] = type;

	(*map_of_coordinates)[coordinatetoint(coordinate[0] - sizeOfCube, coordinate[1], coordinate[2] - sizeOfCube)] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0], coordinate[1], coordinate[2])] = type;
	(*map_of_coordinates)[coordinatetoint(coordinate[0] + sizeOfCube, coordinate[1], coordinate[2] - sizeOfCube)] = type;

	//Stacking the cube onto of each other
	for (int i = 0; i < height_tree; i++){
		createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2], sizeOfCube, type);

		//Stack cube ontop of each other
		coordinate[1] += sizeOfCube; //y coordinate
	}

	//Create the leaves
	createLeaves(obj_coordinates, map_section, coordinate, height_tree, sizeOfCube);
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE HOUSE FUNCTIONS
//--------------------------------------------------------------------------------------------------------------------------------------------

/**
Function to create a house on the map
- Location_roof: starting coordinate of the house (where the first cube is placed)
- height_roof: height dimension of the roof, value provided by the function createHouse
- width_roof: width dimension of the roof, value provided by the function createHouse
- length_roof: length dimension of the roof, value provided by the function createHouse
- sizeOfCube: fixed dimension of a single cube, value provided by the function createHouse
*/
void createRoof(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_roof, int height_roof, int width_roof, int length_roof, float sizeOfCube){

	glm::vec3 coordinate = location_roof;

	//Stacking the cube onto of each other 
	//And reducing the size as the height increases to create a pyramid
	for (int i = 0; i < height_roof; i++){
		for (int j = 0; j < width_roof - 2 * i; j++){
			for (int k = 0; k < length_roof - 2 * i; k++){
				createSceneCube(obj_coordinates, map_section, coordinate[0] + k*sizeOfCube, coordinate[1] + i*sizeOfCube, coordinate[2] - j*sizeOfCube, sizeOfCube, 5);
			}
		}

		coordinate[0] += sizeOfCube;
		coordinate[2] -= sizeOfCube;

	}
}

/**
Function to create a house on the map
- Location_house: starting coordinate of the house (where the first cube is placed)
- height_house: height dimension of the house (only the walls), value provided by the function which called it (randomized)
- width_house: width dimension of the house, value provided by the function which called it (randomized)
- length_house: length dimension of the house, value provided by the function which called it (randomized)
- sizeOfCube: fixed dimension of a single cube, value provided by the function which called it (randomized)
*/
void createHouse(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_house, int height_house, int width_house, int length_house, float sizeOfCube){

	glm::vec3 coordinate = location_house; //value needed in order to create the walls of the house
	int type = 4;

	//Stacking the cube onto of each other
	//Creating the walls along the x and z axis
	for (int i = 0; i < height_house; i++){
		
		if (i == 0){			
			//Setting the base area which is taken up by the house
			for (int w1 = 0; w1 < width_house + 2; w1++){
				for (int l1 = 0; l1 < length_house + 2; l1++){
					(*map_of_coordinates)[coordinatetoint(coordinate[0] + l1*sizeOfCube, coordinate[1], coordinate[2] - w1*sizeOfCube)] = type;
				}
			}
		}

		for (int k = 0; k < length_house; k++){
			createSceneCube(obj_coordinates, map_section, coordinate[0] + k*sizeOfCube, coordinate[1], coordinate[2], sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] + k*sizeOfCube, coordinate[1], coordinate[2] - (width_house - 1)*sizeOfCube, sizeOfCube, type);
		}
		for (int j = 1; j < width_house - 1; j++){
			createSceneCube(obj_coordinates, map_section, coordinate[0], coordinate[1], coordinate[2] - j*sizeOfCube, sizeOfCube, type);
			createSceneCube(obj_coordinates, map_section, coordinate[0] + (length_house - 1)*sizeOfCube, coordinate[1], coordinate[2] - j*sizeOfCube, sizeOfCube, type);
		}

		coordinate[1] += sizeOfCube; //Go to the next level (Y-axis)
	}

	//Call the following function to generate the roof
	createRoof(obj_coordinates, map_section, coordinate, height_house, width_house, length_house, sizeOfCube);
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE CHARACTER
//--------------------------------------------------------------------------------------------------------------------------------------------
void createCharacter(vector<GLfloat>* character_coordinates, vector<Cube>*** map_section, Camera scene_camera, float sizeOfCube){

	glm::vec3 coordinate = scene_camera.getCameraPosition(); //Starting location of our sprite
	float size = sizeOfCube / 2;
	int type = 6;

	//map_of_coordinates[to_string((*character_coordinates)[0]) + to_string((*character_coordinates)[1]) + to_string((*character_coordinates)[2])] = type;

	//Create the hand
	createSceneCube(character_coordinates, map_section, coordinate[0] + 1.25f*size, coordinate[1] - size, coordinate[2] - 3 * size, size / 2, type);
	createSceneCube(character_coordinates, map_section, coordinate[0] + 1.25f*size, coordinate[1] - size / 2, coordinate[2] - 3 * size, size / 2, type);
	createSceneCube(character_coordinates, map_section, coordinate[0] + 1.25f*size, coordinate[1], coordinate[2] - 3 * size, size / 2, type);
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//CREATE MAP
//--------------------------------------------------------------------------------------------------------------------------------------------

/**
Function which whether an object can be placed at a x,y,z coordinate
- starting_position: the position of the object, value provided by the function that calls it
- height_object: width dimension of the map, value provided by the function that calls it
- width_object: width dimension of the map, value provided by the function that calls it
- length_object: length dimension of the map, value provided by the function that calls it
- sizeOfCube: fixed dimension of a single cube, value provided by the function that calls it

An ordered MAP is used for the checkEmptySpace function --> map<string, int> map_of_coordinates
In the createSceneCube function, each cube's position is converted into a string and is linked to an integer value (-1, 0, 1)

We use the the integer value to check whether the coordinate is occupied
*/
bool checkEmptySpaces(glm::vec3 starting_position, int height_object, int width_object, int length_object, float sizeOfCube){

	//Convert each coordinate into a string a check if the coordinate is already occupied
	for (int w = 0; w < width_object; w++){
		for (int l = 0; l < length_object; l++){

			//-1 is water
			if ((*map_of_coordinates)[coordinatetoint(starting_position[0] + l*sizeOfCube, starting_position[1] - sizeOfCube, starting_position[2] - w*sizeOfCube)] == -1){
				return true;
			}

			//Add restriction to boundary
			//Empty string
			if ((*map_of_coordinates)[coordinatetoint(starting_position[0] + l*sizeOfCube, starting_position[1], starting_position[2] - w*sizeOfCube)] != 0){
				return true;
			}
		}
	}

	return false;
}

void generateHillsToScene(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_ground, int numOfHills, int width_Map, int length_Map, float sizeOfCube){
	
	for (int i = 0; i < numOfHills; i++){
		//Random the height, width and length of the hill
		int height_hill = rand() % 10 + 5;
		int width_hill = rand() % 20 + 7;
		int length_hill = rand() % 20 + 8;		
		float pos_x = (int)(rand() % (length_Map - length_hill - 2) + 1) / 10.0f;
		float pos_z = (int)(rand() % (width_Map - width_hill - 2) + 1) / 10.0f; 

		while (pos_x + location_ground[0] + length_hill*sizeOfCube > length_Map*sizeOfCube || pos_z + location_ground[2] + width_hill*sizeOfCube > width_Map*sizeOfCube){
			pos_x = (int)(rand() % (length_Map - length_hill - 2) + 1) / 10.0f;
			pos_z = (int)(rand() % (width_Map - width_hill - 2) + 1) / 10.0f;
		}

		int count = 0;

		while (count < 10 && checkEmptySpaces(glm::vec3(location_ground[0] + pos_x - sizeOfCube, location_ground[1] + sizeOfCube, location_ground[2] - pos_z + sizeOfCube), height_hill, width_hill+2, length_hill+2, sizeOfCube)){
			//The section in which the house is being placed is not free
			//Random the x and z coordinate again			
			pos_x = (int)(rand() % (length_Map - length_hill) + 1) / 10.0f; //(length_Map * sizeOfCube);
			pos_z = (int)(rand() % (width_Map - width_hill) + 1) / 10.0f; //(width_Map * sizeOfCube);
			height_hill = rand() % 10 + 5;
			width_hill = rand() % 20 + 7;
			length_hill = rand() % 20 + 8;

			count++;
		}

		createHill(obj_coordinates, map_section, glm::vec3(location_ground[0] + pos_x, location_ground[1] + sizeOfCube, location_ground[2] - pos_z), height_hill, width_hill, length_hill, sizeOfCube);
	}
}

/**
Function to generate all the trees on the map
The generation of objects in a scene requires:
- Location_ground: starting coordinate of the ground (where the first cube is placed)
- width_Map: width dimension of the map, provided by the function createMap
- length_Map: length dimension of the map, provided by the function createMap
- numOfTrees: number of trees in the scene, provided by the function createMap
- sizeOfCube: fixed dimension of a single cube, provided by the function createMap
*/
void generateTreesToScene(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_ground, int numOfTrees, int width_Map, int length_Map, float sizeOfCube){

	//For each tree, random the height and its position
	for (int i = 0; i < numOfTrees; i++){

		//length_map - 2, because we do not want the tree to appear off the map
		//A tree is always 3 x 3
		float pos_x = (int)(rand() % (length_Map - 4) + 1) / 10.0f; //(length_Map * sizeOfCube);
		float pos_z = (int)(rand() % (width_Map - 4) + 1) / 10.0f; //(width_Map * sizeOfCube);

		//A tree cannot be higher than 8
		int height_tree = rand() % 5 + 4;

		int count = 0;

		//Checks if the "block" required to place the object is free
		//The height is 2*height_tree + 1 because we must account for the leaves
		//If one of the position is not free, it will loop again
		while (count < 10 && checkEmptySpaces(glm::vec3(location_ground[0] + pos_x - sizeOfCube, location_ground[1] + sizeOfCube, location_ground[2] - pos_z + sizeOfCube), 2 * height_tree + 1, 5, 5, sizeOfCube)){
			//If the tree cannot be placed at the specified coordinate

			//Random the x and z coordinate again
			pos_x = (int)(rand() % (length_Map - 4) + 1) / 10.0f; //(length_Map * sizeOfCube);
			pos_z = (int)(rand() % (width_Map - 4) + 1) / 10.0f; //(width_Map * sizeOfCube);

			count++;
		}

		//Create the tree object if all conditions are satisfied
		createTrees(obj_coordinates, map_section, glm::vec3(location_ground[0] + pos_x, location_ground[1] + sizeOfCube, location_ground[2] - pos_z), height_tree, sizeOfCube);
	}
}

/**
Function to generate all the houses on the map
The generation of objects in a scene requires:
- Location_ground: starting coordinate of the ground (where the first cube is placed)
- width_Map: width dimension of the map, provided by the function createMap
- length_Map: length dimension of the map, provided by the function createMap
- numOfHouses: number of houses in the scene, provided by the function createMap
- sizeOfCube: fixed dimension of a single cube, provided by the function createMap
*/
void generateHousesToScene(vector<GLfloat>* obj_coordinates, vector<Cube>*** map_section, glm::vec3 location_ground, int numOfHouses, int width_Map, int length_Map, float sizeOfCube){

	//For each house, random the length, height and width... and check if it can be placed at a certain position
	for (int i = 0; i < numOfHouses; i++){

		//Random the height, width and length of the house
		int height_house = rand() % 10 + 4;
		int width_house = rand() % 10 + 5;
		int length_house = rand() % 10 + 5;

		//length_map - length_house, because I don't want the house to appear off the map
		//A house width is always a random number
		//A house length is always a random number
		//_ _ _ _ _    <-- Roof
		//  _ _ _		<-- Wall
		float pos_x = (int)(rand() % (length_Map - length_house) + 1) / 10.0f; // (length_Map * sizeOfCube);
		float pos_z = (int)(rand() % (width_Map - width_house) + 1) / 10.0f; //(width_Map * sizeOfCube);

		int count = 0;

		//Checks if the "block" required to place the object is free
		//If one of the position is not free, it will loop again
		while (count < 10 && checkEmptySpaces(glm::vec3(location_ground[0] + pos_x - sizeOfCube, location_ground[1] + sizeOfCube, location_ground[2] - pos_z + sizeOfCube), height_house, width_house + 2, length_house + 2, sizeOfCube)){
			//The section in which the house is being placed is not free
			//Random the x and z coordinate again			
			pos_x = (int)(rand() % (length_Map - length_house) + 1) / 10.0f; //(length_Map * sizeOfCube);
			pos_z = (int)(rand() % (width_Map - width_house) + 1) / 10.0f; //(width_Map * sizeOfCube);
			height_house = rand() % 10 + 4;
			width_house = rand() % 10 + 5;
			length_house = rand() % 10 + 5;

			count++;
		}

		//Once we clear the condition, create the house object
		createHouse(obj_coordinates, map_section, glm::vec3(location_ground[0] + pos_x, location_ground[1] + sizeOfCube, location_ground[2] - pos_z), height_house, width_house, length_house, sizeOfCube);
	}

}

/**
Function to generate the map procedurally
The generation of objects in a scene requires:
- Location_ground: starting coordinate of the ground (where the first cube is placed)
- width_Map: width dimension of the map, provided by user
- length_Map: length dimension of the map, provided by user
- numOfTrees: number of trees in the scene, value provided by user
- numOfHouses: number of houses in the scene, value provided by user
- sizeOfCube: fixed dimension of a single cube, value provided by user
*/
void createMap(Camera scene_camera, glm::vec3 location_ground, vector<Cube>*** map_section, int width_Map, int length_Map, int numOfTrees, int numOfHouses, int numOfHills, float sizeOfCube){

	//The position of the water section will be randomized depending on how large the surface is
	//The coordinates, width_obj and length_obj will be random

	//bool _hasWater = rand() % 2;
	createGround(obj_coordinates, map_section, location_ground, width_Map, length_Map, sizeOfCube, false);

	//Create the character and set the camera to its position
	createCharacter(character_coordinates, map_section, scene_camera, sizeOfCube);
	translationSweepMatrix(dir_translation, character_coordinates, &character_vertex_buffer_data, &character_indicesOfPoints);
	
	//Generate a certain number of hills in a scene
	generateHillsToScene(obj_coordinates, map_section, location_ground, numOfHills, width_Map, length_Map, sizeOfCube);

	//Generate a certain number of houses in a scene
	generateHousesToScene(obj_coordinates, map_section, location_ground, numOfHouses, width_Map, length_Map, sizeOfCube);

	//Generate a certain number of trees in a scene
	generateTreesToScene(obj_coordinates, map_section, location_ground, numOfTrees, width_Map, length_Map, sizeOfCube);

	//Once all objects are pushed into the vector, start translating all the coordinates to obtain the cubes
	//dir_translation is needed to create the cubes
	//obj_coordinates are all the position of every point we pushed in from the generation of our objects
	//buffer_data has all the positions
	//indices is used for the EBO
	translationSweepMatrix(dir_translation, obj_coordinates, &g_vertex_buffer_data, &obj_indicesOfPoints);

}

//--------------------------------------------------------------------------------------------------------------------------------------------
//SETUP VERTEX OBJECTS
//--------------------------------------------------------------------------------------------------------------------------------------------

//METHOD TO GENERATE AND BIND ALL THE VERTEX AND ELEMENT OBJECTS
void setupVertexObjects() {

	//GENERATE THE ARRAY OBJECTS AND BUFFER OBJECTS
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);	
	glGenBuffers(1, &EBO); //USED TO HOLD MY obj_indicesOfPoints

	//Binding the Vertex Array Object, then we bind and set the vertex buffers
	glBindVertexArray(VAO);

	//COPYING OUR VERTICES ARRAY IN A VERTEX BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, g_vertex_buffer_data.size() * sizeof(GLfloat), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	//COPYING OUR INDEX ARRAY IN AN ELEMENT BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_indicesOfPoints.size() * sizeof(unsigned int), &obj_indicesOfPoints[0], GL_STATIC_DRAW);

	//SETTING ATTRIBUTE POINTERS - LINKING VERTEX ATTRIBUTES
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,				// stride
		(void*)0			// array buffer offset
		);
	glEnableVertexAttribArray(0); //ENABLING IT
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &char_VAO);
	glBindVertexArray(char_VAO);
	glGenBuffers(1, &char_VBO);
	glGenBuffers(1, &char_EBO);
	glBindBuffer(GL_ARRAY_BUFFER, char_VBO);
	glBufferData(GL_ARRAY_BUFFER, character_vertex_buffer_data.size() * sizeof(GLfloat), &character_vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, char_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, character_indicesOfPoints.size() * sizeof(unsigned int), &character_indicesOfPoints[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0); 
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
/*These are just functions for testing - leave it here for now
//Reminder: Z coordinate is always negative
//Z can be bigger than 1
//Y must be between -1 and 0
//X can be bigger than 1
createWater(glm::vec3(0, -1.0, 0), 5, 5, 0.1);
createGround(glm::vec3(0, -1.0, 0), 100, 100, 0.1);

Testing hitbox for camera
createTrees(glm::vec3(10, -0.1, -10), 3, 0.1);
createTrees(glm::vec3(-100, -0.1, 10), 2, 0.1);
createTrees(glm::vec3(0, 0, 0), 3, 0.1);
createHouse(glm::vec3(1.5, -0.1, -20), 3, 5, 4, 0.1);
createRoof(glm::vec3(0, 0, 0), 3, 5, 4, 0.1);
createSceneCube(0.05, 0, 0.05, 0.5, "ground");
translationSweepMatrix(dir_translation, obj_coordinates, &g_vertex_buffer_data, &obj_indicesOfPoints);
*/

void updateBuffer(){

	//Binding the Vertex Array Object, then we bind and set the vertex buffers
	glBindVertexArray(VAO);

	//COPYING OUR VERTICES ARRAY IN A VERTEX BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, NULL, NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, g_vertex_buffer_data.size() * sizeof(GLfloat), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	//COPYING OUR INDEX ARRAY IN AN ELEMENT BUFFER FOR OPENGL TO USE
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NULL, NULL, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_indicesOfPoints.size() * sizeof(unsigned int), &obj_indicesOfPoints[0], GL_STATIC_DRAW);

	//SETTING ATTRIBUTE POINTERS - LINKING VERTEX ATTRIBUTES
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,				// stride
		(void*)0			// array buffer offset
		);
	glEnableVertexAttribArray(0); //ENABLING IT
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


}

int main() {

	initialize();
	srand(time(NULL));

	//Setting up the map dimensions
	float size_cube = 0.1f;
	int length_map = 100;
	int width_map = 100;
	int numOfTrees = 10;
	int numOfHouses = 5;
	int numOfHills = 1;
	
	
	Camera scene_camera;
	Character scene_character("Demo_WHSD");
	Scene scene_map(scene_camera, scene_character, length_map, width_map, size_cube);	

	//Setting the world environment
	scene_map.setNumberOfHouses(numOfHouses);
	scene_map.setNumberOfTrees(numOfTrees);
	scene_map.setNumberOfHills(numOfHills);

	int size_width_section = width_map / 100;
	int size_length_section = length_map / 100;

	//Dividing the map into sections
	//Use the map_section for 
	//All the cubes in the scene, divided into sections
	vector<Cube>*** map_section = new vector<Cube>**[size_width_section];
	for (int i = 0; i < size_width_section; i++){
		map_section[i] = new vector<Cube>*[size_length_section];

		for (int j = 0; j < size_length_section; j++){
			map_section[i][j] = new vector<Cube>();
		}
	}
	
	//Set the Y-axis to -0.3f --> Ground at -0.3f, at the moment, it is the best Y position for our camera (around eye level for our sprite)
	//View matrix is set up here
	//Starting coordinates must be positive at the moment
	createMap(scene_camera, glm::vec3(0.0f, -0.2f, 0.0f), map_section, width_map, length_map, numOfTrees, numOfHouses, numOfHills, size_cube);

	delete character_coordinates;

	scene_camera.setRadius(size_cube);
	double radius = scene_camera.getRadius();

	//Creating the vector of faces which will hold the "images" of each face of the cube
	vector<const GLchar*> faces;
	faces.push_back("../Source/images/right.jpg");
	faces.push_back("../Source/images/left.jpg");
	faces.push_back("../Source/images/top.jpg");
	faces.push_back("../Source/images/bottom.jpg");
	faces.push_back("../Source/images/back.jpg");
	faces.push_back("../Source/images/front.jpg");
	GLuint cubemapTexture = loadCubemap(faces); //load the texture

	///Load the shaders
	skyboxShader_program = loadShaders("../Source/skybox.vs", "../Source/skybox.fss");
	shader_program = loadShaders("../Source/minecraft.vs", "../Source/minecraft.fss");
	//textureShader_program = loadShaders("../Source/cubeMap.vs", "../Source/cubeMap.fss");

	//SETUP THE VERTEX AND ELEMENT OBJECTS FOR FIRST USE
	setupVertexObjects();

	while (!glfwWindowShouldClose(window)) {

		//Smoothing the movement of the camera
		//Need to find the time between the current frame and the last frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// update other events like input handling
		glfwPollEvents();
		character_actions(scene_map, map_section, radius, size_cube);
		scene_map.getSceneCamera().update(cameraFront, yaw, pitch);

		//Setting camera
		view_matrix = glm::lookAt(scene_map.getSceneCamera().getCameraPosition(), scene_map.getSceneCamera().getCameraPosition() + cameraFront, scene_map.getSceneCamera().getCameraUp());

		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);

		//Use culling
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Draw skybox first
		glDepthMask(GL_FALSE);// Remember to turn depth writing off
		glUseProgram(skyboxShader_program);
		glm::mat4 view = glm::mat4(glm::mat3(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader_program, "projection"), 1, GL_FALSE, glm::value_ptr(proj_matrix));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(glGetUniformLocation(shader_program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		glUseProgram(shader_program);

		//OUR SCENE MATRIX
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, obj_indicesOfPoints.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);

		//OUR CHARACTER MATRIX
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(character_view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(character_model_matrix));

		glBindVertexArray(char_VAO);
		glDrawElements(GL_TRIANGLES, character_indicesOfPoints.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);

		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();

	//Memory deallocation
	for (int i = 0; i < size_width_section; ++i){

		for (int j = 0; j < size_length_section; j++){
			delete map_section[i][j];
		}

		delete[] map_section[i];		
	}

	delete [] map_section;
		 
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//COLLISION HANDLING
//--------------------------------------------------------------------------------------------------------------------------------------------

/* Old camera collision test ... too slow
glm::vec3 find_center_cube(glm::vec3 camera_position, char key_input, float sizeOfCube){

	//The following coordinates are for the location of the cube in front of the camera	
	if (key_input == 'w' || key_input == 's'){
		float pos_x = ((int)(camera_position[0] * 10)) / 10.0f;
		float pos_y = ((int)(camera_position[1] * 10)) / 10.0f;
		float pos_z = ((int)(camera_position[2] * 10)) / 10.0f - sizeOfCube / 2;

		pos_x += 0;
		pos_y += 0;
		pos_z += sizeOfCube / 2;

		glm::vec3 temp(pos_x, pos_y, pos_z);

		return temp;
	}
	else {
		float pos_x = ((int)(camera_position[0] * 10)) / 10.0f - sizeOfCube / 2;
		float pos_y = ((int)(camera_position[1] * 10)) / 10.0f;
		float pos_z = ((int)(camera_position[2] * 10)) / 10.0f;

		pos_x += sizeOfCube / 2;
		pos_y += 0;
		pos_z += 0;

		glm::vec3 temp(pos_x, pos_y, pos_z);

		return temp;
	}
}

bool inside_cube(unordered_map<string, string, StringHasher> map_of_coordinates, glm::vec3 camera_position, char key_input, float sizeOfCube){

	glm::vec3 center_cube = find_center_cube(camera_position, key_input, sizeOfCube);

	string center_cube_coordinates = to_string((int)((center_cube[0] + 1) * 10)) + to_string((int)((center_cube[1] + 1) * 10)) + to_string((int)((fabs(center_cube[2]) + 1) * 10));

	if (map_of_coordinates[center_cube_coordinates].length() == 0){
		return false;
	}
	
	else{
		//Find one of the diagonal of the cube and check if the point is within the cube

		glm::vec3 top_point(center_cube[0] - sizeOfCube / 2, center_cube[1] + sizeOfCube / 2, center_cube[2] + sizeOfCube / 2);
		glm::vec3 bottom_point(center_cube[0] + sizeOfCube / 2, center_cube[1] - sizeOfCube / 2, center_cube[2] - sizeOfCube / 2);

		//Check if the point is within bounds
		if (camera_position[0] >= top_point[0] && camera_position[0] <= bottom_point[0]
			&& camera_position[1] <= top_point[1] && camera_position[1] >= bottom_point[1]
			&& camera_position[2] <= top_point[2] && camera_position[2] >= bottom_point[2]){
			return true;
		}

		//Not within the front cube
		return false;
	}
	

	return true;
}
*/

//This is how the clamp functions
float clamp_values(float value, float min, float max) {
	return std::max(min, std::min(max, value));
}

inline float squared(float v){
	return v*v;
}

bool out_of_bounds(Scene scene_map, glm::vec3 scene_camera_position){

	glm::vec3 max = scene_map.getMaxBoundaryMap();
	glm::vec3 min = scene_map.getMinBoundaryMap();

	if (scene_camera_position[0] > max[0] || scene_camera_position[1] > max[1] || scene_camera_position[2] < max[2] || scene_camera_position[0] < min[0] || scene_camera_position[1] < min[1] || scene_camera_position[2] > min[2]){
		return true;
	}
		
	return false;
}

//Function to detect collision
bool checkCollision(glm::vec3 scene_camera_position, double radius, Cube cube) // AABB - Circle collision
{	
	float half_sizeCube = cube.getSize() / 2;

	// Get center point circle first 
	glm::vec3 camera_center = scene_camera_position + (float)radius;

	// Calculate AABB info (center, half-extents) --> to determine the closest point of the cube to the sphere
	glm::vec3 half_extents(half_sizeCube, half_sizeCube, half_sizeCube);
	glm::vec3 cube_center = cube.getCenter() + half_extents;

	// Get difference vector between both centers
	glm::vec3 difference = camera_center - cube_center;

	// Get the smallest vector from the center to find the closest point of the cube to the camera
	glm::vec3 clamped = glm::clamp(difference, -half_extents, half_extents);

	// Add clamped value to the cube's center and we get the value of box closest to circle
	glm::vec3 closest = cube_center + clamped;

	// Retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - camera_center;

	//requires a small bias >> account for the fact that floats are approximated
	return (double)glm::length(difference) - 0.01 < radius;
}

//Function to loop through all the scene objects to check if the camera has collided with an object
//Instead of vector<Cube> scene_cubes --> Use the map_section to find the cube
//Since we have the camera_position, we can determine which section we are in...
bool has_collided(Scene scene_map, vector<Cube>*** map_section, glm::vec3 scene_camera_position, double radius){

	int coord_x = scene_camera_position[0] / 11;
	int coord_z = -1*(scene_camera_position[2]) / 11;

	//100*100 because all of those are ground cubes, no need to check those objects
	for (int i = 100*100; i < map_section[coord_x][coord_z]->size(); i++){
		if (checkCollision(scene_camera_position, radius, map_section[coord_x][coord_z]->at(i))){
			return true;
		}
	}

	return false;
}

glm::vec3 getCenterViewCubePosition(glm::vec3 camera_position, float size_cube){
	
	float x = (int)(camera_position[0] * 10) / 10.0f;
	float y = (int)(camera_position[1] * 10) / 10.0f;
	float z = (int)(camera_position[2] * 10) / 10.0f;

	return glm::vec3(x, y, z);

}



//--------------------------------------------------------------------------------------------------------------------------------------------
//CAMERA/PERSON MOVEMENT
//--------------------------------------------------------------------------------------------------------------------------------------------

//Function to move our camera
void character_actions(Scene& scene_map, vector<Cube>*** map_section, double radius, float size_cube){

	//Smooth out the camera movement --> avoid lag 
	GLfloat cameraSpeed = 2.0 * deltaTime;
	//Temporary value to store the current camera position
	glm::vec3 nextCameraPos = scene_map.getSceneCamera().getCameraPosition();
	glm::vec3 cameraFront = scene_map.getSceneCamera().getCameraFront();
	Character scene_character = scene_map.getSceneCharacter();


	if (leftclick){
		//Where I want to place my object
		glm::vec3 translation_change = glm::vec3(0.5f*cameraFront[0], 0.5f*cameraFront[1], 0.5f*cameraFront[2]);
		nextCameraPos += translation_change;

		//Get the center position of where the camera is looking at
		nextCameraPos = getCenterViewCubePosition(nextCameraPos, size_cube);

		cout << nextCameraPos[0] << " " << nextCameraPos[1] << " " << nextCameraPos[2] << endl;

		//Check if the cube can be place:
		//1- If there is no object at that location
		if (nextCameraPos[1] > -0.1f || (*map_of_coordinates)[coordinatetoint(nextCameraPos[0], nextCameraPos[1], nextCameraPos[2])] != 0){

			cout << "Im in" << endl;

			glm::vec3 obj_position = getCenterViewCubePosition(nextCameraPos, size_cube);
			obj_position[1] -= size_cube;

			//2- If there is something underneath
			if ((*map_of_coordinates)[coordinatetoint(obj_position[0], obj_position[1], obj_position[2])] == 0){
				cout << "im creating cube" << endl;

				//Add the create cube function here
				createSceneCube(obj_coordinates, map_section, nextCameraPos[0], nextCameraPos[1], nextCameraPos[2], size_cube, 1);

				//clear the buffer
				g_vertex_buffer_data.clear();

				obj_indicesOfPoints.clear();

				//Sweep all the points again
				translationSweepMatrix(dir_translation, obj_coordinates, &g_vertex_buffer_data, &obj_indicesOfPoints);

				//Refresh buffer after adding block
				updateBuffer();
			}
		}	
	}

	else if (rightclick){
		//Remove cube function here




	}


	else if (keys[GLFW_KEY_SPACE]){
		if (!freeRoam && !jumped){
			glm::vec3 translation_change = glm::vec3(0.0f, size_cube, 0.0f);
			nextCameraPos += translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			else if (!has_collided(scene_map, map_section, nextCameraPos, radius)){
				//Make the camera move up in the Y-axis 
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				//scene_map.getSceneCamera().update(cameraPos, cameraFront, cameraUp, yaw, pitch); //update the camera
				jumped = true; //set the global boolean value jump to true
			}

			glm::vec3 translation_change2 = glm::vec3(cameraSpeed * cameraFront[0], 0.0f, cameraSpeed * cameraFront[2]);
			nextCameraPos += translation_change2 - translation_change;
			glm::vec3 onTopOfNextCam = nextCameraPos + translation_change;

			if (has_collided(scene_map, map_section, nextCameraPos, radius) && !has_collided(scene_map, map_section, onTopOfNextCam, radius)){
				nextCameraPos += translation_change;
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				jumped = false;
			}
		}
	}
	else if (!keys[GLFW_KEY_SPACE] && !freeRoam && jumped){
		/*Previous code
		glm::vec3 nextCameraPos = cameraPos;
		glm::vec3 translation_change = glm::vec3(0.0f, 0.1f, 0.0f);
		nextCameraPos -= translation_change;

		if (out_of_bounds(scene_map, nextCameraPos) || has_collided(scene_map, nextCameraPos, radius, scene_cube_objects, false)){
		cameraPos += translation_change;
		}
		else{
		//Decrease position by 1 block size
		nextCameraPos -= translation_change;
		if (nextCameraPos[1] >= -0.1f){
		//While we do not reach a solid cube
		while (!has_collided(scene_map, nextCameraPos, radius, scene_cube_objects, false)){
		//Decrease next cam position by 1 block size until we hit a solid cube
		if (nextCameraPos[1] == -0.1f){
		break;
		}
		nextCameraPos -= translation_change;
		}
		}

		cameraPos = nextCameraPos + translation_change;
		//Update camera
		scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
		}
		*/

		nextCameraPos[1] -= size_cube;
		scene_map.getSceneCamera().setCameraPosition(nextCameraPos);

		jumped = false;
	}

	//USE A TIMER INSTEAD TO DO A CHECK UP
	//WHEN CAMERA HITS OBJECT --> SET A BOOLEAN TO FALSE AND KEEP CHECK FOR OBSTACLES
	//WHEN CAMERA DOES NOT HIT OBJECT --> SET BOOLEAN TO TRUE, AND ONLY CHECK A COUPLE OF TIMES

	// Camera controls
	else if (keys[GLFW_KEY_W]){
		if (freeRoam){
			//Set the temporary position to the requested camera location
			glm::vec3 translation_change = glm::vec3(cameraSpeed * cameraFront[0], cameraSpeed * cameraFront[1], cameraSpeed * cameraFront[2]);
			nextCameraPos += translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			else if (nextCameraPos[1] > -size_cube && !has_collided(scene_map, map_section, nextCameraPos, radius)){
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
			}

		}
		else{
			//Set the temporary position to the requested camera location
			glm::vec3 translation_change = glm::vec3(cameraSpeed * cameraFront[0], 0, cameraSpeed * cameraFront[2]);
			nextCameraPos += translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			//check if there's a collision
			else if (!has_collided(scene_map, map_section, nextCameraPos, radius)){

				glm::vec3 delta(0.0f, size_cube, 0.0f);

				if (nextCameraPos[1] >= size_cube && nextCameraPos[1] <= size_cube*2)
					nextCameraPos = glm::vec3(nextCameraPos[0], 0, nextCameraPos[2]);
				else
					nextCameraPos = nextCameraPos - delta;

				//MUST INCLUDE CONDITION WHERE I CAN KNOW IF I AM HOVERING
				//Check if the camera is on a solid surface
				if (nextCameraPos[1] > -size_cube){
					while (!has_collided(scene_map, map_section, nextCameraPos, radius)){
						if (nextCameraPos[1] == -size_cube){
							break;
						}
						nextCameraPos[1] -= size_cube;
					}
					//Camera is not on a solid surface
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//update the camera position if it there are no collisions
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);

				}

				else{
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
				}
			}
		}
	}

	else if (keys[GLFW_KEY_S]){
		if (freeRoam){
			//Set the temporary position to the requested camera location
			glm::vec3 translation_change = glm::vec3(cameraSpeed * cameraFront[0], cameraSpeed * cameraFront[1], cameraSpeed * cameraFront[2]);
			nextCameraPos -= translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			else if (nextCameraPos[1] > -size_cube && !has_collided(scene_map, map_section, nextCameraPos, radius)){
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
			}
		}
		else{
			//Set the temporary position to the requested camera location
			glm::vec3 translation_change = glm::vec3(cameraSpeed * cameraFront[0], 0, cameraSpeed * cameraFront[2]);
			nextCameraPos -= translation_change;


			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			//check if there's a collision
			else if (!has_collided(scene_map, map_section, nextCameraPos, radius)){

				glm::vec3 delta(0.0f, size_cube, 0.0f);

				if (nextCameraPos[1] >= size_cube && nextCameraPos[1] <= size_cube*2)
					nextCameraPos = glm::vec3(nextCameraPos[0], 0, nextCameraPos[2]);
				else
					nextCameraPos = nextCameraPos - delta;

				//MUST INCLUDE CONDITION WHERE I CAN KNOW IF I AM HOVERING
				//Check if the camera is on a solid surface
				if (nextCameraPos[1] > -size_cube){
					while (!has_collided(scene_map, map_section, nextCameraPos, radius)){
						if (nextCameraPos[1] == -size_cube){
							break;
						}
						nextCameraPos[1] -= size_cube;
					}
					//Camera is not on a solid surface
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//update the camera position if it there are no collisions
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);

				}

				else{
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
				}
			}
		}
	}

	else if (keys[GLFW_KEY_A]){
		if (freeRoam){
			glm::vec3 translation_change = glm::normalize(glm::cross(cameraFront, scene_map.getSceneCamera().getCameraUp())) * cameraSpeed;
			nextCameraPos -= translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			else if (nextCameraPos[1] > -size_cube && !has_collided(scene_map, map_section, nextCameraPos, radius)){
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
			}
		}
		else{
			glm::vec3 translation_change = glm::normalize(glm::cross(cameraFront, scene_map.getSceneCamera().getCameraUp())) * cameraSpeed;
			nextCameraPos -= translation_change;


			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			//check if there's a collision
			else if (!has_collided(scene_map, map_section, nextCameraPos, radius)){

				glm::vec3 delta(0.0f, size_cube, 0.0f);

				if (nextCameraPos[1] >= size_cube && nextCameraPos[1] <= size_cube*2)
					nextCameraPos = glm::vec3(nextCameraPos[0], 0, nextCameraPos[2]);
				else
					nextCameraPos = nextCameraPos - delta;

				//MUST INCLUDE CONDITION WHERE I CAN KNOW IF I AM HOVERING
				//Check if the camera is on a solid surface
				if (nextCameraPos[1] > -size_cube){
					while (!has_collided(scene_map, map_section, nextCameraPos, radius)){
						if (nextCameraPos[1] == -size_cube){
							break;
						}
						nextCameraPos[1] -= size_cube;
					}
					//Camera is not on a solid surface
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//update the camera position if it there are no collisions
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);

				}

				else{
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
				}
			}
		}

	}

	else if (keys[GLFW_KEY_D]){
		if (freeRoam){
			glm::vec3 translation_change = glm::normalize(glm::cross(cameraFront, scene_map.getSceneCamera().getCameraUp())) * cameraSpeed;
			nextCameraPos += translation_change;

			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			else if (nextCameraPos[1] > -size_cube && !has_collided(scene_map, map_section, nextCameraPos, radius)){
				scene_map.getSceneCamera().setCameraPosition(nextCameraPos);
				//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
			}
		}
		else{
			glm::vec3 translation_change = glm::normalize(glm::cross(cameraFront, scene_map.getSceneCamera().getCameraUp())) * cameraSpeed;
			nextCameraPos += translation_change;


			if (out_of_bounds(scene_map, nextCameraPos)){
				//do nothing
			}

			//check if there's a collision
			else if (!has_collided(scene_map, map_section, nextCameraPos, radius)){

				glm::vec3 delta(0.0f, size_cube, 0.0f);

				if (nextCameraPos[1] >= size_cube && nextCameraPos[1] <= size_cube*2)
					nextCameraPos = glm::vec3(nextCameraPos[0], 0, nextCameraPos[2]);
				else
					nextCameraPos = nextCameraPos - delta;

				//MUST INCLUDE CONDITION WHERE I CAN KNOW IF I AM HOVERING
				//Check if the camera is on a solid surface
				if (nextCameraPos[1] > -size_cube){
					while (!has_collided(scene_map, map_section, nextCameraPos, radius)){
						if (nextCameraPos[1] == -size_cube){
							break;
						}
						nextCameraPos[1] -= size_cube;
					}
					//Camera is not on a solid surface
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//update the camera position if it there are no collisions
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);

				}

				else{
					scene_map.getSceneCamera().setCameraPosition(nextCameraPos + delta);
					//scene_camera.update(cameraPos, cameraFront, cameraUp, yaw, pitch);
				}
			}
		}
	}
	else if (keys[GLFW_KEY_C]){
		if (!freeRoam){
			freeRoam = true;
			scene_map.setPreviousCameraPosition(nextCameraPos);
		}
		else{
			freeRoam = false;
			scene_map.getSceneCamera().setCameraPosition(scene_map.getPreviousSceneCameraPosition());
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//INPUT HANDLING
//--------------------------------------------------------------------------------------------------------------------------------------------
void key_callback(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	
	switch (key){	
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS){		
			if (cursor_hidden == true){
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //show the cursor
				cursor_hidden = false;
			}
			else{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				cursor_hidden = true;
			}
		}
		break;

	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
		break;
	}
}

//HANDLE CURSOR POSITION TO KNOW IF IT SHOULD ZOOM IN OR ZOOM OUT
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05f;	// Change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen does not get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void mouse_click(GLFWwindow* window, int key, int action, int mods){

	switch (key){
	case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_PRESS){
			//Rotate hand on X-axis
			character_model_matrix = glm::translate(character_model_matrix, glm::vec3(0, 0, -0.05f));			
			character_model_matrix = glm::rotate(character_model_matrix, 0.25f, glm::vec3(-1, 0, 0));

			//The boolean value will be used in character actions
			leftclick = true;
		}
		else if (action == GLFW_RELEASE){
			//Rotate hand on X-axis
			character_model_matrix = glm::rotate(character_model_matrix, 0.25f, glm::vec3(1, 0, 0));
			character_model_matrix = glm::translate(character_model_matrix, glm::vec3(0, 0, 0.05f));

			leftclick = false;
		}
		break;
	
	//To remove cubes
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (action == GLFW_PRESS){
			//Translate hand on X-axis
			character_model_matrix = glm::translate(character_model_matrix, glm::vec3(0, 0, -0.1f));

			//The boolean value will be used in character actions
			rightclick = true;
		}
		else if (action == GLFW_RELEASE){
			//Translate hand on X-axis
			character_model_matrix = glm::translate(character_model_matrix, glm::vec3(0, 0, 0.1f));

			rightclick = false;
		}
		break;
	}
}

//In case we allow window resize
void windowResize(GLFWwindow* window, int w, int h){
	glfwGetWindowSize(window, &width_window, &height_window);
	glViewport(0, 0, w, h);
}

