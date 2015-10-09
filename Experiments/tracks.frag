#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 color;
uniform sampler3D filterVol;

in vec3 rawPosition;
in vec3 fposition;
in vec3 fnormal;
flat in ivec4 fname;

out vec4 fragColour;
out ivec4 name;

float LinearizeDepth(float z, float n, float f)
{
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main(void)
{
	float density = texture(filterVol, vec3(rawPosition.x/182, rawPosition.y/218, rawPosition.z/182)).r;
	//if(density<0.3) discard;
	//fragColour = vec4(0,0,0,1);
	//return;
	vec3 normal = normalize(fnormal);
	vec3 lightDir = vec3(0,0,1);
	float ambient = 0.3;
	//float diffusion = 0.6*clamp(dot(normal,lightDir),0,1);
	float diffusion = 0.6*abs(dot(normal,lightDir));
	vec3 eyeDir = normalize(-fposition);
	vec3 hv = normalize(eyeDir+lightDir);
	//float specular = 0.5*pow(clamp(dot(hv,normal),0,1),16);
	float specular = 0.5*pow(abs(dot(hv,normal)),16);
	//fragColour = vec4(color,1)*(ambient+diffusion);
	fragColour = vec4(1,0,0,1)*(ambient+diffusion);
	fragColour += vec4(specular,specular,specular,0);
	fragColour.a = 1;
	//fragColour.a = pow((1-abs(dot(normal,eyeDir))),2);
	//fragColour = vec4(density, 1-density, density, 1);
	name = fname;
}