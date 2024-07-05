#version 330

uniform vec4 u_modulate;

out vec4 out_color;

void main() 
{
	out_color = u_modulate;
}