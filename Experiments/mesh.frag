#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 cutCone;
uniform float transExp;

in vec3 eyeCoordNormal;
in vec3 fcolor;
in vec3 eyeCoordPos;
in vec3 rawPosition;

out vec4 fragColour;

void main(){
	if(rawPosition.x>cutCone.x && rawPosition.y>cutCone.y && rawPosition.z>cutCone.z){
		discard;
	}
	vec3 normal = normalize(eyeCoordNormal);
	vec3 lightDir = vec3(0,1,0);
	float ambient = 0.3;
	float diffusion = 0.6*clamp(dot(normal,lightDir),0,1);
	//float diffusion = 0.6*abs(dot(normal,lightDir));
	vec3 eyeDir = -normalize(eyeCoordPos);
	vec3 hv = normalize(eyeDir+lightDir);
	float specular = 0.2*pow(clamp(dot(hv,normal),0,1),64);
	//float specular = 0.2*clamp(pow(abs(dot(hv,normal)),16),0,1);
	fragColour = vec4(1,1,1,1);
	fragColour = fragColour*(ambient+diffusion);
	//fragColour += vec4(specular,specular,specular,0);
	fragColour.a = pow((1-abs(dot(normal,eyeDir))),transExp);
}