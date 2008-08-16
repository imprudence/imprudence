uniform float delta;
void main() 
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy + vec2(delta, delta);
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy + vec2(-delta, delta);
	gl_TexCoord[2].xy = gl_MultiTexCoord0.xy + vec2(-delta, -delta);
	gl_TexCoord[3].xy = gl_MultiTexCoord0.xy + vec2(delta, -delta);
	gl_TexCoord[4].xy = gl_MultiTexCoord0.xy;
}
