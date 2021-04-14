uniform sampler2D src;
uniform vec2 window;
uniform vec2 center;
uniform float size;
uniform float initval;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;
  float d = distance(center, gl_FragCoord.xy);
  vec4 tsrc = texture(src, pos);

  if(d < size)
    {
      float a = min((size - d) * 0.5, 1.0);
      gl_FragColor = vec4(vec3(initval, initval, initval)*a + tsrc.xyz*(1.0 - a), 1.0);
    }
  else
    {
      gl_FragColor = tsrc;
    }
}
