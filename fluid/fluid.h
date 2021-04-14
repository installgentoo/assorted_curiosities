#ifndef FLUID_H_
#define FLUID_H_

#include "mapbuffer.h"
#include "shader.h"
#include "support.h"

class Solid {
public:
  Quad plane;
  Shader *border;
  texmap *surface;
  Solid(int w, int h);
  ~Solid();
};

class Fluid {
public:
  Shader *render, *move, *forces, *intake, *decompose, *iterate, *substract;
  mapbuffer *velmap, *densmap, *pmap, *tmap;
  texmap *dmap;
  Quad plane;
  Fluid(int w, int h);
  ~Fluid();
  void Step(const Solid &solidmap);
  void Move(texmap *velmap, texmap *src, texmap *dest, texmap *solidmap);
  void Iterate(texmap *pmap, texmap *dmap, texmap *dest, texmap *solidmap);
  void Substract(texmap *velmap, texmap *pmap, texmap *dest, texmap *solidmap);
  void Decompose(texmap *velmap, texmap *dest, texmap *solidmap);
  void Intake(texmap *dest);
  void Forces(texmap *velmap, texmap *tmap, texmap *denmap, texmap *dest);

#define cell 1.25f
  static const int numiter = 40;
  static const float inittemp = 11.0f;
  static const float initdens = 1.0f;
  static const float frame = 0.125f;
  static const float buoyancy = 1.0f;
  static const float weight = 0.2f;
  static const float scale = 1.125f / cell;
  static const float tdiss = 0.999f;
  static const float veldiss = 0.99f;
  static const float ddiss = 0.9999f;
};

#endif
