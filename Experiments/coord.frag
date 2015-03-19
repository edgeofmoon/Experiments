#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

in vec3 rawPos;

out vec4 fragColour;

void main() {
	fragColour = vec4(rawPos,1);
}