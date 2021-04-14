uniform sampler2D velmap;
uniform sampler2D tmap;
uniform sampler2D densmap;
uniform vec2 window;
uniform float frame;
uniform float buoyancy;
uniform float weight;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;
  float temp = texture(tmap, pos).r;

  gl_FragColor = texture(velmap, pos);

  if(temp > 0.0)
    {
      gl_FragColor += vec4((frame * temp * buoyancy - weight)*vec2(0.0,1.0), 0.0, 1.0);
    }
}
