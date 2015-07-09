#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 cutCone;
uniform float transExp;

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 eyeCoordNormal;
out vec3 fcolor;
out vec3 eyeCoordPos;
out vec3 rawPosition;

void main() {
	rawPosition = position;
	eyeCoordPos = (mvMat*vec4(position,1)).xyz;
	eyeCoordNormal = (inverse(mvMat)*vec4(normal,0)).xyz;
	gl_Position = projMat*vec4(eyeCoordPos,1);
}