uniform sampler2D scatterMap;

//for per-pixel scatter
vec4 getScatter(vec3 viewVec, vec3 lightDir)
{
   vec2 scatterCoord;
   scatterCoord.x = length(viewVec);
   vec3 normVec = viewVec / scatterCoord.x;
   scatterCoord.y = dot(normVec, lightDir)*0.5 + 0.5;   
   scatterCoord.x = scatterCoord.x / gl_Fog.end;
   scatterCoord.x *= scatterCoord.x; // HACK!! Remove this when we can push the view distance farther out
   return texture2D(scatterMap, scatterCoord) * vec4(gl_Fog.color.rgb, 1.0);
}

void applyScatter(inout vec3 color, vec4 haze)
{
	color.rgb = haze.rgb + haze.a * color.rgb;   
}

//for per-vertex scatter
void applyScatter(inout vec3 color)
{
	vec4 haze = texture2D(scatterMap, gl_TexCoord[5].xy) * vec4(gl_Fog.color.rgb, 1.0);
	color.rgb = haze.rgb + haze.a * color.rgb;
}
