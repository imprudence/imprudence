/** 
 * @file postDeferredF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2DRect diffuseRect;
uniform sampler2DRect specularRect;
uniform sampler2DRect localLightMap;
uniform sampler2DRect sunLightMap;
uniform sampler2DRect giLightMap;
uniform sampler2D	  luminanceMap;
uniform sampler2DRect lightMap;
uniform sampler2D	  lightFunc;


uniform vec3 gi_lum_quad;
uniform vec3 sun_lum_quad;
uniform vec3 lum_quad;
uniform float lum_lod;
uniform vec4 ambient;

uniform vec3 gi_quad;

uniform vec2 screen_res;
varying vec2 vary_fragcoord;

void main() 
{
	vec2 tc = vary_fragcoord.xy;
	vec3 lcol = texture2DLod(luminanceMap, tc/screen_res, lum_lod).rgb;

	float lum = sqrt(lcol.r)*lum_quad.x+lcol.r*lcol.r*lum_quad.y+lcol.r*lum_quad.z;
	
	vec4 diff = texture2DRect(diffuseRect, vary_fragcoord.xy);
	vec4 spec = texture2DRect(specularRect, vary_fragcoord.xy);
	
	float ambocc = texture2DRect(lightMap, vary_fragcoord.xy).g;
			
	vec3 ccol = texture2DRect(giLightMap, vary_fragcoord.xy).rgb;
	vec3 gi_col = ccol;
	/*for (int i = -1; i <= 1; i+=1)
	{
		for (int j = -1; j <= 1; j+=1)
		{
			vec2 tc = vec2(i,j);
			float wght = 1.0/(length(tc)+1.0);
			gi_col += texture2DRect(giLightMap, vary_fragcoord.xy+vec2(i,j)).rgb * wght;
		}
	}*/
	
	//gi_col *= 1.0+spec.a*4.0;
	gi_col = (sqrt(gi_col)*gi_quad.x + gi_col*gi_quad.y)*(diff.rgb+spec.rgb*spec.a)+gi_quad.z*ambocc*ambient.rgb*diff.rgb;
	
	vec4 sun_col =	texture2DRect(sunLightMap, vary_fragcoord.xy);
	
	vec3 local_col = texture2DRect(localLightMap, vary_fragcoord.xy).rgb;
		

	float sun_lum = 1.0-lum;
	sun_lum = sun_lum*sun_lum*sun_lum_quad.x + sun_lum*sun_lum_quad.y+sun_lum_quad.z;
		
	float gi_lum = lum;
	gi_lum = gi_lum*gi_lum*gi_lum_quad.x+gi_lum*gi_lum_quad.y+gi_lum_quad.z;
	gi_col *= 1.0/gi_lum;
	
		
	vec3 col = sun_col.rgb*(1.0+max(sun_lum,0.0))+gi_col+local_col;
	
	gl_FragColor.rgb = col.rgb;
	gl_FragColor.a = max(sun_lum*min(sun_col.r+sun_col.g+sun_col.b, 1.0), sun_col.a);
	
	//gl_FragColor.rgb = texture2DRect(giLightMap, vary_fragcoord.xy).rgb;
	//gl_FragColor.rgb = vec3(texture2D(lightFunc, vary_fragcoord.xy/512.0-vec2(0.5, 0.5)).a);
}
