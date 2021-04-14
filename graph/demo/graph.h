#pragma once
#include <random>

#include "base_classes/shader.h"
#include "base_classes/texture.h"

namespace gl {

namespace demo {
//интерфейсы к рендерилке и данным
struct Viewer {
  virtual ~Viewer() = default;
  virtual void notify() = 0;
};

struct Storage {
  virtual ~Storage() = default;
  virtual vector<double> GetValueRange(ssize_t first, ssize_t size) const = 0;
  virtual int size() const = 0;
};

//имплементация рендерилки графика
struct GraphRenderer : Viewer {
  GraphRenderer() = default;
  virtual ~GraphRenderer() = default;

  void setStorage(shared_ptr<Storage> &storage) { m_storage = storage; }

  void setScale(int scale);
  void setOffset(int offset);
  int scale() const;
  int offset() const;
  void Enable();
  void Disable();
  bool enabled() const;
  void ForceSettings();
  //функция для плавного перехода к нужным значениям
  void AdjustDisplay();

  //функции для сбора вершин в модель и отрисовки её
  virtual void RenderGraph(uint window_width, uint window_height) = 0;
  virtual void RegenerateGraph() = 0;

  virtual void notify() { m_needs_data_update = true; }

protected:
  weak_ptr<Storage> m_storage;
  atomic<bool> m_needs_data_update;
  bool m_active = false;
  int m_num_displayed_values = 200, m_num_desired_displayed_values = 200,
      m_offset = 1000, m_desired_offset = 1000;
  float m_y_state = -2.01; //сдвиг графика, чтобы плавно убрать его с экрана

  CDISABLE_COPY(GraphRenderer)
};

//не понял что вы подразумевали под графиком- график или свечки, так что сделал
//оба
struct SimpleGraph : GraphRenderer {
  SimpleGraph(vec2 bounding_box_l_b, vec2 bounding_box_r_u);
  virtual ~SimpleGraph() = default;

  void RenderGraph(uint window_width, uint window_height);
  void RegenerateGraph();

private:
  //внутренности ogl
  Shader m_shader;
  vec2 m_bounding_box_l_b, m_bounding_box_r_u;
  GLvao m_vao;
  GLbuffer<GL_ELEMENT_ARRAY_BUFFER> m_vbo_idx;
  GLbuffer<GL_ARRAY_BUFFER> m_vbo_xy;
  vector<ushort> m_idx_buff;
  vector<float> m_xy_buff;
};

struct CandleGraph : GraphRenderer {
  CandleGraph(vec2 bounding_box_l_b, vec2 bounding_box_r_u);
  virtual ~CandleGraph() = default;

  void RenderGraph(uint window_width, uint window_height);
  void RegenerateGraph();

private:
  //внутренности ogl
  Shader m_shader;
  vec2 m_bounding_box_l_b, m_bounding_box_r_u;
  GLvao m_vao;
  GLbuffer<GL_ELEMENT_ARRAY_BUFFER> m_vbo_idx;
  GLbuffer<GL_ARRAY_BUFFER> m_vbo_xy, m_vbo_r;
  vector<ushort> m_idx_buff;
  vector<float> m_xy_buff;
  vector<uchar> m_r_buff;
};

//симуляция статической и получающией данные извне имплементации
struct DataStorage : Storage {
  DataStorage();
  virtual ~DataStorage() = default;

  void setViewer(shared_ptr<Viewer> &viewer) { m_viewer = viewer; }

  int size() const;
  vector<double> GetValueRange(ssize_t first, ssize_t size) const;

protected:
  weak_ptr<Viewer> m_viewer;
  mutable mutex m_data_access;
  deque<double> m_data;

  std::default_random_engine m_engine;
  std::uniform_real_distribution<double> m_dist =
      std::uniform_real_distribution<double>(-1, 1);

  CDISABLE_COPY(DataStorage)
};

struct RandomDataStorage : DataStorage {
  void InsertDataBatch(double dud);
};

} // namespace demo

} // namespace gl
