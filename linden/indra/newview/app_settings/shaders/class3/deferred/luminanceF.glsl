/** 
 * @file luminanceF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2DRect diffuseMap;

varying vec2 vary_fragcoord;
uniform float fade;
void main() 
{
	gl_FragColor.rgb = texture2DRect(diffuseMap, vary_fragcoord.xy).rgb;
	gl_FragColor.a = fade;
}
