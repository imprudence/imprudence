vec4 getLightColor();
void applyScatter(inout vec3 col);

uniform samplerCube environmentMap;
uniform sampler2D diffuseMap;
uniform sampler2D bumpMap;


void main() 
{
	vec4 diff = texture2D(diffuseMap, gl_TexCoord[0].xy);
	vec3 ref = textureCube(environmentMap, gl_TexCoord[1].xyz).rgb;
	
	vec4 specular = gl_TexCoord[3];
		
	float b = texture2D(bumpMap, gl_TexCoord[0].xy).a;
	b -= texture2D(bumpMap, gl_TexCoord[2].xy).a;
	
	vec3 col = mix(getLightColor().rgb * diff.rgb, ref, specular.a)+specular.rgb*diff.rgb;
	col += col * b;
	float m = (col.r + col.g + col.b);
	m *= 1.0/3.0;
	col = mix(col, vec3(m), -specular.a*specular.a);
	
	applyScatter(col);
			
	gl_FragColor.rgb = col;
	gl_FragColor.a = diff.a;
}
