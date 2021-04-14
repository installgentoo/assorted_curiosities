#include "_main.h"
#include "math.h"
#define window_width 1024
#define window_height 1024

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(window_width, window_height, "Test", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glSetup(window_width, window_height);
  glewInit();
  Fluid mainfluid(window_width, window_height);
  Solid obsts(window_width, window_height);

  while (!glfwWindowShouldClose(window)) {
    mainfluid.Step(obsts);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
