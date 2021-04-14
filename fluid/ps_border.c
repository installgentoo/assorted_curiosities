void main()
{
  if(length(gl_FragCoord - vec2(512.0,512.0)) > 511.0) gl_FragColor = vec4(1.0);
  else gl_FragColor = vec4(0.0);
}
