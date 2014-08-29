#include "tersbox/base/rawdata.h"

#include "base/files/file_path.h"
#include "base/file_util.h"

bool RawHeightmap::Load() {
  data_.reset(new uint8[kSize * kSize + kSize * 2]);
  int readsize = ::base::ReadFile(path_, (char*)data_.get() + kSize, kSize * kSize);
  if (readsize != kSize * kSize) {
    return false;
  }

  memcpy(data_.get(), data_.get() + kSize, kSize);
  memcpy(data_.get() + kSize + kSize * kSize, data_.get() + kSize * kSize, kSize);
  return true;
}

azer::ImagePtr CreateGridTex() {
  azer::ImagePtr ptr(new azer::Image(128, 128));
  for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 128; x++) {
      //create bump-shaped function f that is zero on each edge
      float a0= (float)y / 127.0f;
      float a1= (float)(127 - x) / 127.0f;
      float a2= (float)(x - y) / 127.0f;

      float f = a0 * a1 * a2;
      f = (float)sqrt(f);

      //quantize the function value and make into color
      //store in rgb components of texture entry
      float k = (int)floor(1400.0f * f);

      if (k < 0) k = 0;
      if (k > 255) k = 255;

      uint32 val = 0;
      val = 30 + (k * k)/290;
      val |= ((int)k >> 8);
      val |= (k < 128 ? 0 : (int)(k - 128) * 2) >> 16;
      ptr->set_pixel(x, y, val);
    }
  }

  return ptr;
}
