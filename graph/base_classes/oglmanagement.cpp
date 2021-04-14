#include "oglmanagement.h"

#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>

#include "gui/guielements.h"
#include "gui/guimenu.h"

using namespace gl;
using namespace glgui;

unique_ptr<GUIBaseWindow> ApplicationManager::s_window;

GUIResourceManager &ApplicationManager::gui_resource_manager() const {
  CASSERT(s_window && s_window->m_resource_manager,
          "GUIResourceManager uninitialized");
  return *s_window->m_resource_manager;
}

ApplicationManager &ApplicationManager::Get() {
  static ApplicationManager s_manager;
  return s_manager;
}

int ApplicationManager::ShouldTerminate() {
  return glfwWindowShouldClose(m_glfw_window);
}

void ApplicationManager::ResizeWindow(int width, int height) {
  glfwSetWindowSize(m_glfw_window, width, height);
  auto &app_manager = ApplicationManager::Get();
  app_manager.m_window_width = cast<uint>(width);
  app_manager.m_window_height = cast<uint>(height);
}

void ApplicationManager::DrawMenu() {
  GLState::Disable(GL_DEPTH_TEST, GL_CULL_FACE);
  s_window->Draw();
  GLState::Enable(GL_DEPTH_TEST, GL_CULL_FACE);
}

void ApplicationManager::SwapBuffers() {
  glfwSwapBuffers(m_glfw_window);
  glfwPollEvents();
}

void ApplicationManager::Quit() {
  glfwSetWindowShouldClose(m_glfw_window, GL_TRUE);
}

glm::vec2 ApplicationManager::GetMousePos() const {
  double x, y;
  glfwGetCursorPos(m_glfw_window, &x, &y);
  return vec2(x * 2 / m_window_width - 1., 1. - y * 2 / m_window_height);
}

void ApplicationManager::KeyCallback(GLFWwindow *, int key, int scancode,
                                     int action, int mods) {
  s_window->ReceiveKey(key, action, mods);
}

void ApplicationManager::MouseButtonCallback(GLFWwindow *, int button,
                                             int action, int mods) {
  s_window->ReceiveMouseButton(button, mods, action);
}

void ApplicationManager::MouseMoveCallback(GLFWwindow *window, double x,
                                           double y) {
  // focus policy
  /*  auto &app_manager = ApplicationManager::Get();
s_window->ReceiveMouseMove(vec2(x * 2 / app_manager.m_window_width - 1., 1. - y
* 2 / app_manager.m_window_height));*/
}

void ApplicationManager::ScrollCallback(GLFWwindow *window, double x,
                                        double y) {
  s_window->ReceiveMouseWheel(x, y);
}

void ApplicationManager::WindowSizeChangeCallback(GLFWwindow *window, int width,
                                                  int height) {}

void ApplicationManager::WindowLeftByCursorCallback(GLFWwindow *, int aquired) {
  if (GL_FALSE == aquired)
    s_window->DefocusChild();
}

void ApplicationManager::WindowFocusCallback(GLFWwindow *window, int aquired) {
  if (GL_FALSE == aquired) {
    s_window->m_pause = false;
    s_window->DefocusChild();
  } else
    s_window->m_pause = true;
}

void ApplicationManager::UnicodeCharacterCallback(GLFWwindow *,
                                                  uint character) {
  s_window->ReceiveKey(character);
}

ApplicationManager::ApplicationManager() {
  if (!glfwInit())
    CERROR("Error initializing glfw");
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  m_glfw_window =
      glfwCreateWindow(cast<int>(m_window_width), cast<int>(m_window_height),
                       "Engine", nullptr, nullptr);
  if (!m_glfw_window) {
    glfwTerminate();
    CERROR("Error creating window");
  }
  glfwMakeContextCurrent(m_glfw_window);
  glfwSetWindowSizeCallback(m_glfw_window,
                            ApplicationManager::WindowSizeChangeCallback);
  glfwSetKeyCallback(m_glfw_window, ApplicationManager::KeyCallback);
  glfwSetMouseButtonCallback(m_glfw_window,
                             ApplicationManager::MouseButtonCallback);
  glfwSetCursorPosCallback(m_glfw_window,
                           ApplicationManager::MouseMoveCallback);
  glfwSetScrollCallback(m_glfw_window, ApplicationManager::ScrollCallback);
  glfwSetCharCallback(m_glfw_window,
                      ApplicationManager::UnicodeCharacterCallback);
  glfwSwapInterval(1);

  if (gl3wInit())
    CERROR("GL3W failed to initialize OpenGL");
  if (!gl3wIsSupported(3, 3))
    CERROR("Opengl 3.3 not supported");

  GLState::Enable(GL_DEPTH_TEST, GL_MULTISAMPLE);
  GLState::Disable(GL_BLEND);
  GLCHECK(glClearColor(0.f, 0.f, 0.f, 1.f));

  CINFO("OpenGL initialized");
  s_window.reset(new GUIBaseWindow(*this));

  samlin.reset(new Sampler("2d_clamp_linear", GL_TEXTURE_WRAP_S,
                           GL_CLAMP_TO_EDGE, GL_TEXTURE_WRAP_T,
                           GL_CLAMP_TO_EDGE, GL_TEXTURE_MIN_FILTER, GL_LINEAR,
                           GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  samnear.reset(new Sampler("2d_clamp_nearest", GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE, GL_TEXTURE_WRAP_T,
                            GL_CLAMP_TO_EDGE, GL_TEXTURE_MIN_FILTER, GL_NEAREST,
                            GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

ApplicationManager::~ApplicationManager() {
  s_window.reset();
  glfwDestroyWindow(m_glfw_window);
  glfwTerminate();
  CINFO("Terminated glfw");
}
