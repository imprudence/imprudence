vec4 calcLighting(vec3 pos, vec3 norm, vec4 color, vec3 baseCol);
mat4 getSkinnedTransform();
void default_scatter(vec3 viewVec, vec3 lightDir);

attribute vec4 materialColor;
attribute vec4 binormal; 

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
				
	vec4 pos;
	mat4 trans = getSkinnedTransform();
	pos.x = dot(trans[0], gl_Vertex);
	pos.y = dot(trans[1], gl_Vertex);
	pos.z = dot(trans[2], gl_Vertex);
	pos.w = 1.0;
	
	vec3 norm;
	norm.x = dot(trans[0].xyz, gl_Normal);
	norm.y = dot(trans[1].xyz, gl_Normal);
	norm.z = dot(trans[2].xyz, gl_Normal);
	norm = normalize(norm);
		
	vec3 binorm;
	binorm.x = dot(trans[0].xyz, binormal.xyz);
	binorm.y = dot(trans[1].xyz, binormal.xyz);
	binorm.z = dot(trans[2].xyz, binormal.xyz);
	
	float spec = 1.0-max(dot(reflect(normalize(pos.xyz), norm),gl_LightSource[0].position.xyz), 0.0);
	spec *= spec;
	spec = 1.0-spec;  
		
	vec4 color = calcLighting(pos.xyz, norm, materialColor, gl_Color.rgb);			
	gl_FrontColor = color; 
		
	gl_Position = gl_ProjectionMatrix * pos;
		
	vec3 N = norm;
	vec3 B = normalize(binorm);
	vec3 T = cross(N,B);
	
	//gl_TexCoord[1].xy = gl_MultiTexCoord0.xy + 1.0/512.0 * vec2(dot(T,gl_LightSource[0].position.xyz),
	//												dot(B,gl_LightSource[0].position.xyz));
									 
	
	default_scatter(pos.xyz, gl_LightSource[0].position.xyz);
}