#include "guilogic.h"

#include <GLFW/glfw3.h>

#include "base_classes/oglmanagement.h"
#include "guielements.h"
#include "guimenu.h"

using namespace glgui;

inline GUIElementObject &GUIElementLogic::parent() {
  CASSERT(m_parent, "No parent yet");
  return *m_parent;
}

GUIComponent &GUIElementLogic::component(uint num) {
  return parent().Component(cast<int>(num));
}

void GUIEditableTextLogic::ReceiveMouseButton(int button, int mods,
                                              int action) {
  if (GLFW_PRESS == action && GLFW_MOUSE_BUTTON_LEFT == button) {
    reinterpret_cast<GUIText *>(m_text->obj())->Write(m_str.c_str());
    m_text->CorrectPos(true);
  }
}

GUIText &GUIElementLogic::component_as_text(uint num) {
  GUIText *text = dynamic_cast<GUIText *>(component(num).obj());
  CASSERT(text, "Wrong object number");
  return *text;
}

void GUIElementLogic::SetParent(GUIElementObject *parent) { m_parent = parent; }

void GUIResumeButtonLogic::ReceiveMouseButton(int button, int mods,
                                              int action) {
  if (GLFW_PRESS == action && GLFW_MOUSE_BUTTON_LEFT == button)
    m_layout->setVisible(false);
}

GUITextBoxLogic::GUITextBoxLogic(uint text, uint selector, const char *filter)
    : m_text(text), m_selector(selector), m_filter(filter) {}

void GUITextBoxLogic::ReceiveKey(uint character) {
  if (!m_filter.size() || m_filter.find(char(character)) != string::npos) {
    auto &app_manager = ApplicationManager::Get();
    GUIText &text = component_as_text(m_text);
    double length = 0.;
    for (auto i : text.m_text)
      length += double(text.font().charinfo(i).advance);
    length *= 2. / app_manager.window_width();

    if (m_active && uint(int(character) - 32) < 96 &&
        m_max_length > text.m_text.size()) {
      const double advance =
          double(text.font().charinfo(character).advance * 2) /
          app_manager.window_width();
      if (length + advance < parent().size().x) {
        text.m_text.insert(text.m_text.begin() + m_selected_char,
                           char(character));
        length += advance;
        ++m_selected_char;
        text.Write();
        component(m_selector).movePos(vec2(advance, 0.));
      }
    }
  }
}

void GUITextBoxLogic::ReceiveKey(int key, int action, int mods) {
  auto &app_manager = ApplicationManager::Get();
  GUIText &text = component_as_text(m_text);
  double length = 0.;
  for (auto i : text.m_text)
    length += double(text.font().charinfo(i).advance);
  length *= 2. / app_manager.window_width();

  if (GLFW_PRESS == action || GLFW_REPEAT == action) {
    switch (key) {
    case GLFW_KEY_LEFT:
      if (m_selected_char > 0) {
        --m_selected_char;
        const double advance =
            double(text.font().charinfo(text.m_text[m_selected_char]).advance *
                   2) /
            app_manager.window_width();
        component(m_selector).movePos(vec2(-advance, 0.));
      }
      break;
    case GLFW_KEY_RIGHT:
      if (m_selected_char < text.m_text.size()) {
        const double advance =
            double(text.font().charinfo(text.m_text[m_selected_char]).advance *
                   2) /
            app_manager.window_width();
        ++m_selected_char;
        component(m_selector).movePos(vec2(advance, 0.));
      }
      break;
    case GLFW_KEY_BACKSPACE: {
      if (m_selected_char > 0) {
        --m_selected_char;
        const double advance =
            double(text.font().charinfo(text.m_text[m_selected_char]).advance *
                   2) /
            app_manager.window_width();
        component(m_selector).movePos(vec2(-advance, 0.));
        length -= advance;
        text.m_text.erase(text.m_text.begin() + m_selected_char);
        text.Write();
      } else
        length = 0;
    } break;
    case GLFW_KEY_DELETE:
      if (m_selected_char < text.m_text.size()) {
        const double advance =
            double(text.font().charinfo(text.m_text[m_selected_char]).advance *
                   2) /
            app_manager.window_width();
        length -= advance;
        text.m_text.erase(text.m_text.begin() + m_selected_char);
        text.Write();
      }
      break;
    }
  }
}

void GUITextBoxLogic::ReceiveMouseButton(int button, int mods, int action) {
  if (GLFW_PRESS == action && GLFW_MOUSE_BUTTON_LEFT == button) {
    GUIComponent &selector = component(m_selector);
    selector.setVisible(true);
    const GUIText &text = component_as_text(m_text);
    auto &app_manager = ApplicationManager::Get();
    double pos = ApplicationManager::Get().GetMousePos().x -
                 component(m_text).m_pos.x,
           seek = 0.;
    pos = std::max(pos, 0.);
    for (uint i = 0, size = text.m_text.size(); i < size; ++i) {
      seek += double(text.font().charinfo(text.m_text[i]).advance * 2) /
              app_manager.window_width();
      if (seek > pos) {
        m_selected_char = i;
        selector.setPos(vec2(
            seek - double(text.font().charinfo(text.m_text[i]).advance * 2) /
                       app_manager.window_width(),
            0.));
        m_active = true;
        return;
      }
    }
    m_selected_char = cast<uint>(text.m_text.size());
    selector.setPos(vec2(seek, 0.));
    m_active = true;
  }
}

void GUITextBoxLogic::DefocusChild() {
  component(m_selector).setVisible(false);
  m_active = false;
}

void GUIButtonLogic::ReceiveMouseButton(int button, int mods, int action) {
  if (GLFW_PRESS == action && GLFW_MOUSE_BUTTON_LEFT == button)
    m_func();
}
