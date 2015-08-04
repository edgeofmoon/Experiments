#version 330

uniform sampler2D colorTexture;

uniform vec2 center;
uniform vec2 radius;
uniform float focusRadius;
uniform float scale;

in vec2 position;
out vec4 fragColour;

float LinearMapTo(float value, vec2 domain, vec2 range){
	// linear mapping for simplicity
	float t = (value-domain.x)/(domain.y-domain.x);
	return t*(range.y-range.x)+range.x;
}

float GaussianMapTo(float value, vec2 domain, vec2 range){
	float t = (value-domain.x)/(domain.y-domain.x);
	float g = exp(-5*t*t);
	return LinearMapTo(g, vec2(1, exp(-5.0)), range);
}

void main() {
	//fragColour = texture(colorTexture, position);
	//fragColour = vec4(0.5,0.4,0.3,1);
	//return;
	vec2 tmp = vec2((position.x-center.x)/radius.x, (position.y-center.y)/radius.y);
	float scaledDistance = length(tmp);
	if(scaledDistance>=1){
		fragColour = texture(colorTexture, position);
	}
	else{
		vec2 dir = normalize(vec2(position.x-center.x, position.y-center.y));
		if(scaledDistance > focusRadius){
			// dropoff zone
			float newScale = GaussianMapTo(scaledDistance, vec2(focusRadius, 1), vec2(scale, 1));
			vec2 samplePos = (position-center)/newScale+center;
			fragColour = texture(colorTexture, samplePos);
		}
		else{
			//focus zone
			float sampleX = (position.x-center.x)/scale+center.x;
			float sampleY = (position.y-center.y)/scale+center.y;
			fragColour = texture(colorTexture, vec2(sampleX, sampleY));
		}
	}
}