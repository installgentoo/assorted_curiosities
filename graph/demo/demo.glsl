//--GLOBAL:
#version 330 core

//--GEO demo___graph_geom_shader
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 8) out;

vec2 catmullrom(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t)
{
  return 0.5 * (2 * p1 + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t + (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t);
}

void main()
{
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;

  gl_Position = p1;
  EmitVertex();

  const float t2 = float(1) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t2), 0., 1.);
  EmitVertex();

  const float t3 = float(2) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t3), 0., 1.);
  EmitVertex();

  const float t4 = float(3) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t4), 0., 1.);
  EmitVertex();

  const float t5 = float(4) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t5), 0., 1.);
  EmitVertex();

  const float t6 = float(5) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t6), 0., 1.);
  EmitVertex();

  const float t7 = float(6) / 7;
  gl_Position = vec4(catmullrom(p0.xy, p1.xy, p2.xy, p3.xy, t7), 0., 1.);
  EmitVertex();

  gl_Position = p2;
  EmitVertex();

  EndPrimitive();
}
