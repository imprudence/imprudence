
vec2 getScatterCoord(vec3 viewVec, vec3 lightDir)
{
   vec2 scatterCoord;
   scatterCoord.x = length(viewVec);
   vec3 normVec = normalize(viewVec);
   scatterCoord.y = dot(normVec, lightDir)*0.5 + 0.5;   
   scatterCoord.x = scatterCoord.x / gl_Fog.end;
   scatterCoord.x *= scatterCoord.x; // HACK!! Remove this when we can push the view distance farther out
   return scatterCoord;
}

void default_scatter(vec3 viewVec, vec3 lightDir)
{
	gl_TexCoord[5].xy = getScatterCoord(viewVec, lightDir);
}
