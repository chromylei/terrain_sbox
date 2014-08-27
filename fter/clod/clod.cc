#include "tersbox/fter/clod/clod.h"

#include "azer/util/tile.h"

using azer::util::Tile;


int32* Clod::GenIndices(int32* indices, uint32 flags) {
  azer::util::Tile::Pitch pitch;
  int32* cur = indices;
  for (int i = 0; i + 4< tile_->GetGridLineNum(); i += 4) {
    for (int j = 0; j + 4 < tile_->GetGridLineNum(); j += 4) {
      pitch.top = i;
      pitch.bottom = i + 4;
      pitch.left = j;
      pitch.right = j + 4;
      cur = GenIndices(pitch, cur, flags);
    }
  }

  return cur;
}

int32* Clod::GenIndices(azer::util::Tile::Pitch& pitch, int32* indices,
                        uint32 flags) {
  return InitPitchFan(pitch, tile_->GetGridLineNum(), indices, flags);
}

int32* Clod::InitPitchFan(const Tile::Pitch& pitch, int kGridLine,
                          int32* indices, uint32 flags) {
  DCHECK_EQ(pitch.right - pitch.left, pitch.bottom - pitch.top);
  const int step = (pitch.right - pitch.left) / 2;

  int topline = pitch.top * kGridLine;
  int midline = (pitch.top + step) * kGridLine;
  int bottomline = pitch.bottom * kGridLine;
  int32* cur = indices;

  // top
  if (flags & 0x00000001) {
    *cur++ = midline + step + pitch.left;
    *cur++ = topline + step + pitch.left;
    *cur++ = topline + pitch.left;

    *cur++ = midline + step + pitch.left;
    *cur++ = topline + pitch.right;
    *cur++ = topline + step + pitch.left;
    
  } else {
    *cur++ = midline + step + pitch.left;
    *cur++ = topline + pitch.right;
    *cur++ = topline + pitch.left;
  }

  // bottom
  if (flags & 0x00000002) {
    *cur++ = midline + step + pitch.left;
    *cur++ = bottomline + pitch.left;
    *cur++ = bottomline + step + pitch.left;

    *cur++ = midline + step + pitch.left;
    *cur++ = bottomline + step + pitch.left;
    *cur++ = bottomline + pitch.right;
  } else {
    *cur++ = midline + step + pitch.left;
    *cur++ = bottomline + pitch.left;
    *cur++ = bottomline + pitch.right;
  }

  // left
  if (flags & 0x00000004) {
    *cur++ = midline + step + pitch.left;
    *cur++ = topline + pitch.left;
    *cur++ = midline + pitch.left;

    *cur++ = midline + step + pitch.left;
    *cur++ = midline + pitch.left;
    *cur++ = bottomline + pitch.left;
  } else {
    *cur++ = midline + step + pitch.left;
    *cur++ = topline + pitch.left;
    *cur++ = bottomline + pitch.left;
  }

  // left
  if (flags & 0x00000008) {
    *cur++ = midline + step + pitch.left;
    *cur++ = midline + pitch.right;
    *cur++ = topline + pitch.right;

    *cur++ = midline + step + pitch.left;
    *cur++ = bottomline + pitch.right;
    *cur++ = midline + pitch.right;
  } else {
    *cur++ = midline + step + pitch.left;
    *cur++ = bottomline + pitch.right;
    *cur++ = topline + pitch.right;
  }

  return cur;
}
