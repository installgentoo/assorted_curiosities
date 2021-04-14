#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <cl.h>
#include <math.h>
#include <pthread.h>
void startupOpenCL();
const uint flSize = sizeof(float);
const uint uiSize = sizeof(uint);
const uint cmSize = sizeof(cl_mem);

typedef struct {
  uint nPart;
  float pRad, Spring, Shear, Attract, cellSize;
} tSimParams;

class psys {
public:
  psys(uint nPart, uint gridSize, float pRad, float Spring, float Shear,
       float Attract);
  void update(GLFWwindow *window);
  void draw();
  friend void *proxy_func0(void *);
  pthread_t thread;
  tSimParams parm;
  float *c_Pos, *c_Vel;
  cl_mem g_Pos, g_Vel;
};
