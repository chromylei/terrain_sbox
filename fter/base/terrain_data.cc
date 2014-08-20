#include "tersbox/fter/base/terrain_data.h"


#include <cmath>
#include "base/rand_util.h"
#include "azer/util/image.h"


void TerrainData::Gen() {
  float heightrange = fRange;
  float height1 = GenHeight(heightrange / 2, heightrange);
  float height2 = GenHeight(heightrange / 2, heightrange);
  float height3 = GenHeight(heightrange / 2, heightrange);
  float height4 = GenHeight(heightrange / 2, heightrange);
  SetHeight(0, 0, height1);
  SetHeight(0, kWidth-1, height2);
  SetHeight(kWidth-1, 0, height3);
  SetHeight(kWidth-1, kWidth-1, height4);
  float hrange = heightrange * std::pow(2, -fRoughness);
  Gen(0, 0, kWidth - 1, kWidth - 1, hrange);
}

void TerrainData::Gen(int left, int top, int right, int bottom,
                      float heightrange) {
  if (right - left <= 1 || bottom - top <= 1) {
    return;
  }
  int midpos_x = (left + right) / 2;
  int midpos_y = (top + bottom) / 2;
  float hrange = heightrange * std::pow(2, -fRoughness);

  GenCenter(left, top, right, bottom, heightrange);
  GenLeftMid(left, top, right, bottom, heightrange);
  GenRightMid(left, top, right, bottom, heightrange);
  GenTopMid(left, top, right, bottom, heightrange);
  GenBottomMid(left, top, right, bottom, heightrange);
  
  Gen(left, top, midpos_x, midpos_y, hrange);
  Gen(midpos_x, top, right, midpos_y, hrange);
  Gen(left, midpos_y, midpos_x, bottom, hrange);
  Gen(midpos_x, midpos_y, right, bottom, hrange);
}

float TerrainData::GenHeight(float avg, float heightrange) {
  double r = ::base::RandDouble();
  return avg + (r - 0.5) * heightrange;
}

bool TerrainData::Save(const ::base::FilePath& path) {
  azer::Image image(kWidth, kWidth, azer::kRGBA8);
  uint8* ptr = (uint8*)image.data();
  for (int i = 0; i < kWidth; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      uint8 val = (uint8)((heightmap_.get())[i * kWidth + j]) ;
      *ptr++ = val;
      *ptr++ = val;
      *ptr++ = val;
      *ptr++ = 255;
    }
  }

  return azer::util::SaveImage(&image, path);
}

void TerrainData::SetHeight(int x, int y, float h) {
  CHECK (x >= 0 && x < kWidth && y >= 0 && y < kWidth);
  float* heightmap = heightmap_.get();
  heightmap[x * kWidth + y] = h;
}

float TerrainData::GetHeight(int x, int y) {
  float* heightmap = heightmap_.get();
  if (x >= 0 && x < kWidth && y >= 0 && y < kWidth) {
    return heightmap[x * kWidth + y];
  } else {
    return 1.0f;
  }
}

void TerrainData::GenCenter(int left, int top, int right, int bottom,
                             float heightrange) {
  float* heightmap = heightmap_.get();
  float height1 = GetHeight(top, left);
  float height2 = GetHeight(top, right);
  float height3 = GetHeight(bottom, left);
  float height4 = GetHeight(bottom, right);
  float avg = (height1 + height2 + height3 + height4) / 4.0f;
  SetHeight((top + bottom) / 2, (left + right) / 2, GenHeight(avg, heightrange));
}

void TerrainData::GenTopMid(int left, int top, int right, int bottom,
                             float heightrange) {
  float* heightmap = heightmap_.get();
  float height1 = GetHeight(top, left);
  float height2 = GetHeight(top, right);
  float avg = (height1 + height2) / 2.0f;
  SetHeight(top, (left + right) / 2, GenHeight(avg, heightrange));
}

void TerrainData::GenBottomMid(int left, int top, int right, int bottom,
                                float heightrange) {
  float* heightmap = heightmap_.get();
  float height1 = GetHeight(bottom, left);
  float height2 = GetHeight(bottom, right);
  float avg = (height1 + height2) / 2.0f;
  SetHeight(bottom, (left + right) / 2, GenHeight(avg, heightrange));
}

void TerrainData::GenLeftMid(int left, int top, int right, int bottom,
                              float heightrange) {
  float* heightmap = heightmap_.get();
  float height1 = GetHeight(top, left);
  float height2 = GetHeight(bottom, left);
  float avg = (height1 + height2) / 2.0f;
  SetHeight((top + bottom) / 2, left, GenHeight(avg, heightrange));
}

void TerrainData::GenRightMid(int left, int top, int right, int bottom,
                               float heightrange) {
  float* heightmap = heightmap_.get();
  float height1 = GetHeight(top, right);
  float height2 = GetHeight(bottom, right);
  float avg = (height1 + height2) / 2.0f;
  SetHeight((top + bottom) / 2, right, GenHeight(avg, heightrange));
}
