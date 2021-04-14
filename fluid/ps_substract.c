uniform sampler2D velmap;
uniform sampler2D pmap;
uniform sampler2D solid;
uniform vec2 window;
uniform float scale;

void main()
{
  vec2 pos = gl_FragCoord.xy / window;
  if(texture(solid, pos).x > 0.0)
    {
      gl_FragColor = vec4(1.0);
      return;
    }

  float pU = textureOffset(pmap, pos, ivec2(0, 1)).r;
  float pD = textureOffset(pmap, pos, ivec2(0, -1)).r;
  float pL = textureOffset(pmap, pos, ivec2(1, 0)).r;
  float pR = textureOffset(pmap, pos, ivec2(-1, 0)).r;
  float pC = texture(pmap, pos).r;

  vec2 solv = vec2(0);
  vec2 mask = vec2(1);

  if(textureOffset(solid, pos, ivec2(0, 1)).x > 0.0)  { pU = pC; solv.y = 1.0; mask.y = 0.0; }
  if(textureOffset(solid, pos, ivec2(0, -1)).x > 0.0) { pD = pC; solv.y = 1.0; mask.y = 0.0; }
  if(textureOffset(solid, pos, ivec2(1, 0)).x > 0.0)  { pL = pC; solv.x = 1.0; mask.x = 0.0; }
  if(textureOffset(solid, pos, ivec2(-1, 0)).x > 0.0) { pR = pC; solv.x = 1.0; mask.x = 0.0; }

  vec2 grad = vec2(pL - pR, pU - pD) * scale;
  vec2 fv = texture(velmap, pos).xy - grad;
  gl_FragColor = vec4((mask * fv) + solv, 0.0, 1.0);
}
