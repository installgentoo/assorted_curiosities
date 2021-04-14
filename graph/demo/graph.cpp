#include "graph.h"

#include "base_classes/compute_quad.h"

using namespace gl;

SHADER(demo___graph_vertex_shader)
"layout(location = 0)in vec2 Position;",
"",
"void main()",
"{",
"  gl_Position = vec4(Position.xy, 0., 1.);",
"}");

SHADER(demo___graph_pixel_shader)
"layout(location = 0)out vec4 glFragColor;",
"",
"void main()",
"{",
"  glFragColor = vec4(1., 0.84, 0.01, 1.);",
"}");

SHADER(demo___candle_vertex_shader)
"layout(location = 0)in vec2 Position;",
"layout(location = 1)in float Color;",
"out vec4 glColor;",
"",
"void main()",
"{",
"  gl_Position = vec4(Position.xy, 0., 1.);",
"  glColor = vec4(Color, 1. - Color, 0., 1.);",
"}");

SHADER(demo___candle_pixel_shader)
"layout(location = 0)out vec4 glFragColor;",
"in vec4 glColor;",
"",
"void main()",
"{",
"  glFragColor = glColor;",
"}");

using namespace gl;
using namespace demo;

void GraphRenderer::setScale(int scale) {
  if (m_num_desired_displayed_values == scale)
    return;

  m_num_desired_displayed_values = scale;
  this->notify();
}

void GraphRenderer::setOffset(int offset) {
  if (m_desired_offset == offset)
    return;

  m_desired_offset = offset;
  this->notify();
}

int GraphRenderer::scale() const { return m_num_displayed_values; }

int GraphRenderer::offset() const { return m_offset; }

void GraphRenderer::Enable() {
  if (m_active == true)
    return;

  m_active = true;
  this->notify();
}

void GraphRenderer::Disable() {
  if (m_active == false)
    return;

  m_active = false;
  this->notify();
}

bool GraphRenderer::enabled() const { return m_active || !(m_y_state < -2.); }

void GraphRenderer::ForceSettings() {
  m_num_desired_displayed_values = m_num_displayed_values;
  m_desired_offset = m_offset;
  if (m_active)
    m_y_state = 0.;
  else
    m_y_state = -2.01;

  this->notify();
}

void GraphRenderer::AdjustDisplay() {
  CASSERT(!m_storage.expired(),
          "No data attached to active graph or data was deleted");
  if (m_storage.expired())
    return;
  const int size = m_storage.lock().get()->size();

  const int target_size =
                std::min(std::max(m_num_desired_displayed_values, 1), size),
            size_dist = abs(target_size - m_num_displayed_values),
            size_sign = (target_size > m_num_displayed_values) * 2 - 1;

  const int target_offset =
                std::min(std::max(m_desired_offset, m_num_displayed_values / 2),
                         size - m_num_displayed_values / 2),
            offset_dist = abs(target_offset - m_offset),
            offset_sign = (target_offset > m_offset) * 2 - 1;

  m_needs_data_update = false;

  if (m_active && m_y_state < 0.) {
    m_y_state += 0.05;
  } else
    m_needs_data_update = true;

  if (!m_active && m_y_state > -2.01) {
    m_y_state -= 0.05;
  } else
    m_needs_data_update = true;

  if (size_dist != 0) {
    int step = size_sign;

    if (size_dist > 10)
      step = size_dist * size_sign / 10;

    m_num_displayed_values += 2 * step;

    m_needs_data_update = true;
  } else
    m_num_desired_displayed_values = m_num_displayed_values;

  if (offset_dist != 0) {
    int step = offset_sign;

    if (offset_dist > 10)
      step = offset_dist * offset_sign / 10;

    m_offset += step;

    m_needs_data_update = true;
  } else
    m_desired_offset = m_offset;
}

SimpleGraph::SimpleGraph(vec2 bounding_box_l_b, vec2 bounding_box_r_u)
    : m_shader("demo___graph_vertex_shader", "demo___graph_pixel_shader",
               "demo___graph_geom_shader"),
      m_bounding_box_l_b(move(move(bounding_box_l_b))),
      m_bounding_box_r_u(move(move(bounding_box_r_u))) {}

void SimpleGraph::RenderGraph(uint window_width, uint window_height) {
  if (this->enabled()) {
    GLState::Enable(GL_LINE_SMOOTH);
    GLState::Viewport(window_width * m_bounding_box_r_u.x,
                      window_height * m_bounding_box_r_u.y,
                      window_width * m_bounding_box_l_b.x,
                      window_height * m_bounding_box_l_b.y);

    m_shader.Use();
    m_vao.Bind();
    GLCHECK(glDrawElements(GL_LINES_ADJACENCY, m_idx_buff.size(),
                           GL_UNSIGNED_SHORT, 0));

    if (m_needs_data_update) {
      this->RegenerateGraph();
      m_needs_data_update = false;
      this->AdjustDisplay();
    }
  }
}

void SimpleGraph::RegenerateGraph() {
  const ssize_t first = std::max(m_offset - m_num_displayed_values / 2, 0),
                size = m_num_displayed_values;

  CASSERT(!m_storage.expired(),
          "No data attached to active graph or data was deleted");
  if (m_storage.expired())
    return;

  const auto storage = m_storage.lock().get();
  vector<double> data = storage->GetValueRange(first, size);
  const size_t n_candles = data.size();
  CASSERT(n_candles > 2, "Data too small");

  m_idx_buff.clear();
  m_xy_buff.clear();

  ushort counter = 0;
  const double step = 2. / (n_candles - 1);
  double x = -1.;

  m_xy_buff.emplace_back(-1);
  m_xy_buff.emplace_back(0);
  for (const auto i : data) {
    m_idx_buff.emplace_back(counter);
    m_idx_buff.emplace_back(counter + 1);
    m_idx_buff.emplace_back(counter + 2);
    m_idx_buff.emplace_back(counter + 3);
    m_xy_buff.emplace_back(x);
    m_xy_buff.emplace_back(i / 1.25 - m_y_state);

    ++counter;
    x += step;
  }
  m_xy_buff.emplace_back(1);
  m_xy_buff.emplace_back(0);
  m_idx_buff.resize(m_idx_buff.size() - 4);

  m_vbo_idx.AllocateBuffer(m_idx_buff, GL_STATIC_DRAW);
  m_vbo_xy.AllocateBuffer(m_xy_buff, GL_STATIC_DRAW);
  m_vbo_xy.AttribFormat(0, 2);
}

CandleGraph::CandleGraph(vec2 bounding_box_l_b, vec2 bounding_box_r_u)
    : m_shader("demo___candle_vertex_shader", "demo___candle_pixel_shader"),
      m_bounding_box_l_b(move(move(bounding_box_l_b))),
      m_bounding_box_r_u(move(move(bounding_box_r_u))) {}

void CandleGraph::RenderGraph(uint window_width, uint window_height) {
  if (this->enabled()) {
    GLState::Disable(GL_CULL_FACE);
    GLState::Viewport(window_width * m_bounding_box_r_u.x,
                      window_height * m_bounding_box_r_u.y,
                      window_width * m_bounding_box_l_b.x,
                      window_height * m_bounding_box_l_b.y);

    m_shader.Use();
    m_vao.Draw(m_idx_buff);

    if (m_needs_data_update) {
      this->RegenerateGraph();
      m_needs_data_update = false;
      this->AdjustDisplay();
    }
  }
}

void CandleGraph::RegenerateGraph() {
  const ssize_t pos = (m_offset - m_num_displayed_values / 2) - 1,
                first = std::max(pos, 0l),
                size = m_num_displayed_values + (pos < 0);

  CASSERT(!m_storage.expired(),
          "No data attached to active graph or data was deleted");
  if (m_storage.expired())
    return;

  const auto storage = m_storage.lock().get();
  vector<double> data = storage->GetValueRange(first, size);
  const size_t n_candles = data.size() - 1;
  CASSERT(n_candles > 2, "Data too small");

  m_idx_buff.clear();
  m_xy_buff.clear();
  m_r_buff.clear();

  ushort counter = 0;
  const double step = 2. / (n_candles - 1);
  double x = -1. + step / 2, yp = data[0] / 1.25;

  for (size_t i = 1; i < data.size() - 1; ++i) {
    const double x1 = x - step / 2, x2 = x + step / 2, y1 = yp,
                 y2 = data[i] / 1.25;

    MakeQuad<float>(x1, y1 - m_y_state, x2, y2 - m_y_state, m_xy_buff,
                    m_idx_buff, counter);
    int d = int(yp > y2);
    m_r_buff.emplace_back(255 * d);
    m_r_buff.emplace_back(255 * d);
    m_r_buff.emplace_back(255 * d);
    m_r_buff.emplace_back(255 * d);

    x += step;
    yp = y2;
  }

  m_vbo_idx.AllocateBuffer(m_idx_buff, GL_STATIC_DRAW);
  m_vbo_xy.AllocateBuffer(m_xy_buff, GL_STATIC_DRAW);
  m_vbo_xy.AttribFormat(0, 2);
  m_vbo_r.AllocateBuffer(m_r_buff, GL_STATIC_DRAW);
  m_vbo_r.AttribFormat(1, 1, GL_UNSIGNED_BYTE, GL_TRUE);
}

DataStorage::DataStorage() : m_data(deque<double>(20000)) {
  for (auto &i : m_data)
    i = m_dist(m_engine);
}

int DataStorage::size() const {
  const std::lock_guard<mutex> l(m_data_access);
  return cast<int>(m_data.size());
}

vector<double> DataStorage::GetValueRange(ssize_t first, ssize_t size) const {
  vector<double> values(cast<size_t>(size), 0);
  {
    const std::lock_guard<mutex> l(m_data_access);
    size =
        std::max(std::min(first + size, cast<ssize_t>(m_data.size()) - 1), 0l);
    first = std::max(std::min(first, cast<ssize_t>(m_data.size()) - 1), 0l);
    std::copy(m_data.begin() + first, m_data.begin() + size, values.begin());
  }
  return values;
}

void RandomDataStorage::InsertDataBatch(double dud) {
  {
    const std::lock_guard<mutex> l(m_data_access);
    m_data.erase(m_data.begin());
    m_data.emplace_back(m_dist(m_engine));
  }

  if (!m_viewer.expired())
    m_viewer.lock()->notify();
}
