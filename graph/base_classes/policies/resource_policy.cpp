#include "resource_policy.h"

#include <lodepng.h>

#include <fstream>

#include "logging_policy.h"

using namespace gl;
using namespace std;

std::vector<char>
OnDemandResourcePolicy::LoadResource(const char *resource_name) {
  ifstream file(resource_name, ifstream::binary | ifstream::in);
  vector<char> data;
  if (file.is_open()) {
    file.exceptions(ifstream::failbit | ifstream::badbit);
    try {
      streamsize size = 0;
      if (file.seekg(0, ios::end).good())
        size = file.tellg();
      if (file.seekg(0, ios::beg).good())
        size -= file.tellg();
      data.resize(cast<size_t>(size));
      if (size > 0)
        file.read(data.data(), size);
      file.close();
    } catch (ifstream::failure) {
      CERROR("Error accessing file " << resource_name);
    }
  } else {
    CINFO("No file " << resource_name);
  }

  return data;
}

vector<uchar> LodepngDecodePolicy::DecodeImage(vector<char> &data,
                                               uint channels, uint &width,
                                               uint &height) {
  LodePNGColorType colortype = LCT_RGB;
  switch (channels) {
  case 1:
    colortype = LCT_GREY;
    break;
  case 3:
    colortype = LCT_RGB;
    break;
  case 4:
    colortype = LCT_RGBA;
    break;
    CASSERT(0, "Invalid channels for image loading");
  }

  vector<uchar> image;
  const uint error = lodepng::decode(image, width, height,
                                     reinterpret_cast<uchar *>(data.data()),
                                     data.size(), colortype, 8);
  if (error)
    CERROR("Lodepng error: " << lodepng_error_text(error));

  const uint half_m_height = height / 2, stride = width * channels;
  const unique_ptr<uchar[]> temp(new uchar[stride]);

  for (uint i = half_m_height; i; --i) {
    void *top = image.data() + stride * (i - 1),
         *bottom = image.data() + stride * (height - i);
    memcpy(temp.get(), bottom, stride);
    memcpy(bottom, top, stride);
    memcpy(top, temp.get(), stride);
  }

  return image;
}
