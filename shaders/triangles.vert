#version 330

in vec3 i_position;
in vec3 i_offset;
in vec4 i_color;

out vec4 v_color;

uniform mat4 u_projection_matrix;

void main ()
{
	v_color = i_color;
	gl_Position = u_projection_matrix * vec4( (i_offset + i_position), 1.0 );
}