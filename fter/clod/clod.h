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
 private:
  azer::util::Tile* tile_;
  DISALLOW_COPY_AND_ASSIGN(Clod);
};
