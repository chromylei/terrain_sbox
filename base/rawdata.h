#pragma once

#include "base/basictypes.h"
#include "base/files/file_path.h"

class RawHeightmap {
 public:
  RawHeightmap(const ::base::FilePath& path, int size)
      : path_(path)
      , kSize(size) {
  }

  bool Load();

  uint8 height(int x, int y);
 private:
  const int kSize;
  const ::base::FilePath path_;
  std::unique_ptr<uint8> data_;
  DISALLOW_COPY_AND_ASSIGN(RawHeightmap);
};

inline uint8 RawHeightmap::height(int x, int y) {
  return data_.get()[(y + 1) * kSize + x];
}
