uniform sampler2D diffuseMap;

void main()
{
	vec4 color1 = texture2D(diffuseMap, gl_TexCoord[0].xy);
	vec4 color2 = texture2D(diffuseMap, gl_TexCoord[1].xy);
	vec4 color3 = texture2D(diffuseMap, gl_TexCoord[2].xy);
	vec4 color4 = texture2D(diffuseMap, gl_TexCoord[3].xy);
	vec4 color5 = texture2D(diffuseMap, gl_TexCoord[4].xy);
							
	vec4 col = (color1+color2+color3+color4+color5)*0.21;
	col = max(col, col*0.25 + color5*0.75);
	gl_FragColor = col;
}
