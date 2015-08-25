#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 color;

in vec3 position;
in vec3 normal;
in ivec4 name;

out vec3 fposition;
out vec3 fnormal;
flat out ivec4 fname;

void main (void)
{
	vec3 tposition = position;
	fnormal = (transpose(inverse(mvMat))*vec4(normal,0)).xyz;
	fposition = vec3(mvMat*vec4(tposition,1));
	fname = name;
	gl_Position = projMat*mvMat*vec4(position,1);
}