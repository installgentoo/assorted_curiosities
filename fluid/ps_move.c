uniform sampler2D velmap;
uniform sampler2D srcmap;
uniform sampler2D solid;
uniform vec2 window;
uniform float frame;
uniform float dissip;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;

  if(texture(solid, pos).x > 0.0)
    {
      gl_FragColor = vec4(0.0);
      return;
    }

  vec2 rv = texture(velmap, pos).xy;
  vec2 coord = (gl_FragCoord.xy - frame * rv);
  gl_FragColor = texture(srcmap, 1.0 / window * coord) * dissip;
}
