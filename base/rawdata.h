#pragma once

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/logging.h"

class RawHeightmap {
 public:
  RawHeightmap(const ::base::FilePath& path, int size)
      : path_(path)
      , kSize(size) {
  }

  bool Load();

  uint8 height(int x, int y) const;
  int width() const { return kSize;}
 private:
  const int kSize;
  const ::base::FilePath path_;
  std::unique_ptr<uint8> data_;
  DISALLOW_COPY_AND_ASSIGN(RawHeightmap);
};

inline uint8 RawHeightmap::height(int x, int y) const {
  int index = (y + 1) * kSize + x;
  DCHECK_LT(y, kSize + 2);
  DCHECK_LT(x, kSize);
  DCHECK_LT(index, kSize * kSize + kSize * 2);
  return data_.get()[index];
}
