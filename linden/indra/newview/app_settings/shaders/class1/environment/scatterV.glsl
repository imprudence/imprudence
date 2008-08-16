

vec2 getScatterCoord(vec3 viewVec, vec3 lightDir)
{
   vec2 scatterCoord = vec2(0,0);
   return scatterCoord;
}

void default_scatter(vec3 viewVec, vec3 lightDir)
{
	float f = gl_Fog.density * (gl_ModelViewProjectionMatrix * gl_Vertex).z;
	f = clamp(exp2(-f),0.0,1.0);
	gl_TexCoord[5].a = f;
	gl_TexCoord[5].rgb = gl_Fog.color.rgb;
}
