#version 330

uniform mat4 view_matrix, model_matrix, proj_matrix;

layout (location = 0) in vec3 Position;		//vertex position
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main () {
	mat4 CTM = proj_matrix * view_matrix * model_matrix;
	gl_Position = CTM * vec4 (Position, 1.0);

	TexCoords = texCoords;
}