#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler2D meshDepthTex;
uniform sampler2D meshColorTex;

uniform float windowWidth;
uniform float windowHeight;

in vec3 position;
out vec3 rawPos;

void main() {
	rawPos = position;
	gl_Position = projMat*mvMat*vec4(position,1);
}