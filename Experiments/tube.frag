#version 330

in vec4 colourV;
in vec3 fnormal;
in vec3 fposition;

in float isInBox;

out vec4 fragColour;

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
	fragColour = colourV;//texture(texUnit, texCoord);
	fragColour = colourV*(ambient+diffusion);
	fragColour += vec4(specular,specular,specular,0);
	fragColour.a = 1;
}