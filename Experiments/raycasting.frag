#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler2D backFace;
uniform sampler2D frontFace;

uniform vec3 volSize;

uniform float windowWidth;
uniform float windowHeight;

in vec3 opos;

out vec4 fragColour;

void main() {
	//fragColour=texture(backFace, vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight));
	vec2 coord = vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight);
	vec3 start = texture(backFace, coord).xyz;
	vec3 end = texture(frontFace, coord).xyz;
	float distance = length(end-start);
	vec3 dir = (end-start)/distance;
	float step = distance/100;

	vec3 offset = vec3(1/volSize.x, 1/volSize.y, 1/volSize.z);

	float accTransparency = 1;
	float accDistance = 0;
	
	for(int i = 0;i<100; i++){
		vec3 loc = start + dir*accDistance;
		
		vec3 loc000=vec3(loc.x-offset.x, loc.y-offset.y, loc.z-offset.z);
		vec3 loc100=vec3(loc.x+offset.x, loc.y-offset.y, loc.z-offset.z);
		vec3 loc010=vec3(loc.x-offset.x, loc.y+offset.y, loc.z-offset.z);
		vec3 loc110=vec3(loc.x+offset.x, loc.y+offset.y, loc.z-offset.z);
		vec3 loc001=vec3(loc.x-offset.x, loc.y-offset.y, loc.z+offset.z);
		vec3 loc101=vec3(loc.x+offset.x, loc.y-offset.y, loc.z+offset.z);
		vec3 loc011=vec3(loc.x-offset.x, loc.y+offset.y, loc.z+offset.z);
		vec3 loc111=vec3(loc.x+offset.x, loc.y+offset.y, loc.z+offset.z);

		float density = texture(densityVol, loc).r;
		float density000 = texture(densityVol, loc000).r;
		float density100 = texture(densityVol, loc100).r;
		float density010 = texture(densityVol, loc010).r;
		float density110 = texture(densityVol, loc110).r;
		float density001 = texture(densityVol, loc001).r;
		float density101 = texture(densityVol, loc101).r;
		float density011 = texture(densityVol, loc011).r;
		float density111 = texture(densityVol, loc111).r;

		accTransparency = accTransparency*exp(-density*step*10);
		//accTransparency = min(accTransparency, 0.8-density);
		accDistance+=step;
		//if(accDistance >= distance) break;
		if(accTransparency <= 0.01) break;
	}
	float opacity = 1-accTransparency;
	fragColour=vec4(opacity,opacity,opacity,1);
	//fragColour=vec4(start,1);
}