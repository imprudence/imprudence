/** 
 * @file postgiV.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

varying vec3 vary_normal;

void main()
{
	//transform vertex
	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex; 
	gl_Position = pos;
	gl_TexCoord[0].xy = pos.xy*0.5+0.5;
}
