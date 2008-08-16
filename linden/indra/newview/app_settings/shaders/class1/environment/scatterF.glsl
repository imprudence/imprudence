uniform sampler2D scatterMap;

//for per-pixel scatter
vec4 getScatter(vec3 viewVec, vec3 lightDir)
{
   return gl_TexCoord[5];
}

void applyScatter(inout vec3 color)
{
	color = gl_TexCoord[5].a*color + (1.0-gl_TexCoord[5].a) * gl_TexCoord[5].rgb;
}

void applyScatter(inout vec3 color, vec4 haze)
{
	color.rgb = haze.rgb + haze.a * color.rgb;   
}
