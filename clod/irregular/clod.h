#pragma once

#include "base/basictypes.h"
#include "azer/util/tile.h"

class Clod {
 public:
  Clod(azer::Tile* tile) : tile_(tile) {}

  /**
   *  init indices for following geo
   *       *-----*-----*
   *       | \   |   / |
   *       |   \ | /   |
   *       *-----*-----*
   *       |   / | \   |
   *       | /   |   \ |
   *       *-----*-----*
   */

  enum PitchFanSplit {
    kSplitTop    = 0x00000001,
    kSplitBottom = 0x00000002,
    kSplitLeft   = 0x00000004,
    kSplitRight  = 0x00000008,
    kSplitOne    = 0x00000010,
    kSplitAll    = 0x0000000F,
  };
  static int32* InitPitchFan(const azer::Tile::Pitch& pitch, int kGridLine,
                             int32* indices, uint32 flags = kSplitAll);

  /**
   * generate indices for pitch
   */
  int32* GenIndices(std::vector<azer::Tile::Pitch>* pitch,
                    int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(azer::Tile::Pitch& pitch, int32* indices,
                    uint32 flags = kSplitAll);

  /**
   * 根据给定的分割级别进行分割
   * 注意在分割的时候必须保证分割方式能够切好填满整个 tile, 不能出现分割半个的情况
   * 例如 在边缘处的 level == 2
   * 生成 splitlevel 的算法其实很简单就是
   *  level = 0 必须以 4 * 4 的方式出现
   *  level = 1 必须以 2 * 2 的方式出现
   *  level = 2 则无所谓
   */
  int32* GenIndices(int32* indices, int32* splitlevel);
  int32* GenIndices(const azer::Tile::Pitch& pitch,
                    int32* indices, int32* splitlevel);
  void CalcLOD();
  int level(int x, int y, int32* level);
 private:
  int left_level(int x, int y, int step, int32* level);
  int right_level(int x, int y, int step, int32* level);
  int top_level(int x, int y, int step, int32* level);
  int bottom_level(int x, int y, int step, int32* level);
  azer::Tile* tile_;
  std::unique_ptr<int32> levels_;
  DISALLOW_COPY_AND_ASSIGN(Clod);
};


inline int Clod::left_level(int x, int y, int step, int32* level) {
  int newx = (x < step ? 0 : x - step);
  return level[y * tile_->GetGridLineNum() + newx];
}

inline int Clod::right_level(int x, int y, int step, int32* level) {
  int newx = ((x < tile_->GetGridLineNum() - step) ? x + step : x);
  return level[y * tile_->GetGridLineNum() + newx];
}

inline int Clod::top_level(int x, int y, int step, int32* level) {
  int newy = (y < step ? 0 : y - step);
  return level[newy * tile_->GetGridLineNum() + x];
}

inline int Clod::bottom_level(int x, int y, int step, int32* level) {
  int newy = (y < tile_->GetGridLineNum() + step ? y + step : y);
  return level[newy * tile_->GetGridLineNum() + x];
}
