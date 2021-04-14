#include "logging_policy.h"

#include <fstream>
#include <mutex>

using namespace gl;
using namespace std;

ostream &FilePolicy::GetStream() {
  if (!m_log_file) {
    static mutex m;
    const lock_guard<mutex> l(m);
    if (!m_log_file)
      m_log_file = new ofstream("log.txt", ofstream::out | ofstream::app);
  }
  return *m_log_file;
}

ostream &CoutPolicy::GetStream() const { return cout; }
