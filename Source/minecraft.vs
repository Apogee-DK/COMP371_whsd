#version 330

uniform mat4 view_matrix, model_matrix, proj_matrix;

layout (location = 0) in vec3 Position;		//vertex position
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main () {
	mat4 CTM = proj_matrix * view_matrix * model_matrix;
	gl_Position = CTM * vec4 (Position, 1.0);

	TexCoords = texCoords;
	FragPos = Position;
	Normal = normal;


}