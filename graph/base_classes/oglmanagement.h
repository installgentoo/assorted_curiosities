#pragma once

#include "gui/guicomponents.h"

struct GLFWwindow;

namespace glgui {
struct GUIBaseWindow;
}

namespace gl {

struct ApplicationManager {
  uint window_width() const { return m_window_width; }
  uint window_height() const { return m_window_height; }
  GLFWwindow *glfw_window() const { return m_glfw_window; }
  glgui::GUIBaseWindow &gui_base_window() const {
    CASSERT(s_window, "Basewindow uninitialized");
    return *s_window;
  }
  glgui::GUIResourceManager &gui_resource_manager() const;

  static ApplicationManager &Get();

  int ShouldTerminate();
  void ResizeWindow(int width, int height);
  void DrawToScreen(bool clear) const {
    GLState::BindOutputFramebuffer();
    GLState::Viewport(m_window_width, m_window_height);
    if (clear)
      GLState::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  void DrawMenu();
  void SwapBuffers();
  void Quit();
  vec2 GetMousePos() const;

  unique_ptr<Sampler> samlin, samnear;

private:
  static unique_ptr<glgui::GUIBaseWindow> s_window;
  uint m_window_width = 600, m_window_height = 480;
  GLFWwindow *m_glfw_window;

  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void MouseMoveCallback(GLFWwindow *window, double x, double y);
  static void ScrollCallback(GLFWwindow *window, double x, double y);
  static void WindowSizeChangeCallback(GLFWwindow *window, int width,
                                       int height);
  static void WindowLeftByCursorCallback(GLFWwindow *window, int aquired);
  static void WindowFocusCallback(GLFWwindow *window, int aquired);
  static void UnicodeCharacterCallback(GLFWwindow *window, uint character);

  ApplicationManager();
  ~ApplicationManager();
  ApplicationManager(const ApplicationManager &) = delete;
  ApplicationManager &operator=(const ApplicationManager &) = delete;
};

} // namespace gl
