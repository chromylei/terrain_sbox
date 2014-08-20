#pragma once

#include <memory>
#include "azer/base/image.h"
#include "base/basictypes.h"
#include "base/files/file_path.h"

class TerrainData {
 public:
  TerrainData(int width, float roughness, float heightrange)
      : kWidth(width)
      , fRoughness(roughness)
      , fRange(heightrange) {
    heightmap_.reset(new float[width * width]);
  }

  bool Save(const ::base::FilePath& path);
  void GenPlasma();
 private:
  float GenHeight(float avg, float heightrange);
  float GetHeight(int x, int y);
  void SetHeight(int x, int y, float h);
  void NormalizeHeightmap();
  const int kWidth;
  const float fRoughness;
  const float fRange;
  std::unique_ptr<float[]> heightmap_;
  DISALLOW_COPY_AND_ASSIGN(TerrainData);
};
