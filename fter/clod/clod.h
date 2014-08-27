#pragma once

#include "base/basictypes.h"
#include "azer/util/tile.h"

class Clod {
 public:
  Clod(azer::util::Tile* tile) : tile_(tile) {}

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
  static int32* InitPitchFan(const azer::util::Tile::Pitch& pitch, int kGridLine,
                             int32* indices, uint32 flags = kSplitAll);

  /**
   * generate indices for pitch
   */
  int32* GenIndices(std::vector<azer::util::Tile::Pitch>* pitch,
                    int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(azer::util::Tile::Pitch& pitch, int32* indices,
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
  int32* GenIndices(const azer::util::Tile::Pitch& pitch,
                    int32* indices, int32* splitlevel);
  void CalcLOD();
 private:
  azer::util::Tile* tile_;
  std::unique_ptr<int32> levels_;
  DISALLOW_COPY_AND_ASSIGN(Clod);
};


