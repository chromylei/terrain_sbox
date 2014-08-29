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

