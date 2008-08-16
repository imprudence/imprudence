vec4 getScatter(vec3 viewVec, vec3 lightDir);

varying vec3 lightd;
varying vec3 viewVec;

void main() 
{
   vec4 color = gl_Color;
   vec4 haze = getScatter(viewVec, lightd) * vec4(gl_Fog.color.rgb, 1.0);
   color.rgb = haze.rgb + haze.a * color.rgb;
   gl_FragColor = color;
}
