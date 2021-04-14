#pragma once
#include <fstream>
#include <string>

struct Point {
  int32_t m_x = 0;
  int32_t m_y = 0;
  std::string m_name;

  bool operator==(const Point &other) const;
};

std::istream &operator>>(std::istream &stream, Point &point);
std::ostream &operator<<(std::ostream &stream, const Point &point);
