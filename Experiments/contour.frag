#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform vec3 color;

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
	fragColour = vec4(color,1)*(ambient+diffusion);
	fragColour += vec4(specular,specular,specular,0);
	fragColour.a = 1;
	name = fname;
	//name = ivec4(123,2,1,1);
}