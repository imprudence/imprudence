vec4 calcLightingSpecular(vec3 pos, vec3 norm, vec4 color, inout vec4 specularColor, vec4 baseCol);
void default_scatter(vec3 viewVec, vec3 lightDir);

attribute vec4 materialColor;
attribute vec4 specularColor;
attribute vec4 binormal;

void main()
{
	//transform vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	vec3 pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vec3 norm = normalize(gl_NormalMatrix * gl_Normal);
	vec3 binorm = normalize(gl_NormalMatrix * binormal.xyz);
	vec3 tangent = cross(binorm, norm);
	binorm.xy = vec2(dot(tangent, gl_LightSource[0].position.xyz),
					dot(binorm, gl_LightSource[0].position.xyz))*1.0/128.0;
	
	vec4 spec = specularColor;
	gl_FrontColor.rgb = calcLightingSpecular(pos, norm, materialColor, spec, gl_Color).rgb;			
	gl_TexCoord[3] = spec;
	gl_FrontColor.a = materialColor.a;
	vec3 ref = reflect(pos,norm);
	gl_TexCoord[1].xyz = (gl_TextureMatrix[1]*vec4(ref,1.0)).xyz;
	gl_TexCoord[2].xy = binorm.xy + gl_MultiTexCoord0.xy;
	
	default_scatter(pos.xyz, gl_LightSource[0].position.xyz);
}

