#pragma once

#include <functional>

#include "base_classes/policies/code_policy.h"

namespace glgui {

struct GUILayout;
struct GUIText;
struct GUIComponent;
struct GUIElementObject;
struct GUIResourceManager;

struct GUIElementLogic {
  GUIElementObject &parent();
  GUIComponent &component(uint num);
  GUIText &component_as_text(uint num);
  void SetParent(GUIElementObject *parent);
  virtual GUIElementLogic *Clone() = 0;
  virtual void ReceiveMouseButton(int button, int mods, int action) {}
  virtual void ReceiveMouseMove(const vec2 &pos) {}
  virtual void ReceiveKey(int key, int action, int mods) {}
  virtual void ReceiveKey(uint character) {}
  virtual void ReceiveMouseWheel(double x, double y) {}
  virtual void DefocusChild() {}
  virtual ~GUIElementLogic() = default;

protected:
  GUIElementObject *m_parent;
  GUIElementLogic() = default;
  GUIElementLogic(const GUIElementLogic &) = delete;
  GUIElementLogic &operator=(const GUIElementLogic &) = delete;
};

struct GUIButtonLogic : GUIElementLogic {
  GUIButtonLogic(function<void()> func) : m_func(func) {}
  GUIElementLogic *Clone() { return new GUIButtonLogic(m_func); }

private:
  function<void()> m_func;

  void ReceiveMouseButton(int button, int mods, int action);
};

struct GUIEditableTextLogic : GUIElementLogic {
  GUIEditableTextLogic(GUIComponent *text, const char *str)
      : m_str(str), m_text(text) {}
  GUIElementLogic *Clone() {
    return new GUIEditableTextLogic(m_text, m_str.c_str());
  }

private:
  string m_str;
  GUIComponent *m_text;

  void ReceiveMouseButton(int button, int mods, int action);
};

struct GUIResumeButtonLogic : GUIElementLogic {
  GUIResumeButtonLogic(GUILayout *layout) : m_layout(layout) {}
  GUIElementLogic *Clone() { return new GUIResumeButtonLogic(m_layout); }

private:
  GUILayout *m_layout;

  void ReceiveMouseButton(int button, int mods, int action);
};

struct GUITextBoxLogic : GUIElementLogic {
  GUITextBoxLogic(uint text, uint selector, const char *filter = "");
  GUIElementLogic *Clone() {
    return new GUITextBoxLogic(m_text, m_selector, m_filter.c_str());
  }

private:
  uint m_text, m_selector, m_max_length = 70, m_selected_char = 0;
  bool m_active = false;
  string m_filter;

  void ReceiveKey(int key, int action, int mods);
  void ReceiveKey(uint character);
  void ReceiveMouseButton(int button, int mods, int action);
  void DefocusChild();
};

} // namespace glgui
