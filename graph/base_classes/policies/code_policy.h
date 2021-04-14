#pragma once
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#define GLM_FORCE_RADIANS
#include <string.h>

#include <algorithm>
#include <deque>
#include <future>
#include <glm/matrix.hpp>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CDISABLE_COPY(CLASS)                                                   \
  CLASS(const CLASS &) = delete;                                               \
  CLASS &operator=(const CLASS &) = delete;

namespace gl {
class CUNIQUE {
  CUNIQUE(const CUNIQUE &) = delete;
  CUNIQUE &operator=(const CUNIQUE &) = delete;

protected:
  CUNIQUE() = default;
  ~CUNIQUE() = default;
};

typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int64_t lint;
typedef uint64_t ulint;

constexpr double to_radians(double degr) { return degr * (M_PI / 180.); }

using glm::ivec2;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::mat4x3;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using std::array;
using std::async;
using std::atomic;
using std::deque;
using std::function;
using std::future;
using std::map;
using std::move;
using std::mutex;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::thread;
using std::unique_lock;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::weak_ptr;

using std::make_shared;
} // namespace gl

namespace glgui {
using namespace gl;
}
