#include "azer/util/tile.h"

using azer::util::Tile;

int32* InitPitchTopFan(const Tile::Pitch& pitch, int kGridLine,
                       int32* indices, uint32 flags) {
  DCHECK_EQ(pitch.right - pitch.left, pitch.bottom - pitch.top);
  const int step = (pitch.right - pitch.left) / 2;

  int topline = pitch.top * kGridLine;
  int midline = (pitch.top + step) * kGridLine;
  int bottomline = pitch.bottom * kGridLine;
  int32* cur = indices;

  // top
  if (flags & 0x00000001) {
    *cur++ = midline + step;
    *cur++ = topline + step;
    *cur++ = topline + pitch.left;

    *cur++ = midline + step;
    *cur++ = topline + pitch.right;
    *cur++ = topline + step;
    
  } else {
    *cur++ = midline + step;
    *cur++ = topline + pitch.right;
    *cur++ = topline + pitch.left;
  }

  // bottom
  if (flags & 0x00000002) {
    *cur++ = midline + step;
    *cur++ = bottomline + pitch.left;
    *cur++ = bottomline + step;

    *cur++ = midline + step;
    *cur++ = bottomline + step;
    *cur++ = bottomline + pitch.right;
  } else {
    *cur++ = midline + step;
    *cur++ = bottomline + pitch.left;
    *cur++ = bottomline + pitch.right;
  }

  // left
  if (flags & 0x00000004) {
    *cur++ = midline + step;
    *cur++ = topline + pitch.left;
    *cur++ = midline + pitch.left;

    *cur++ = midline + step;
    *cur++ = midline + pitch.left;
    *cur++ = bottomline + pitch.left;
  } else {
    *cur++ = midline + step;
    *cur++ = topline + pitch.left;
    *cur++ = bottomline + pitch.left;
  }

  // left
  if (flags & 0x00000008) {
    *cur++ = midline + step;
    *cur++ = midline + pitch.right;
    *cur++ = topline + pitch.right;

    *cur++ = midline + step;
    *cur++ = bottomline + pitch.right;
    *cur++ = midline + pitch.right;
  } else {
    *cur++ = midline + step;
    *cur++ = bottomline + pitch.right;
    *cur++ = topline + pitch.right;
  }

  return cur;
}
