#pragma once

#include <memory>
#include "azer/base/image.h"
#include "base/basictypes.h"
#include "base/files/file_path.h"

class TerrainData {
 public:
  TerrainData(int width)
      : kWidth(width) {
    heightmap_.reset(new float[width * width]);
  }

  bool Save(const ::base::FilePath& path);
  void Gen(float heightrange, float fRoughness);
 private:
  void Gen(int left, int top, int right, int bottom,
           float heightrange, float fRoughness);
  const int kWidth;
  std::unique_ptr<float> heightmap_;
  DISALLOW_COPY_AND_ASSIGN(TerrainData);
};
