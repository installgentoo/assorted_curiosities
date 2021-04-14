#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <particleSystem_class.h>
#include <particle_system.cpp>

GLuint vbo;

size_t wgSize = 128, glSize0;

inline size_t uSnap(size_t a, size_t b) {
  return ((a % b) == 0) ? a : (a - (a % b) + b);
}

void *proxy_func0(void *ptr) {
  static_cast<psys *>(ptr)->draw();
  return NULL;
}

void psys::draw() {
  clSetKernelArg(ckGrav, 0, cmSize, (void *)&g_Pos);
  clSetKernelArg(ckGrav, 1, cmSize, (void *)&g_Vel);
  clSetKernelArg(ckGrav, 2, cmSize, (void *)&params);
  clEnqueueNDRangeKernel(cqQueue0, ckGrav, 1, NULL, &glSize0, &wgSize, 0, NULL,
                         NULL);

  clSetKernelArg(ckCollide, 0, cmSize, (void *)&g_Pos);
  clSetKernelArg(ckCollide, 1, cmSize, (void *)&g_Vel);
  clSetKernelArg(ckCollide, 2, cmSize, (void *)&params);
  clEnqueueNDRangeKernel(cqQueue0, ckCollide, 1, NULL, &glSize0, &wgSize, 0,
                         NULL, NULL);

  clEnqueueReadBuffer(cqQueue1, g_Pos, CL_TRUE, 0, parm.nPart * 2u * flSize,
                      c_Pos, 0, NULL, NULL);
}

void psys::update(GLFWwindow *window) {
  /*  pthread_join(thread,NULL);
  pthread_create(&thread,NULL,proxy_func0,this);
*/
  draw();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBufferData(GL_ARRAY_BUFFER, parm.nPart * flSize * 2u, c_Pos,
               GL_STREAM_DRAW);
  glDrawArrays(GL_POINTS, 0, parm.nPart);
  glfwSwapBuffers(window);
}

psys::psys(uint nPart, uint gridSize, float particleradius, float Spring,
           float Shear, float Attract) {
  parm.pRad = particleradius;
  parm.nPart = nPart;
  parm.cellSize = parm.pRad * 2.0f;
  parm.Spring = Spring;
  parm.Shear = Shear;
  parm.Attract = Attract;

  c_Pos = (float *)malloc(parm.nPart * 2u * flSize);
  c_Vel = (float *)malloc(parm.nPart * 2u * flSize);
  for (uint y = 0u; y < gridSize; y++)
    for (uint x = 0u; x < gridSize; x++) {
      uint idx = y * gridSize + x;
      if (idx < parm.nPart) {
        c_Pos[idx * 2u] = parm.pRad * 2.0f * x;
        c_Pos[idx * 2u + 1u] = parm.pRad * 2.0f * y;
      }
    }

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 2u * flSize, 0);

  g_Pos =
      clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     parm.nPart * 2u * flSize, NULL, NULL);
  g_Vel =
      clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     parm.nPart * 2u * flSize, NULL, NULL);
  clEnqueueWriteBuffer(cqQueue0, g_Pos, CL_TRUE, 0, parm.nPart * 2u * flSize,
                       c_Pos, 0, NULL, NULL);
  clEnqueueWriteBuffer(cqQueue0, g_Vel, CL_TRUE, 0, parm.nPart * 2u * flSize,
                       c_Vel, 0, NULL, NULL);
  clEnqueueWriteBuffer(cqQueue0, params, CL_TRUE, 0, sizeof(tSimParams), &parm,
                       0, NULL, NULL);
  glSize0 = uSnap(parm.nPart, wgSize);
}
