void default_scatter(vec3 viewVec, vec3 lightDir);

uniform vec2 d1;
uniform vec2 d2;
uniform float time;
uniform vec3 eyeVec;

float wave(vec2 v, float t, float f, vec2 d, float s) 
{
   return (dot(d, v)*f + t*s)*f;
}

void main()
{
   //transform vertex
   vec4 position = gl_Vertex;
   mat4 modelViewProj = gl_ModelViewProjectionMatrix;
   vec4 oPosition = modelViewProj * position;
   vec3 oRefCoord = oPosition.xyz + vec3(0, 0, 0.2);
      
   //get view vector
   vec4 oEyeVec;
   oEyeVec.xyz = position.xyz-eyeVec;
      
   //get wave position parameter (create sweeping horizontal waves)
   vec3 v = position.xyz;
   v.x += (cos(v.x*0.08+time*0.01)+sin(v.y*0.02))*6.0;
      
   //get two normal map (detail map) texture coordinates
   vec2 oTexCoord = gl_MultiTexCoord0.xy;
   vec2 littleWave1 = (v.xy)*vec2(0.7, 1.5)+d2*time*0.065;
   vec2 littleWave2 = (v.xy)*vec2(0.07, 0.15)-d1*time*0.087;
         
   //pass wave parameters to pixel shader
   float t = time * 0.075;
   vec2 bigWave = (v.xy)*vec2(0.04,0.04)+d1*t;
      
   //pass color and fog color to pixel shader
   vec4 col = gl_Color;
   col.a = clamp(abs(dot(normalize(oEyeVec.xyz), vec3(0,0,1))),0.0,1.0);
   col.a = 1.0-col.a;
   col.a += 0.75;
   default_scatter((gl_ModelViewMatrix * gl_Vertex).xyz, gl_LightSource[0].position.xyz);
   
   gl_Position = oPosition;
   gl_TexCoord[0].xy = oTexCoord;
   gl_TexCoord[0].zw = littleWave1;
   gl_TexCoord[1].xy = littleWave2;
   gl_TexCoord[1].zw = bigWave;
   gl_TexCoord[2].xyz = oEyeVec.xyz;
   gl_TexCoord[3].xyz = oRefCoord;
   gl_FrontColor = col;
}
