#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <particleSystem_class.h>

#define window_width 640
#define window_height 480
psys *psystem;

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void glSetup(int width, int height) {
  glViewport(0, 0, width, height);
  gluPerspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
  glRotatef(20, 1, 0, 0);
  glTranslatef(-1.0f, -2.0f, -4.0f);
}

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(window_width, window_height, "Particles", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glSetup(window_width, window_height);
  glewInit();
  startupOpenCL();

  psystem = new psys(4096, 64, 0.02f, 0.4f, 0.1f, 0.01f);

  while (!glfwWindowShouldClose(window)) {
    psystem->update(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
