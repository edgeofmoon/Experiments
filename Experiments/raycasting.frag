#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

uniform sampler3D densityVol;
uniform sampler3D gradVol;
uniform sampler2D backFace;
uniform sampler2D colorMap;

uniform vec3 volSize;
uniform vec3 cutCone;

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

/*
// i: 0~26
vec3 GetIndex(int i){
	vec3 array[27]=vec3[27](
		vec3(-1,-1,-1), vec3(0,-1,-1), vec3(1,-1,-1),
		vec3(-1,0,-1), vec3(0,0,-1), vec3(1,0,-1),
		vec3(-1,1,-1), vec3(0,1,-1), vec3(1,1,-1),
		vec3(-1,-1,0), vec3(0,-1,0), vec3(1,-1,0),
		vec3(-1,0,0), vec3(0,0,0), vec3(1,0,0),
		vec3(-1,1,0), vec3(0,1,0), vec3(1,1,0),
		vec3(-1,-1,1), vec3(0,-1,1), vec3(1,-1,1),
		vec3(-1,0,1), vec3(0,0,1), vec3(1,0,1),
		vec3(-1,1,1), vec3(0,1,1), vec3(1,1,1)
	)

	return array[i];
}
*/

// i: 0~26
vec3 GetIndex(float i){
	float x = floor(mod(i,3))-1;
	float y = floor(mod(i,9)/3)-1;
	float z = floor(i/9)-1;
	return vec3(x,y,z);
}

vec3 EvaluateNormal(sampler3D vol, vec3 loc){
	vec3 offset = vec3(1/volSize.x, 1/volSize.y, 1/volSize.z);
	vec3 iloc = floor(loc*volSize)/volSize;

	vec3 normalAcc = vec3(0,0,0);
	for(int i = 0;i<27;i++){
		//vec3 tiloc=iloc+GetIndex(i)*offset;
		vec3 tiloc=loc+GetIndex(i)*offset;
		float density = texture(vol, tiloc).r;
		//float distance = length(loc-tiloc);
		float distance = length(GetIndex(i));
		float wk;
		if(i==13) {
			wk=0;
		}
		else{
			wk=1/distance/distance;
		}
		normalAcc += wk*density*GetIndex(i);
	}

	return normalAcc;
}

// loc: texture coord
vec4 InterpolatePointZ(sampler3D vol, vec3 loc){
	float z0 = floor(loc.z*volSize.z)/volSize.z;
	float z1 = z0+1/volSize.z;
	float fracz = loc.z-z0;
	return texture(vol, vec3(loc.xy, z0))*(1-fracz)+texture(vol, vec3(loc.xy, z1))*fracz;
}
vec4 InterpolatePointYZ(sampler3D vol, vec3 loc){
	float y0 = floor(loc.y*volSize.y)/volSize.y;
	float y1 = y0+1/volSize.y;
	float fracy = loc.y-y0;

	return InterpolatePointZ(vol, vec3(loc.x, y0, loc.z))*(1-fracy)
		+InterpolatePointZ(vol, vec3(loc.x, y1, loc.z))*fracy;
}
vec4 InterpolatePointXYZ(sampler3D vol, vec3 loc){
	float x0 = floor(loc.x*volSize.x)/volSize.x;
	float x1 = x0+1/volSize.x;
	float fracx = loc.x-x0;

	return InterpolatePointYZ(vol, vec3(x0, loc.yz))*(1-fracx)
		+InterpolatePointYZ(vol, vec3(x1, loc.yz))*fracx;
}



// loc: texture coord
vec3 GetLocalGradient(sampler3D vol, vec3 loc){
	vec3 offsetX = vec3(1/volSize.x, 0, 0);
	vec3 offsetY = vec3(0, 1/volSize.y, 0);
	vec3 offsetZ = vec3(0, 0, 1/volSize.z);

	float dx = texture(densityVol, loc+offsetX).r-texture(densityVol, loc-offsetX).r;
	float dy = texture(densityVol, loc+offsetY).r-texture(densityVol, loc-offsetY).r;
	float dz = texture(densityVol, loc+offsetZ).r-texture(densityVol, loc-offsetZ).r;
	
	//float dx = texture(densityVol, loc+offsetX).r-texture(densityVol, loc).r;
	//float dy = texture(densityVol, loc+offsetY).r-texture(densityVol, loc).r;
	//float dz = texture(densityVol, loc+offsetZ).r-texture(densityVol, loc).r;

	//float dx = InterpolatePointXYZ(densityVol, loc+offsetX).r-InterpolatePointXYZ(densityVol, loc-offsetX).r;
	//float dy = InterpolatePointXYZ(densityVol, loc+offsetY).r-InterpolatePointXYZ(densityVol, loc-offsetY).r;
	//float dz = InterpolatePointXYZ(densityVol, loc+offsetZ).r-InterpolatePointXYZ(densityVol, loc-offsetZ).r;
	
	return vec3(dx,dy,dz);
}

// loc: texture coord
vec3 InterpolateGradientZ(sampler3D vol, vec3 loc){
	float z0 = floor(loc.z*volSize.z)/volSize.z;
	float z1 = z0+1/volSize.z;
	float fracz = loc.z-z0;
	return GetLocalGradient(vol, vec3(loc.xy, z0))*(1-fracz)+GetLocalGradient(vol, vec3(loc.xy, z1))*fracz;
}
vec3 InterpolateGradientYZ(sampler3D vol, vec3 loc){
	float y0 = floor(loc.y*volSize.y)/volSize.y;
	float y1 = y0+1/volSize.y;
	float fracy = loc.y-y0;

	return InterpolateGradientZ(vol, vec3(loc.x, y0, loc.z))*(1-fracy)
		+InterpolateGradientZ(vol, vec3(loc.x, y1, loc.z))*fracy;
}
vec3 InterpolateGradientXYZ(sampler3D vol, vec3 loc){
	float x0 = floor(loc.x*volSize.x)/volSize.x;
	float x1 = x0+1/volSize.x;
	float fracx = loc.x-x0;

	return InterpolateGradientYZ(vol, vec3(x0, loc.yz))*(1-fracx)
		+InterpolateGradientYZ(vol, vec3(x1, loc.yz))*fracx;
}

// loc: texture coord
vec3 GetLocalGradientSmooth(vec3 loc){

	//vec3 g0 = InterpolateGradientXYZ(densityVol, loc);
	//vec3 g0 = GetLocalGradient(densityVol, loc);
	vec3 g0 = EvaluateNormal(densityVol, loc);
	//vec3 g0 = texture(gradVol, loc).xyz;
	return g0;
}

// loc: texture coord
// pos: world coord
vec4 GetLightedColor(vec3 loc, vec3 pos, float density){

	//return vec4(1,1,1,1);
	//return vec4(abs(GetLocalGradient(loc)),1);

	return vec4(vec3(density),1);

	vec4 localDir = -vec4(GetLocalGradientSmooth(loc),0);
	vec3 normal = normalize((transpose(inverse(mvMat))*localDir).xyz);
	vec3 lightDir = vec3(0,1,0);
	float ambient = 0.3;
	float diffusion = 0.6*clamp(dot(normal,lightDir),0,1);
	//float diffusion = 0.6*abs(dot(normal,lightDir));
	vec3 eyeDir = -normalize(pos);
	vec3 hv = normalize(eyeDir+lightDir);
	float specular = 0.5*pow(clamp(dot(hv,normal),0,1),64);
	//float specular = 0.5*pow(abs(dot(hv,normal)),16);
	//vec4 rstColour = vec4(vec3(density),1);
	vec4 rstColour = vec4(texture(colorMap, vec2(density, 0.5)).xyz,1);
	//vec4 rstColour = vec4(1,1,1,1);
	rstColour = rstColour*(ambient+diffusion);
	rstColour += vec4(specular,specular,specular,0);
	rstColour.a = 1;
	return rstColour;
}

void WriteDepth(vec3 pos){
	float far=gl_DepthRange.far; 
	float near=gl_DepthRange.near;

	vec4 clip_space_pos = projMat * mvMat * vec4(pos,1);

	float ndc_depth = clip_space_pos.z / clip_space_pos.w;

	float depth = (((far-near) * ndc_depth) + near + far) / 2.0;
	gl_FragDepth = depth;
}

void main() {
	fragColour=vec4(0,0,0,1);
	//fragColour=texture(backFace, vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight));
	
	vec2 coord = vec2(gl_FragCoord.x/windowWidth, gl_FragCoord.y/windowHeight);
	vec3 start = rawPos;
	vec3 end = texture(backFace, coord).xyz;
	//vec3 end = rawPos;
	//vec3 start = texture(backFace, coord).xyz;
	float distance = length(end-start);
	vec3 dir = (end-start)/distance;
	float step = distance/sampeRate;

	vec3 eyeDir=normalize(fragPos);

	float accTransparency = 1;
	float accDistance = 0;
	vec3 accColor = vec3(0,0,0);
	
	// experiment
	bool fvisible = false;
	bool tvisible = false;
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		vec3 loc2 = vec3(loc.x,loc.y,1-loc.z);
		if(loc.x>cutCone.x && loc.y<cutCone.y && loc.z>cutCone.z){
			tvisible = true;
			accDistance+=step;
			continue;
		};
		float density = texture(densityVol, loc2).r;
		if(density>thresLow){
			vec3 color = GetLightedColor(loc, fragPos+eyeDir*accDistance, density).xyz;
			accColor = color;
			accTransparency = 0;
			fvisible = tvisible || fvisible;
			break;
		}
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		//if(accDistance>distance) break;

	}
	fragColour=vec4(accColor*(1-accTransparency),(1-accTransparency));
	//fragColour=vec4(accTransparency,accTransparency,accTransparency,accTransparency);
	vec3 dir2 = normalize(vec3(inverse(mvMat)*vec4(eyeDir,0)));
	vec3 pos = rawPos+dir2*accDistance;
	WriteDepth(pos);
	
	/*
	// isosurface
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		if(loc.x>0.5 && loc.y<0.5 && loc.z>0.5){
			accDistance+=step;
			continue;
		};
		float density = texture(densityVol, loc).r;
		if(density>thresLow){
			vec3 color = GetLightedColor(loc, fragPos+eyeDir*accDistance, density).xyz;
			accColor = color;
			accTransparency = 0;
			break;
		}
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		//if(accDistance>distance) break;
	}
	fragColour=vec4(accColor*(1-accTransparency),(1-accTransparency));
	//fragColour=vec4(accTransparency,accTransparency,accTransparency,accTransparency);
	vec3 dir2 = normalize(vec3(inverse(mvMat)*vec4(eyeDir,0)));
	vec3 pos = rawPos+dir2*accDistance;
	WriteDepth(pos);
	*/

	/*
	// density->color map
	// gradient->opacity
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		if(density>thresLow){
			vec3 color = GetLightedColor(loc, fragPos+eyeDir*accDistance, density).xyz;
			//float transparency = exp(-density*decayFactor*step);
			float transparency = max(1-length(GetLocalGradientSmooth(loc))*density*step*decayFactor,0);
			//float transparency = max(1-density*step*decayFactor,0);
			accColor += color*(1-transparency)*accTransparency;
			accTransparency *= transparency;
		}
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		//if(accDistance>distance) break;
	}
	fragColour=vec4(accColor*(1-accTransparency),1);
	*/

	/*
	// density->color map
	// gradient->opacity
	// untile threshold is reached
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		vec3 color = vec3(1,1,1);
		//float transparency = exp(-density*decayFactor*step);
		float transparency = max(1-length(GetLocalGradientSmooth(loc))*density*step*decayFactor,0);
		//float transparency = max(1-density*step*decayFactor,0);
		if(density>thresLow){
			color = GetLightedColor(loc, fragPos+eyeDir*accDistance, density).xyz;
			accTransparency *= 1.5;
			transparency = 0;
		}
		accColor += color*(1-transparency)*accTransparency;
		accTransparency *= transparency;
		if(density>thresLow){
			break;
		}
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		//if(accDistance>distance) break;
	}
	fragColour=vec4(accColor*(1-accTransparency),1);
	*/

	/*
	// max intensity projection
	float maxDensity = 0;
	float target_distance = 0;
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		if(density>maxDensity){
			maxDensity = density;
			target_distance = accDistance;
		}
		accDistance+=step;
	}
	vec3 loc = start + dir*target_distance;
	float density = texture(densityVol, loc).r;
	fragColour = GetLightedColor(loc, fragPos+eyeDir*target_distance, density);
	*/

	/*
	// max intensity projection
	// add outer layer as reference
	float maxDensity = 0;
	float target_distance = 0;
	int idx = 0;
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		if(density>maxDensity){
			maxDensity = density;
			idx = i;
		}
		accDistance+=step;
	}
	accDistance = 0;
	for(int i = 0;i<idx; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		vec3 color = vec3(1,1,1);
		//float transparency = exp(-density*decayFactor*step);
		float transparency = max(1-length(GetLocalGradientSmooth(loc))*density*step*decayFactor,0);
		//float transparency = max(1-density*step*decayFactor,0);
		accColor += color*(1-transparency)*accTransparency;
		accTransparency *= transparency;
		accDistance+=step;
		if(accTransparency <= 0.01) break;
		//if(accDistance>distance) break;
	}
	// composite max
	accDistance += step;
	vec3 loc = start + dir*accDistance;
	float density = texture(densityVol, loc).r;
	vec3 color = GetLightedColor(loc, fragPos+eyeDir*accDistance, density).xyz;
	float transparency = 0;
	accColor += color*(1-transparency)*accTransparency;
	accTransparency *= transparency;
	fragColour=vec4(accColor*(1-accTransparency),1);
	*/

	/*
	// two sample isosurface extraction
	float maxDensity = 0;
	float target_distance = 0;
	for(int i = 0;i<sampeRate; i++){
		vec3 loc = start + dir*accDistance;
		float density = texture(densityVol, loc).r;
		if(density>maxDensity){
			maxDensity = density;
			target_distance = accDistance;
		}
		accDistance+=step;
	}
	vec3 loc0=start + dir*(target_distance-step);
	vec3 loc1=start + dir*target_distance;
	float density0 = texture(densityVol, loc0).r;
	float density1 = texture(densityVol, loc1).r;
	vec4 color0 = GetLightedColor(loc0, fragPos+eyeDir*(target_distance-step), density0);
	vec4 color1 = GetLightedColor(loc1, fragPos+eyeDir*target_distance, density1);
	fragColour = (color0+color1)*0.5;
	//fragColour = color1;
	*/
}