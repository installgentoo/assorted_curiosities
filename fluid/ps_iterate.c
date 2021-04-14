uniform sampler2D pmap;
uniform sampler2D dmap;
uniform sampler2D solid;
uniform vec2 window;
uniform float a;
uniform float invb;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;
  vec4 pU = textureOffset(pmap, pos, ivec2(0, 1));
  vec4 pD = textureOffset(pmap, pos, ivec2(0, -1));
  vec4 pL = textureOffset(pmap, pos, ivec2(1, 0));
  vec4 pR = textureOffset(pmap, pos, ivec2(-1, 0));
  vec4 pC = texture(pmap, pos);

  if(textureOffset(solid, pos, ivec2(0, 1)).x > 0.0)  pU = pC;
  if(textureOffset(solid, pos, ivec2(0, -1)).x > 0.0) pD = pC;
  if(textureOffset(solid, pos, ivec2(1, 0)).x > 0.0)  pL = pC;
  if(textureOffset(solid, pos, ivec2(-1, 0)).x > 0.0) pR = pC;

  vec4 bC = texture(dmap, pos);
  gl_FragColor = (pR + pL + pD + pU + a * bC) * invb;
}
