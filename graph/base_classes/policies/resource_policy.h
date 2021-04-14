#pragma once
#include "code_policy.h"

namespace gl {

template <class m_policy> struct ResourceControl {
  static vector<char> LoadResource(const char *resource_name) {
    return m_policy::LoadResource(resource_name);
  }
};

struct OnDemandResourcePolicy {
  static vector<char> LoadResource(const char *resource_name);
};
struct PreloadResourcePolicy {
  // implement
};

template <class m_policy> struct ImageDecoderControl {
  static vector<uchar> DecodeImage(vector<char> &data, uint channels,
                                   uint &width, uint &height) {
    return m_policy::DecodeImage(data, channels, width, height);
  }
};

struct LodepngDecodePolicy {
  static vector<uchar> DecodeImage(vector<char> &data, uint channels,
                                   uint &width, uint &height);
};

typedef ResourceControl<OnDemandResourcePolicy> ResourceLoader;

typedef ImageDecoderControl<LodepngDecodePolicy> ImageDecoder;

} // namespace gl
