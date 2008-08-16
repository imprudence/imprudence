vec4 calcLightingSpecular(vec3 pos, vec3 norm, vec4 color, inout vec4 specularColor, vec4 baseCol);
void default_scatter(vec3 viewVec, vec3 lightDir);

attribute vec4 materialColor;
attribute vec4 specularColor;

void main()
{
	//transform vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	vec3 pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vec3 norm = normalize(gl_NormalMatrix * gl_Normal);
	vec4 spec = specularColor;
	gl_FrontColor.rgb = calcLightingSpecular(pos, norm, materialColor, spec, gl_Color).rgb;			
	gl_FrontColor.a = materialColor.a;
	gl_TexCoord[2] = spec;
	vec3 ref = reflect(pos,norm);
	gl_TexCoord[1] = gl_TextureMatrix[1]*vec4(ref,1);
		
	default_scatter(pos.xyz, gl_LightSource[0].position.xyz);
}

