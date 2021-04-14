uniform sampler2D text;
uniform vec2 window;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;
  gl_FragColor = texture(text, pos);
}
