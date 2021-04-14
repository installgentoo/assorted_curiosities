#pragma once
#include <iostream>

#include "code_policy.h"

namespace gl {

template <class m_policy, int m_init_level>
struct LoggingControl : m_policy, CUNIQUE {
  enum Level { ERROR, WARNING, DEBUG };

  static LoggingControl &Get() {
    static LoggingControl s_control;
    return s_control;
  }

  constexpr static bool IsLevel(Level level) { return m_init_level >= level; }

  template <class arg_type> LoggingControl &operator<<(arg_type arg) {
    try {
      m_policy::GetStream() << arg;
    } catch (...) {
    }
    return *this;
  }

  void Flush() {
    try {
      m_policy::GetStream().flush();
    } catch (...) {
    }
  }
};

struct FilePolicy {
protected:
  std::ostream &GetStream();

private:
  std::ofstream *m_log_file = nullptr;
};

struct CoutPolicy {
protected:
  std::ostream &GetStream() const;
};

#ifdef NDEBUG
typedef LoggingControl<CoutPolicy, 2> Log;
#else
typedef LoggingControl<CoutPolicy, 2> Log;
#endif

#ifdef NDEBUG
#define CTERMINATE exit(-1);
#else
#define CTERMINATE assert(0);
#endif

#define CINFO(text)                                                            \
  {                                                                            \
    Log::Get() << text << "\n";                                                \
    Log::Get().Flush();                                                        \
  }
#define CERROR(text)                                                           \
  {                                                                            \
    Log::Get() << "E " << text << " |in " << __FILE__ << ":" << __LINE__       \
               << "\n";                                                        \
    Log::Get().Flush();                                                        \
    CTERMINATE                                                                 \
  }
#define CWARNING(text)                                                         \
  {                                                                            \
    if (Log::IsLevel(Log::WARNING)) {                                          \
      Log::Get() << "W " << text << " |in " << __FILE__ << ":" << __LINE__     \
                 << "\n";                                                      \
      Log::Get().Flush();                                                      \
    }                                                                          \
  }
#define CASSERT(var, text)                                                     \
  {                                                                            \
    if (Log::IsLevel(Log::DEBUG) && !(var)) {                                  \
      Log::Get() << "A " << text << " |in expr (" << #var << "), in "          \
                 << __FILE__ << ":" << __LINE__ << "\n";                       \
      Log::Get().Flush();                                                      \
      CTERMINATE                                                               \
    }                                                                          \
  }
#define GLCHECK(f)                                                             \
  f;                                                                           \
  {                                                                            \
    if (Log::IsLevel(Log::DEBUG)) {                                            \
      const GLenum errCode = glGetError();                                     \
      if (errCode != GL_NO_ERROR) {                                            \
        Log::Get() << "W OpenGL error " << glCodeToError(errCode)              \
                   << " |in func (" << #f << "), in " << __FILE__ << ":"       \
                   << __LINE__ << "\n";                                        \
        Log::Get().Flush();                                                    \
        assert(0);                                                             \
      }                                                                        \
    }                                                                          \
  }

template <class To, class T> To cast(T n) {
  CASSERT((static_cast<long double>(n) >=
           static_cast<long double>(std::numeric_limits<To>::min())) &&
              (static_cast<long double>(n) <=
               static_cast<long double>(std::numeric_limits<To>::max())),
          "Cast overflow")
  return static_cast<To>(n);
}

} // namespace gl
