/** 
 * @file avatarF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2D diffuseMap;

varying vec3 vary_normal;

void main() 
{
	vec4 diff = gl_Color*texture2D(diffuseMap, gl_TexCoord[0].xy);
	// Viewer 2.0 uses 0.2 but for KL's viewer if i want a complete avatar need this to be 0.0 for now.
	if (diff.a < 0.0)
	{
		discard;
	}
	
	gl_FragData[0] = vec4(diff.rgb, 1.0);
	gl_FragData[1] = vec4(0,0,0,0);
	gl_FragData[2] = vec4(normalize(vary_normal)*0.5+0.5, 0.0);
}

