#include "fluid.h"

Solid::Solid(int w, int h) {
  surface = new texmap(w, h, 1);
  border = new Shader("./vs_basic.c", "./ps_border.c");
  border->Use();
  glBindFramebuffer(GL_FRAMEBUFFER, surface->pixbuff);
  plane.Draw();
}

Solid::~Solid() {
  delete (surface);
  delete (border);
}

Fluid::Fluid(int w, int h) {
  velmap = new mapbuffer(w, h, 2), densmap = new mapbuffer(w, h, 1),
  pmap = new mapbuffer(w, h, 1), tmap = new mapbuffer(w, h, 1);
  dmap = new texmap(w, h, 1);

  tmap->one->Clear(0.0);

  render = new Shader("./vs_basic.c", "./ps_render.c");
  move = new Shader("./vs_basic.c", "./ps_move.c");
  forces = new Shader("./vs_basic.c", "./ps_forces.c");
  intake = new Shader("./vs_basic.c", "./ps_intake.c");
  decompose = new Shader("./vs_basic.c", "./ps_decompose.c");
  iterate = new Shader("./vs_basic.c", "./ps_iterate.c");
  substract = new Shader("./vs_basic.c", "./ps_substract.c");
  move->Log();
  forces->Log();
  intake->Log();
  decompose->Log();
  iterate->Log();
  substract->Log();

  move->Use();
  glUniform1i(glGetUniformLocation(move->prog, "velmap"), 0);
  glUniform1i(glGetUniformLocation(move->prog, "srcmap"), 1);
  glUniform1i(glGetUniformLocation(move->prog, "solid"), 2);
  glUniform2f(glGetUniformLocation(move->prog, "window"), w, h);
  glUniform1f(glGetUniformLocation(move->prog, "frame"), frame);

  forces->Use();
  glUniform1i(glGetUniformLocation(forces->prog, "velmap"), 0);
  glUniform1i(glGetUniformLocation(forces->prog, "tmap"), 1);
  glUniform1i(glGetUniformLocation(forces->prog, "densmap"), 2);
  glUniform2f(glGetUniformLocation(forces->prog, "window"), w, h);
  glUniform1f(glGetUniformLocation(forces->prog, "frame"), frame);
  glUniform1f(glGetUniformLocation(forces->prog, "buoyancy"), buoyancy);
  glUniform1f(glGetUniformLocation(forces->prog, "weight"), weight);

  intake->Use();
  glUniform1i(glGetUniformLocation(intake->prog, "src"), 0);
  glUniform2f(glGetUniformLocation(intake->prog, "window"), w, h);
  glUniform2f(glGetUniformLocation(intake->prog, "center"), w / 2, w / 2);
  glUniform1f(glGetUniformLocation(intake->prog, "size"), (float)w / 30.0f);

  decompose->Use();
  glUniform1i(glGetUniformLocation(decompose->prog, "velmap"), 0);
  glUniform2f(glGetUniformLocation(decompose->prog, "window"), w, h);
  glUniform1i(glGetUniformLocation(decompose->prog, "solid"), 1);
  glUniform1f(glGetUniformLocation(decompose->prog, "halfinvsize"),
              0.5f / cell);

  iterate->Use();
  glUniform1i(glGetUniformLocation(iterate->prog, "pmap"), 0);
  glUniform1i(glGetUniformLocation(iterate->prog, "dmap"), 1);
  glUniform1i(glGetUniformLocation(iterate->prog, "solid"), 2);
  glUniform2f(glGetUniformLocation(iterate->prog, "window"), w, h);
  glUniform1f(glGetUniformLocation(iterate->prog, "a"), -cell * cell);
  glUniform1f(glGetUniformLocation(iterate->prog, "invb"), 0.25f);

  substract->Use();
  glUniform1i(glGetUniformLocation(substract->prog, "velmap"), 0);
  glUniform1i(glGetUniformLocation(substract->prog, "pmap"), 1);
  glUniform1i(glGetUniformLocation(substract->prog, "solid"), 2);
  glUniform2f(glGetUniformLocation(substract->prog, "window"), w, h);
  glUniform1f(glGetUniformLocation(substract->prog, "scale"), scale);

  render->Use();
  glUniform1i(glGetUniformLocation(render->prog, "text"), 0);
  glUniform2f(glGetUniformLocation(render->prog, "window"), w, h);
}

Fluid::~Fluid() {
  delete (velmap);
  delete (densmap);
  delete (pmap);
  delete (tmap);
  delete (dmap);
  delete (render);
  delete (move);
  delete (forces);
  delete (intake);
  delete (decompose);
  delete (iterate);
  delete (substract);
  delete (render);
}

void Fluid::Move(texmap *velmap, texmap *src, texmap *dest, texmap *solidmap) {
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, velmap->text);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, src->text);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, solidmap->text);
}

void Fluid::Iterate(texmap *pmap, texmap *dmap, texmap *dest,
                    texmap *solidmap) {
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pmap->text);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, dmap->text);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, solidmap->text);
}

void Fluid::Substract(texmap *velmap, texmap *pmap, texmap *dest,
                      texmap *solidmap) {
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, velmap->text);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, pmap->text);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, solidmap->text);
}

void Fluid::Decompose(texmap *velmap, texmap *dest, texmap *solidmap) {
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, velmap->text);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, solidmap->text);
}

void Fluid::Intake(texmap *dest) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, dest->text);
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
}

void Fluid::Forces(texmap *velmap, texmap *tmap, texmap *denmap, texmap *dest) {
  glBindFramebuffer(GL_FRAMEBUFFER, dest->pixbuff);
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, velmap->text);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tmap->text);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, denmap->text);
}

void Fluid::Step(const Solid &solidmap) {
  move->Use();
  glUniform1f(glGetUniformLocation(move->prog, "dissip"), veldiss);
  Move(velmap->one, velmap->one, velmap->two, solidmap.surface);
  plane.Draw();
  velmap->Swap();
  glUniform1f(glGetUniformLocation(move->prog, "dissip"), tdiss);
  Move(velmap->one, tmap->one, tmap->two, solidmap.surface);
  plane.Draw();
  tmap->Swap();
  glUniform1f(glGetUniformLocation(move->prog, "dissip"), ddiss);
  Move(velmap->one, densmap->one, densmap->two, solidmap.surface);
  plane.Draw();
  densmap->Swap();

  forces->Use();
  Forces(velmap->one, tmap->one, densmap->one, velmap->two);
  plane.Draw();
  velmap->Swap();

  intake->Use();
  Intake(tmap->one);
  glUniform1f(glGetUniformLocation(intake->prog, "initval"), inittemp);
  plane.Draw();
  Intake(densmap->one);
  glUniform1f(glGetUniformLocation(intake->prog, "initval"), initdens);
  plane.Draw();

  decompose->Use();
  Decompose(velmap->one, dmap, solidmap.surface);
  plane.Draw();
  pmap->one->Clear(0.0f);

  for (int i = 0; i < numiter; i++) {
    iterate->Use();
    Iterate(pmap->one, dmap, pmap->two, solidmap.surface);
    plane.Draw();
    pmap->Swap();
  }

  substract->Use();
  Substract(velmap->one, pmap->one, velmap->two, solidmap.surface);
  plane.Draw();
  velmap->Swap();

  glActiveTexture(GL_TEXTURE0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, densmap->one->text);
  render->Use();
  plane.Draw();
}
