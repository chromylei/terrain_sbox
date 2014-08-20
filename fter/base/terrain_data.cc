#include "tersbox/fter/base/terrain_data.h"


#include <cmath>
#include <limits>
#include "base/rand_util.h"
#include "azer/util/image.h"


void TerrainData::GenPlasma() {
  memset(heightmap_.get(), 0, sizeof(float) * kWidth * kWidth);
  int rect = kWidth;
  float hr = 256;
  SetHeight(0, 0, 0.0f);
  while (rect > 0) {
    /*
      a.......b
      .       .
      .       .
      .   e   .
      .       .
      .       .
      c.......d
     */
    for (int i = 0 ; i < kWidth; i+= rect) {
      for (int j = 0 ; j < kWidth; j+= rect) {
        float height_a = GetHeight(i, j);
        float height_b = GetHeight(i, (j + rect) % kWidth);
        float height_c = GetHeight((i + rect) % kWidth, j);
        float height_d = GetHeight((i + rect) % kWidth, (j + rect) % kWidth);
        float avg = (height_a + height_b + height_c + height_d) / 4.0;
        SetHeight(i + rect / 2, j + rect / 2, GenHeight(avg, hr));
      }
    }

    /*
           .......
           .     .
           .     .
           .  d  .
           .     .
           .     .
     ......a..g..b
     .     .     .
     .     .     .
     .  e  h  f  .
     .     .     .
     .     .     .
     ......c......
     g = (d+f+a+b)/4 + random
     h = (a+c+e+f)/4 + random
    */
    for (int i = 0; i < kWidth; i+= rect) {
      for (int j = 0; j < kWidth; j += rect) {
        float height_a = GetHeight(i, j);
        float height_b = GetHeight(i, (j + rect) % kWidth);
        float height_c = GetHeight((i + rect) % kWidth, j);
        float height_d = GetHeight((i - rect / 2 + kWidth) % kWidth, j + rect / 2);
        float height_e = GetHeight(i + rect / 2, (j - rect / 2 + kWidth) % kWidth);
        float height_f = GetHeight(i + rect / 2, j + rect / 2);
        float height_g_avg = (height_a + height_b + height_d + height_f) / 4;
        float height_g = GenHeight(height_g_avg, hr);
        float height_h_avg = (height_a + height_c + height_e + height_f) / 4;
        float height_h = GenHeight(height_h_avg, hr);
        SetHeight(i, j + rect / 2, height_g);
        SetHeight(i + rect / 2, j, height_h);
      }
    }

    rect /= 2;
    hr *= std::pow(2, -fRoughness);
  }
}

float TerrainData::GenHeight(float avg, float heightrange) {
  double r = ::base::RandDouble();
  return avg + (r - 0.5) * heightrange;
}

void TerrainData::NormalizeHeightmap() {
  float minh = std::numeric_limits<float>::max();
  float maxh = -std::numeric_limits<float>::max();
  for (int i = 0; i < kWidth; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      float h = GetHeight(i, j);
      if (h > maxh) maxh = h;
      if (h < minh) minh = h;
    }
  }

  float range = maxh - minh;
  for (int i = 0; i < kWidth; ++i) {
    for (int j = 0; j < kWidth; ++j) {
      float h = (GetHeight(i, j) - minh) / range * fRange;
      SetHeight(i, j, h);
    }
  }
}

bool TerrainData::Save(const ::base::FilePath& path) {
  NormalizeHeightmap();

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
  CHECK (x >= 0 && x < kWidth && y >= 0 && y < kWidth);
  float* heightmap = heightmap_.get();
  return heightmap[x * kWidth + y];
}

