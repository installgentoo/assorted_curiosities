#pragma once
#include <limits>

#include "policies/code_policy.h"

namespace gl {

template <class T> void MakeQuadIdx(T &s, vector<T> &idx) {
  idx.reserve(idx.size() + 6);
  idx.emplace_back(s);
  idx.emplace_back(s + 1);
  idx.emplace_back(s + 3);
  idx.emplace_back(s + 3);
  idx.emplace_back(s + 1);
  idx.emplace_back(s + 2);
  CASSERT(s + 4 < std::numeric_limits<T>::max(), "Quad index overflow " << s);
  s += 4;
}

template <class T> void MakeQuad(T x1, T y1, T x2, T y2, vector<T> &buffer) {
  buffer.reserve(buffer.size() + 8);
  buffer.emplace_back(x1);
  buffer.emplace_back(y1);

  buffer.emplace_back(x2);
  buffer.emplace_back(y1);

  buffer.emplace_back(x2);
  buffer.emplace_back(y2);

  buffer.emplace_back(x1);
  buffer.emplace_back(y2);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T x2, T y2, vector<T> &buffer, vector<I_T> &idx,
              I_T &vert) {
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, x2, y2, buffer);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T x2, T y2, vector<T> &buffer, vector<I_T> &idx) {
  I_T vert = 0;
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, x2, y2, buffer);
}

template <class T>
void MakeQuad(T x1, T y1, T z1, T x2, T y2, T z2, vector<T> &buffer) {
  buffer.reserve(buffer.size() + 12);
  buffer.emplace_back(x1);
  buffer.emplace_back(y1);
  buffer.emplace_back(z1);

  buffer.emplace_back(x2);
  buffer.emplace_back(y1);
  buffer.emplace_back(z2);

  buffer.emplace_back(x2);
  buffer.emplace_back(y2);
  buffer.emplace_back(z2);

  buffer.emplace_back(x1);
  buffer.emplace_back(y2);
  buffer.emplace_back(z1);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T z1, T x2, T y2, T z2, vector<T> &buffer,
              vector<I_T> &idx, I_T &vert) {
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, z1, x2, y2, z2, buffer);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T z1, T x2, T y2, T z2, vector<T> &buffer,
              vector<I_T> &idx) {
  I_T vert = 0;
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, z1, x2, y2, z2, buffer);
}

template <class T>
void MakeQuad(T x1, T y1, T u1, T v1, T x2, T y2, T u2, T v2,
              vector<T> &buffer) {
  buffer.reserve(buffer.size() + 16);
  buffer.emplace_back(x1);
  buffer.emplace_back(y1);
  buffer.emplace_back(u1);
  buffer.emplace_back(v1);

  buffer.emplace_back(x2);
  buffer.emplace_back(y1);
  buffer.emplace_back(u2);
  buffer.emplace_back(v1);

  buffer.emplace_back(x2);
  buffer.emplace_back(y2);
  buffer.emplace_back(u2);
  buffer.emplace_back(v2);

  buffer.emplace_back(x1);
  buffer.emplace_back(y2);
  buffer.emplace_back(u1);
  buffer.emplace_back(v2);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T u1, T v1, T x2, T y2, T u2, T v2, vector<T> &buffer,
              vector<I_T> &idx, I_T &vert) {
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, u1, v1, x2, y2, u2, v2, buffer);
}

template <class T, class I_T>
void MakeQuad(T x1, T y1, T u1, T v1, T x2, T y2, T u2, T v2, vector<T> &buffer,
              vector<I_T> &idx) {
  I_T vert = 0;
  MakeQuadIdx(vert, idx);
  MakeQuad(x1, y1, u1, v1, x2, y2, u2, v2, buffer);
}

} // namespace gl
