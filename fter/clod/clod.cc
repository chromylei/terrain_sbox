#include "tersbox/fter/clod/clod.h"

#include "azer/util/tile.h"

using azer::Tile;


int32* Clod::GenIndices(int32* indices, uint32 flags) {
  azer::Tile::Pitch pitch;
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

int32* Clod::GenIndices(azer::Tile::Pitch& pitch, int32* indices,
                        uint32 flags) {
  return InitPitchFan(pitch, tile_->GetGridLineNum(), indices, flags);
}

int32* Clod::GenIndices(const azer::Tile::Pitch& pitch,
                        int32* indices, int32* levels) {
  int32* cur = indices;
  for (int i = pitch.top; i < pitch.bottom - 1; ++i) {
    for (int j = pitch.left; j < pitch.right - 1; ++j) {
      int level = levels[i * tile_->GetGridLineNum() + j];
      int step = std::pow(2.0f, 1 + level / 2);
      if (i % step == 0 && j % step == 0) {
        azer::Tile::Pitch p;
        p.top = i;
        p.bottom = i + step;
        p.left = j;
        p.right = j + step;

        uint32 flags = (level % 2 == 0) ? kSplitAll : 0;
        if (flags == kSplitAll) {
          if (level < left_level(j, i, step, levels)) {
            flags &= ~kSplitLeft;
          }

          if (level < right_level(j, i, step, levels)) {
            flags &= ~kSplitRight;
          }

          if (level < top_level(j, i, step, levels)) {
            flags &= ~kSplitTop;
          }

          if (level < bottom_level(j, i, step, levels)) {
            flags &= ~kSplitBottom;
          }
        }
        cur = InitPitchFan(p, tile_->GetGridLineNum(), cur, flags);
      }
    }
  }

  return cur;
}

int32* Clod::GenIndices(int32* indices, int32* levels) {
  azer::Tile::Pitch pitch;
  pitch.top = 0;
  pitch.left = 0;
  pitch.bottom = tile_->GetGridLineNum();
  pitch.right = tile_->GetGridLineNum();
  return GenIndices(pitch, indices, levels);
}

int32* Clod::InitPitchFan(const Tile::Pitch& pitch, int kGridLine,
                          int32* indices, uint32 flags) {
  DCHECK_EQ(pitch.right - pitch.left, pitch.bottom - pitch.top)
      << "width: " << pitch.right - pitch.left << " != "
      << "height: " << pitch.bottom - pitch.top;
  const int step = (pitch.right - pitch.left) / 2;

  int topline = pitch.top * kGridLine;
  int midline = (pitch.top + step) * kGridLine;
  int bottomline = pitch.bottom * kGridLine;
  int32* cur = indices;

  // top
  if (flags & kSplitTop) {
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
  if (flags & kSplitBottom) {
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
  if (flags & kSplitLeft) {
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
  if (flags & kSplitRight) {
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

void Clod::CalcLOD() {
}
