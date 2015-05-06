#version 330


uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler3D attriVol;
uniform sampler2D backFace;
uniform sampler2D colorMap;

uniform vec3 volSize;
uniform vec3 cutCone;

uniform float windowWidth;
uniform float windowHeight;

uniform float sampeRate;
uniform float decayFactor;
uniform float thresHigh;
uniform float thresLow;

in vec3 position;
out vec3 fragPos;
out vec3 rawPos;

void main() {
	fragPos = (mvMat*vec4(position,1)).xyz;
	rawPos = position;
	gl_Position = projMat*mvMat*vec4(position,1);
}