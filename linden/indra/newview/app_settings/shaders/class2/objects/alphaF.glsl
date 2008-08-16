vec4 getLightColor();
void applyScatter(inout vec3 col);

uniform samplerCube environmentMap;
uniform sampler2D diffuseMap;

void main() 
{
	vec4 diff = texture2D(diffuseMap, gl_TexCoord[0].xy);
	vec3 ref = textureCube(environmentMap, gl_TexCoord[1].xyz).rgb;
	vec4 specular = gl_TexCoord[2];
	vec3 col = mix(getLightColor().rgb * diff.rgb, ref, specular.a)+specular.rgb*diff.rgb;

	applyScatter(col);
		
	gl_FragColor.rgb = col.rgb;
	gl_FragColor.a = diff.a*gl_Color.a;	
}
