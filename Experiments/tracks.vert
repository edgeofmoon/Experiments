#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 color;
uniform sampler3D filterVol;

in vec3 position;
in vec3 normal;
in ivec4 name;

out vec3 rawPosition;
out vec3 fposition;
out vec3 fnormal;
flat out ivec4 fname;

void main (void)
{
	rawPosition = position;
	fnormal = (transpose(inverse(mvMat))*vec4(normal,0)).xyz;
	fposition = vec3(mvMat*vec4(rawPosition,1));
	fname = name;
	gl_Position = projMat*mvMat*vec4(position,1);
}