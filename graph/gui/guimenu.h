#pragma once

#include <functional>

#include "base_classes/oglmanagement.h"
#include "guielements.h"

namespace glgui {

struct GUIBaseWindow {
  friend struct ::gl::ApplicationManager;
  friend struct GUIElementLogic;

  void AddMenu(GUILayout *layout);
  void AddGlobalKeyMapping(int key, function<void()> func);
  void Draw() const;

private:
  vector<GUILayout *> m_layouts;
  unordered_map<int, function<void()>> m_global_key_mapping;
  int m_focused_child = -1;
  const ApplicationManager &m_app_manager;
  bool m_pause = false, m_movement_tracked = false;
  unique_ptr<GUIResourceManager> m_resource_manager;

  void MoveMouse();
  void ReceiveMouseButton(int button, int mods, int action);
  void ReceiveKey(int key, int action, int mods);
  void ReceiveKey(uint character);
  void ReceiveMouseMove(const vec2 &pos, bool override = false);
  void ReceiveMouseWheel(double x, double y);
  void DefocusChild();

  void PassToWorld() {}

  GUIBaseWindow(const ApplicationManager &manager);
  GUIBaseWindow(const GUIBaseWindow &) = delete;
  GUIBaseWindow &operator=(const GUIBaseWindow &) = delete;
};

} // namespace glgui
