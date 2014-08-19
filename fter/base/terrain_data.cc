#include "tersbox/fter/base/terrain_data.h"


#include <cmath>
#include "base/random_util.h"


void TerrainData::Gen(float heightrange, float fRoughness) {
  Gen(0, 0, kWidth - 1, kWidth - 1, heightrange, fRoughness);
}

void TerrainData::Gen(int left, int top, int right, int bottom,
                      float heightrange, float fRoughness) {
  if (left - top == 1) {
    return;
  }

  int midpos_x = (left + right) / 2;
  int midpox_y = (top + bottom) / 2;
  heightmap_[midpos_y * kWidth + midpos_x] =
      ::base::RandDouble() * heightrange - heightrange / 2.0f;
  Gen(left, top, midpos_x, midpos_y, heightrange, fRoughness);
  Gen(midpos_x, top, right, midpos_y, heightrange, fRoughness);
  Gen(left, midpoy_y, midpos_x, bottom, heightrange, fRoughness);
  Gen(midpos_x, midpoy_y, right, bottom, heightrange, fRoughness);
}

void TerrainData::Save(const ::base::FilePath& path) {
  azer::Image image(kWidth, kWidth, kRGBA32);
  uint32* ptr = (float*)image.data();
  for (int i = 0; i < kWidth; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      *ptr++ = heightmap_[i * kWidth + j];
      *ptr++ = heightmap_[i * kWidth + j];
      *ptr++ = heightmap_[i * kWidth + j];
      *ptr++ = 0.0;
    }
  }
}
