#include "guimenu.h"

#include <GLFW/glfw3.h>

using namespace glgui;

void GUIBaseWindow::AddMenu(GUILayout *layout) { m_layouts.push_back(layout); }

void GUIBaseWindow::AddGlobalKeyMapping(int key, function<void()> func) {
  m_global_key_mapping[key] = func;
}

void GUIBaseWindow::Draw() const {
  GLState::Enable(GL_BLEND);
  GLCHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  for (auto i : m_layouts)
    if (i->visible())
      i->Draw();
  GLState::Disable(GL_BLEND);
}

void GUIBaseWindow::MoveMouse() {
  if (!m_movement_tracked) {
    double x, y;
    glfwGetCursorPos(m_app_manager.glfw_window(), &x, &y);
    ReceiveMouseMove(vec2(vec2(x * 2 / m_app_manager.window_width() - 1.,
                               1. - y * 2 / m_app_manager.window_height())),
                     true);
  }
}

void GUIBaseWindow::ReceiveMouseButton(int button, int mods, int action) {
  MoveMouse();
  if (m_focused_child != -1 &&
      m_layouts[cast<size_t>(m_focused_child)]->visible())
    m_layouts[cast<size_t>(m_focused_child)]->ReceiveMouseButton(button, mods,
                                                                 action);
  else
    PassToWorld();
}

void GUIBaseWindow::ReceiveKey(int key, int action, int mods) {
  // focus policy
  //  MoveMouse();
  if (GLFW_PRESS == action)
    for (auto i : m_global_key_mapping)
      if (key == i.first) {
        i.second();
        return;
      }
  if (m_focused_child != -1 &&
      m_layouts[cast<size_t>(m_focused_child)]->visible())
    m_layouts[cast<size_t>(m_focused_child)]->ReceiveKey(key, action, mods);
  else
    PassToWorld();
}

void GUIBaseWindow::ReceiveKey(uint character) {
  // focus policy
  //  MoveMouse();
  if (m_focused_child != -1 &&
      m_layouts[cast<size_t>(m_focused_child)]->visible())
    m_layouts[cast<size_t>(m_focused_child)]->ReceiveKey(character);
  else
    PassToWorld();
}

void GUIBaseWindow::ReceiveMouseMove(const vec2 &pos, bool override) {
  if (override || m_movement_tracked) {
    for (int i = m_layouts.size() - 1; i >= 0; --i) {
      GUILayout *layout = m_layouts[cast<size_t>(i)];
      const vec2 max = layout->m_size, min = pos - layout->m_pos;
      if (layout->visible() && min.y > 0 && min.x > 0 && min.x < max.x &&
          min.y < max.y) {
        if (i != m_focused_child) {
          if (m_focused_child != -1)
            m_layouts[cast<size_t>(m_focused_child)]->DefocusChild();
          m_focused_child = i;
        }
        layout->ReceiveMouseMove(pos);
        return;
      }
    }

    if (m_focused_child != -1) {
      m_layouts[cast<size_t>(m_focused_child)]->DefocusChild();
      m_focused_child = -1;
    }
  }
  PassToWorld();
}

void GUIBaseWindow::ReceiveMouseWheel(double x, double y) {
  // focus policy
  //  MoveMouse();
  if (m_focused_child != -1 &&
      m_layouts[cast<size_t>(m_focused_child)]->visible())
    m_layouts[cast<size_t>(m_focused_child)]->ReceiveMouseWheel(x, y);
  else
    PassToWorld();
}

void GUIBaseWindow::DefocusChild() {
  if (m_focused_child != -1) {
    m_layouts[cast<size_t>(m_focused_child)]->DefocusChild();
    m_focused_child = -1;
  }
}

GUIBaseWindow::GUIBaseWindow(const ApplicationManager &manager)
    : m_app_manager(manager), m_resource_manager(new GUIResourceManager) {}
