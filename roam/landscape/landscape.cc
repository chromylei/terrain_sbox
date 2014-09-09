#include "tersbox/roam/landscape/landscape.h"


#include "base/logging.h"
#include "azer/render/render.h"

void LandScape::update(const azer::Camera& camera) {
  memset(visible_flags_, 0, sizeof(visible_flags_));
  for (int row = 0; row < kPagePerRow; ++row) {
    for (int col = 0; col < kPagePerRow; ++col) {
      int index = row * kPagePerRow + col;
      ROAMPitchPtr& ptr = page_[index];
      /*
      if (!ptr->IsVisible(camera)) {
        continue;
      }
      */

      visible_flags_[index] = 1;
      if (col > 0) {
        ROAMPitchPtr& neighbor = page_[index - 1];
        DCHECK(neighbor.get() != NULL);
        ptr->SetLeftNeighbor(neighbor.get());
        visible_flags_[index - 1] = 1;
      }
      if (row > 0) {
        ROAMPitchPtr& neighbor = page_[index - kPagePerRow];
        DCHECK(neighbor.get() != NULL);
        ptr->SetTopNeighbor(neighbor.get());
        visible_flags_[index - kPagePerRow] = 1;
      }
    }
  }

  for (int row = 0; row < kPagePerRow; ++row) {
    for (int col = 0; col < kPagePerRow; ++col) {
      int index = row * kPagePerRow + col;
      if (visible_flags_[index]) {
        ROAMPitchPtr& ptr = page_[index];
        ptr->tessellate(camera);
      }
    }
  }
}

int32* LandScape::indices(int32* indicesptr) {
  int32* cur = indicesptr;
  for (int row = 0; row < kPagePerRow; ++row) {
    for (int col = 0; col < kPagePerRow; ++col) {
      int index = row * kPagePerRow + col;
      if (visible_flags_[index]) {
        ROAMPitchPtr& ptr = page_[index];
        cur = ptr->indices(cur);
      }
    }
  }

  return cur;
}

