#pragma once

#include "base/basictypes.h"
#include "azer/util/tile.h"

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
int32* InitPitchFan(const azer::util::Tile::Pitch& pitch, int kGridLine,
                      int32* indices, uint32 flags = kSplitAll);
