/** 
 * @file avatarAlphaF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2D diffuseMap;
uniform sampler2DRectShadow shadowMap0;
uniform sampler2DRectShadow shadowMap1;
uniform sampler2DRectShadow shadowMap2;
uniform sampler2DRectShadow shadowMap3;
uniform sampler2D noiseMap;

uniform mat4 shadow_matrix[6];
uniform vec4 shadow_clip;
uniform vec2 screen_res;

vec3 atmosLighting(vec3 light);
vec3 scaleSoftClip(vec3 light);

varying vec3 vary_ambient;
varying vec3 vary_directional;
varying vec4 vary_position;
varying vec3 vary_normal;

void main() 
{
	float shadow = 1.0;
	vec4 pos = vary_position;
	vec3 norm = normalize(vary_normal);
	
	vec3 nz = texture2D(noiseMap, gl_FragCoord.xy/128.0).xyz;

	vec4 spos = pos;
	
	if (spos.z > -shadow_clip.w)
	{	
		vec4 lpos;
		
		if (spos.z < -shadow_clip.z)
		{
			lpos = shadow_matrix[3]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap3, lpos).x;
			shadow += max((pos.z+shadow_clip.z)/(shadow_clip.z-shadow_clip.w)*2.0-1.0, 0.0);
		}
		else if (spos.z < -shadow_clip.y)
		{
			lpos = shadow_matrix[2]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap2, lpos).x;
		}
		else if (spos.z < -shadow_clip.x)
		{
			lpos = shadow_matrix[1]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap1, lpos).x;
		}
		else
		{
			lpos = shadow_matrix[0]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap0, lpos).x;
		}
	}
	
	
	vec4 col = vec4(vary_ambient + vary_directional*shadow, gl_Color.a);	
	vec4 color = texture2D(diffuseMap, gl_TexCoord[0].xy) * col;
	
	color.rgb = atmosLighting(color.rgb);

	color.rgb = scaleSoftClip(color.rgb);

	gl_FragColor = color;
}
