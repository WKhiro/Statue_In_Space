#version 330 core

//Define colors.
uniform vec3 set_color;

//Define out variable for the fragment shader: color.
out vec4 color;

void main()
{
	color = vec4(set_color, 1.0);
}