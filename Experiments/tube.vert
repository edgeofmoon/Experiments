#version 330

uniform sampler2D texUnit;
uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 boxLow1;
uniform vec3 boxHigh1;
uniform vec3 boxLow2;
uniform vec3 boxHigh2;
uniform int screenSpace;

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in float radius;
in vec4 color;


out vec4 colourV;
out vec3 fnormal;
out vec3 fposition;

out float isInBox;

void main (void)
{
	//float r = radius;
	float r = 0.4;
	vec3 tposition = position;//+normal*r;
	gl_Position = projMat*mvMat*vec4(tposition,1);

	fposition = vec3(mvMat*vec4(tposition,1));
	colourV = vec4(0.5,0.5,0.5,1);
	fnormal = (transpose(inverse(mvMat))*vec4(normal,0)).xyz;
}