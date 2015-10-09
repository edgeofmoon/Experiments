#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D vol;
uniform sampler2D backFace;
uniform sampler2D colorMap;
uniform float threshold;

uniform vec3 volSize;

uniform float windowWidth;
uniform float windowHeight;

uniform float sampeRate;
uniform float decayFactor;

in vec3 position;
out vec3 fragPos;
out vec3 rawPos;

void main() {
	fragPos = (mvMat*vec4(position,1)).xyz;
	rawPos = position;
	gl_Position = projMat*mvMat*vec4(position,1);
}