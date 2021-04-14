uniform sampler2D velmap;
uniform sampler2D solid;
uniform vec2 window;
uniform float halfinvsize;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;

  float vU = textureOffset(velmap, pos, ivec2(0, 1)).y;
  float vD = textureOffset(velmap, pos, ivec2(0, -1)).y;
  float vL = textureOffset(velmap, pos, ivec2(1, 0)).x;
  float vR = textureOffset(velmap, pos, ivec2(-1, 0)).x;

  if(textureOffset(solid, pos, ivec2(0, 1)).x > 0.0)  vU = 1.0;
  if(textureOffset(solid, pos, ivec2(0, -1)).x > 0.0) vD = 1.0;
  if(textureOffset(solid, pos, ivec2(1, 0)).x > 0.0)  vL = 1.0;
  if(textureOffset(solid, pos, ivec2(-1, 0)).x > 0.0) vR = 1.0;

  gl_FragColor = vec4(halfinvsize * (vL - vR + vU - vD), 0.0, 0.0, 1.0);
}
