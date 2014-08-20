#include "tersbox/fter/base/terrain_data.h"


#include <cmath>
#include "base/rand_util.h"
#include "azer/util/image.h"


void TerrainData::Gen(float heightrange, float fRoughness) {
  Gen(0, 0, kWidth - 1, kWidth - 1, heightrange, fRoughness);
}

void TerrainData::Gen(int left, int top, int right, int bottom,
                      float heightrange, float fRoughness) {
  if (right - left <= 1) {
    return;
  }

  
  int midpos_x = (left + right) / 2;
  int midpos_y = (top + bottom) / 2;
  (heightmap_.get())[midpos_y * kWidth + midpos_x] =
      ::base::RandDouble() * heightrange - heightrange / 2.0f;

  float hrange = heightrange * std::pow(-1, fRoughness);
  Gen(left, top, midpos_x, midpos_y, hrange, fRoughness);
  Gen(midpos_x, top, right, midpos_y, hrange, fRoughness);
  Gen(left, midpos_y, midpos_x, bottom, hrange, fRoughness);
  Gen(midpos_x, midpos_y, right, bottom, hrange, fRoughness);
}

bool TerrainData::Save(const ::base::FilePath& path) {
  azer::Image image(kWidth, kWidth, azer::kRGBA8);
  uint8* ptr = (uint8*)image.data();
  for (int i = 0; i < kWidth; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      uint8 val = (uint8)((heightmap_.get())[i * kWidth + j]);
      *ptr++ = val;
      *ptr++ = val;
      *ptr++ = val;
      *ptr++ = 255;
    }
  }

  return azer::util::SaveImage(&image, path);
}
