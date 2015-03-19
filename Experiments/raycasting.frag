#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler2D backFace;

uniform vec3 volSize;

uniform float windowWidth;
uniform float windowHeight;

uniform float sampeRate;
uniform float decayFactor;
uniform float thresHigh;
uniform float thresLow;

in vec3 position;
in vec3 fragPos;
in vec3 rawPos;

out vec4 fragColour;

void main() {
	fragColour=vec4(0,0,0,0);
	//fragColour=texture(backFace, vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight));
	
	vec2 coord = vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight);
	vec3 start = rawPos;
	vec3 end = texture(backFace, coord).xyz;
	float distance = length(end-start);
	vec3 dir = (end-start)/distance;
	float step = distance/sampeRate;

	vec3 offsetX = vec3(1/volSize.x, 0, 0);
	vec3 offsetY = vec3(0, 1/volSize.y, 0);
	vec3 offsetZ = vec3(0, 0, 1/volSize.z);

	float accTransparency = 1;
	float accDistance = 0;
	
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;

		float density = texture(densityVol, loc).r;

		/*
		if(density>=thresLow && density<=thresHigh){
			accTransparency = accTransparency*exp(-density*step*decayFactor);
		}
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		*/
		
		accDistance+=step;
		if(density < thresLow) continue;

		float dx = texture(densityVol, loc+offsetX).r-texture(densityVol, loc-offsetX).r;
		float dy = texture(densityVol, loc+offsetY).r-texture(densityVol, loc-offsetY).r;
		float dz = texture(densityVol, loc+offsetZ).r-texture(densityVol, loc-offsetZ).r;

		vec4 localDir = vec4(dx,dy,dz,0);

		vec3 normal = normalize((transpose(inverse(mvMat))*localDir).xyz);
		vec3 lightDir = vec3(0,0,1);
		float ambient = 0.3;
		float diffusion = 0.6*clamp(dot(normal,lightDir),0,1);
		vec3 pos = fragPos+dir*accDistance;
		vec3 eyeDir = -normalize(pos);
		vec3 hv = normalize(eyeDir+lightDir);
		//float specular = 0.5*pow(clamp(dot(hv,normal),0,1),16);
		float specular = 0.5*pow(abs(dot(hv,normal)),16);
		fragColour = vec4(abs(normalize(localDir.xyz)),1);
		fragColour = fragColour*(ambient+diffusion);
		fragColour += vec4(specular,specular,specular,0);
		fragColour.a = 1;
		break;
	}
	//float opacity = 1-accTransparency;
	//fragColour=vec4(opacity,opacity,opacity,1);

	//fragColour=vec4(start,1);
	//fragColour=vec4(end,1);
}