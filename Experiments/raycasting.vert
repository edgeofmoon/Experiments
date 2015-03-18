#version 330


uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler2D backFace;
uniform sampler2D frontFace;

uniform vec3 volSize;

uniform float windowWidth;
uniform float windowHeight;

in vec3 position;
out vec3 opos;

void main() {
	opos = position;
	gl_Position = projMat*mvMat*vec4(position,1);
}