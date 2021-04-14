#include <GLFW/glfw3.h>

#include "graph.h"
#include "gui/guielements.h"
#include "gui/guilogic.h"
#include "gui/guimenu.h"

using namespace glgui;
using namespace demo;

//адаптер к графикам
struct GraphControls {
  void change_scale(int by) {
    if (!m_attached_graph)
      return;

    auto &graph = *m_attached_graph;
    graph.setScale(std::min(std::max(200, graph.scale() + by), 1000));
  }

  void change_pos(int by) {
    if (!m_attached_graph)
      return;

    auto &graph = *m_attached_graph;
    graph.setOffset(graph.offset() + by);
  }

  void set_renderer(GraphRenderer *graph) {
    if (m_attached_graph != nullptr)
      m_attached_graph->Disable();

    m_attached_graph = graph;
    m_attached_graph->Enable();
  }

private:
  GraphRenderer *m_attached_graph = nullptr;
};

int main() {
  //базовый класс, инициализирует opengl
  ApplicationManager &app_manager = ApplicationManager::Get();
  app_manager.ResizeWindow(1600, 800);

  //синглтон, символизирующий собой весь гуй
  GUIBaseWindow &base_window = app_manager.gui_base_window();
  app_manager.gui_resource_manager().setDefaultFont("arial.ttf", 24);
  app_manager.gui_resource_manager().setDefaultCustomBkgr(
      "smooth_background.png");

  //файл с геометрическим шейдером для второго графика
  ShaderManager::Get().LoadAllShadersFromFile("demo.glsl");

  //лейауты- это по задумке как окошки разных открываемых меню. в них кладутся
  //элементы
  GUILayout *main_menu = new GUILayout(vec2(-1.));

  //элементы гуя, собираются из компонентов, все методы манипуляции
  //компонентами внутри guielement, они самоочевидны
  GUIElement chart_area("chart_area", vec2(0.025, 0.05));
  chart_area->AddCustomBackground(vec2(1.5, 1.9), vec2(0.025), 3);
  chart_area->Component().setPos(vec2(0.022, 0.025));

  //фон
  shared_ptr<Texture> bkgr_tex = make_shared<Texture>("bkgr.png");
  GUIElement bkgr("bkgr");
  bkgr->AddPicture(bkgr_tex, vec2(2.), vec4(1));

  //елемент можно сохранить в темплейт
  GUIElement plus_button("plus_button", vec2(0.072, 0.1));
  plus_button->AddBackground(vec2(0.025, 0.05), vec4(0.3));
  plus_button->AddText("+", "arial.ttf", 24, vec4(1, 0.84, 0.01, 1));
  plus_button->Component().setPos(vec2(0., 0.0025));
  plus_button->Component().setAlign(GUI::MIDDLE);
  plus_button.SaveAsTemplate("button");

  GUIElement min_button("min_button", "button", vec2(0.11, 0.1));
  min_button->Component().Write("-");
  min_button->Component().setPos(vec2(0., 0.008));

  GUIElement left_button("left_button", "button", vec2(0.2, 0.1));
  left_button->Component().Write("<");

  GUIElement right_button("right_button", "button", vec2(0.238, 0.1));
  right_button->Component().Write(">");

  GUIElement chart_control_1("chart_1", vec2(1.55, 0.15));
  chart_control_1->AddCustomBackground(vec2(0.4, 0.8), vec2(0.025), 5);
  chart_control_1->AddText(
      "Chart #2\nRandom data\n-click to  switch to\n\n\n\nESC to quit",
      "arial.ttf", 24, vec4(1, 0.84, 0.01, 1));
  chart_control_1->Component().setPos(vec2(0.025, -0.017));
  chart_control_1->Component().setAlign(GUI::TOP);
  chart_control_1.SaveAsTemplate("chart");

  GUIElement chart_control_2("chart_2", "chart", vec2(1.55, 1.05));
  chart_control_2->Component().Write(
      "Chart #1\nPreset data\n-click to switch to");

  //создаём инстансы графиков и их содержимого. просто поинтеры, т.к. у нас
  //нкакой сложной логики на этом уровне нет
  shared_ptr<Storage> data_storage_random(new RandomDataStorage);
  shared_ptr<Storage> data_storage_static(new DataStorage);
  shared_ptr<Viewer> graph_viewer1(
      new CandleGraph(vec2(0.067, 0.1), vec2(1.46, 1.856)));
  shared_ptr<Viewer> graph_viewer2(
      new SimpleGraph(vec2(0.067, 0.1), vec2(1.46, 1.856)));

  CandleGraph *graph_p1 = static_cast<CandleGraph *>(graph_viewer1.get());
  SimpleGraph *graph_p2 = static_cast<SimpleGraph *>(graph_viewer2.get());
  RandomDataStorage *r_storage_p =
      static_cast<RandomDataStorage *>(data_storage_random.get());
  DataStorage *s_storage_p =
      static_cast<DataStorage *>(data_storage_static.get());

  graph_p1->setStorage(data_storage_random);
  graph_p2->setStorage(data_storage_static);
  r_storage_p->setViewer(graph_viewer1);
  s_storage_p->setViewer(graph_viewer2);

  //собираем гуй из элементов
  GraphControls controls;
  controls.set_renderer(static_cast<GraphRenderer *>(graph_viewer1.get()));
  graph_p1->ForceSettings();

  base_window.AddMenu(main_menu);
  base_window.AddGlobalKeyMapping(GLFW_KEY_ESCAPE,
                                  [&app_manager]() { app_manager.Quit(); });
  main_menu->AddElement(bkgr);
  main_menu->AddElement(chart_area);
  //логика вставляется в кнопки
  main_menu->AddElement(
      plus_button, new GUIButtonLogic([&]() { controls.change_scale(-50); }));
  main_menu->AddElement(
      min_button, new GUIButtonLogic([&]() { controls.change_scale(50); }));
  main_menu->AddElement(
      left_button, new GUIButtonLogic([&]() { controls.change_pos(-1000); }));
  main_menu->AddElement(
      right_button, new GUIButtonLogic([&]() { controls.change_pos(1000); }));
  main_menu->AddElement(chart_control_1, new GUIButtonLogic([&]() {
                          controls.set_renderer(static_cast<GraphRenderer *>(
                              graph_viewer1.get()));
                        }));
  main_menu->AddElement(chart_control_2, new GUIButtonLogic([&]() {
                          controls.set_renderer(static_cast<GraphRenderer *>(
                              graph_viewer2.get()));
                        }));

  ShaderManager::Get().ClearCache();

  //представим что это внешний источник данных
  atomic<bool> wait(true);
  std::thread t1([r_storage_p, &wait]() {
    while (wait) {
      r_storage_p->InsertDataBatch(0);
      std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
  });

  //основной цикл приложения
  while (!app_manager.ShouldTerminate()) {
    app_manager.DrawToScreen(true);
    app_manager.DrawMenu();

    //не освобождаю память от графиков, ведь их всего 2 и это демка. эти
    //методы и так не рисуют выключенный график
    graph_p1->RenderGraph(app_manager.window_width() / 2,
                          app_manager.window_height() / 2);
    graph_p2->RenderGraph(app_manager.window_width() / 2,
                          app_manager.window_height() / 2);

    app_manager.SwapBuffers();
  }

  //конец!
  wait = false;
  t1.join();
  exit(EXIT_SUCCESS);
}
