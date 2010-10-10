/** 
 * @file postgiF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2D		diffuseGIMap;
uniform sampler2D		normalGIMap;
uniform sampler2D		depthGIMap;
uniform sampler2D		diffuseMap;

uniform sampler2D		lastDiffuseGIMap;
uniform sampler2D		lastNormalGIMap;
uniform sampler2D		lastMinpGIMap;
uniform sampler2D		lastMaxpGIMap;

uniform float gi_blend;

uniform mat4 gi_mat;  //gPipeline.mGIMatrix - eye space to sun space
uniform mat4 gi_mat_proj; //gPipeline.mGIMatrixProj - eye space to projected sun space
uniform mat4 gi_norm_mat; //gPipeline.mGINormalMatrix - eye space normal to sun space normal matrix
uniform mat4 gi_inv_proj; //gPipeline.mGIInvProj - projected sun space to sun space
uniform float gi_radius;
uniform float gi_intensity;
uniform vec2 gi_kern[16];
uniform vec2 gi_scale;


vec4 getGIPosition(vec2 gi_tc)
{
	float depth = texture2D(depthGIMap, gi_tc).a;
	vec2 sc = gi_tc*2.0;
	sc -= vec2(1.0, 1.0);
	vec4 ndc = vec4(sc.x, sc.y, 2.0*depth-1.0, 1.0);
	vec4 pos = gi_inv_proj*ndc;
	pos.xyz /= pos.w;
	pos.w = 1.0;
	return pos;
}


void main() 
{
	vec2 c_tc = gl_TexCoord[0].xy;
	
	vec3 diff = vec3(0,0,0);
	vec3 minp = vec3(1024,1024,1024);
	vec3 maxp = vec3(-1024,-1024,-1024);
	vec3 norm = vec3(0,0,0);
	
	float dweight = 0.0;
	
	vec3 cnorm = normalize(texture2D(normalGIMap, c_tc).rgb*2.0-1.0);
	
	vec3 cpos = vec3(0,0,0);
	float tweight = 0.0;
	
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			vec2 tc = vec2(i-4+0.5, j-4+0.5);
			float weight = 1.0-length(tc)/6.0;
			tc *= 1.0/(256.0);
			tc += c_tc;
					
			vec3 n = texture2D(normalGIMap, tc).rgb*2.0-1.0;
			tweight += weight;
			
			diff += weight*texture2D(diffuseGIMap, tc).rgb;
		
			norm += n*weight;
			
			dweight += dot(n, cnorm);
			
			vec3 pos = getGIPosition(tc).xyz;
			cpos += pos*weight;
			
			minp = min(pos, minp);
			maxp = max(pos, maxp); 
		}
	}
		
	dweight = abs(1.0-dweight/64.0);
	float mind = min(sqrt(dweight+0.5), 1.0);
	
	dweight *= dweight;
	
	cpos /= tweight;
	
	diff = clamp(diff/tweight, vec3(1.0/2.2), vec3(1,1,1));
	norm = normalize(norm);
	maxp = cpos;
	minp = vec3(dweight, mind, cpos.z-minp.z);
	
	//float blend = 1.0;
	//diff = mix(texture2D(lastDiffuseGIMap, c_tc).rgb, diff, blend);
	//norm = mix(texture2D(lastNormalGIMap, c_tc).rgb, norm, blend);
	//maxp = mix(texture2D(lastMaxpGIMap, c_tc).rgb, maxp, blend);
	//minp = mix(texture2D(lastMinpGIMap, c_tc).rgb, minp, blend);
	
	gl_FragData[0].rgb = diff;
	gl_FragData[2].xyz = normalize(norm);
	gl_FragData[1].xyz = maxp;
	gl_FragData[3].xyz = minp;
}
