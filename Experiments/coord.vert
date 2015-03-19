#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

in vec3 position;
out vec3 rawPos;

void main() {
	vec4 modelPos = mvMat*vec4(position,1);
	rawPos = position;
	gl_Position = projMat*modelPos;
}