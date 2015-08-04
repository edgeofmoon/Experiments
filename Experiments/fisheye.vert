#version 330

uniform sampler2D colorTexture;

uniform vec2 center;
uniform vec2 radius;
uniform float focusRadius;
uniform float scale;

in vec2 in_position;

out vec2 position;

void main() {
	position = in_position*0.5+vec2(0.5,0.5);
	gl_Position = vec4(in_position.x, in_position.y,0,1);
}