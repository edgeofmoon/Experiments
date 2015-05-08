#version 330


uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler2D meshDepthTex;
uniform sampler2D meshColorTex;
uniform sampler2D colorMap;

uniform float windowWidth;
uniform float windowHeight;

in vec3 rawPos;
out vec4 fragColour;

void main() {
	
	float value = texture(densityVol, vec3(rawPos.xy,1-rawPos.z)).r;
	vec2 texCoord = vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight);
	float meshDepth = texture(meshDepthTex, texCoord).r;
	vec4 meshColor = texture(meshColorTex, texCoord);
	//fragColour = vec4(value,value,value,1);
	//fragColour = meshColor;
	if(meshDepth<0.98 && meshColor.r<0.05){
		//fragColour = vec4(value,value,value,1);
		fragColour = texture(colorMap, vec2(value,0.5));
	}
	else{
		discard;
	}
	
}