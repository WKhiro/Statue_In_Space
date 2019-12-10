#version 330 core

//The vertex shader gets called once per vertex.

//Define the position
layout (location = 0) in vec3 vertex;

//Define uniform MVP passed from the object.
uniform mat4 MVP;

void main()
{
	
    gl_Position = MVP * vec4(vertex.x, vertex.y, vertex.z, 1.0f);  
}  