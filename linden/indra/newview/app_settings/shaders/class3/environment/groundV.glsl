varying vec3 lightd;
varying vec3 viewVec;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	lightd = gl_LightSource[0].position.xyz;
	viewVec = (gl_ModelViewMatrix * gl_Vertex).xyz;
	
	gl_FrontColor = gl_Color;
}
