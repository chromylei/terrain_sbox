#include "tersbox/roam/landscape/landscape.h"
#include "base/logging.h"

void LandScape::update(const azer::Camera& camera) {
  visible_num_ = 0;

  int cnt = 0;
  for (int row = 0; row < kPagePerRow; ++row) {
    for (int col = 0; col < kPagePerRow; ++col) {
      int index = row * kPagePerRow + col;
      if (col > 0) {
        ROAMPitchPtr ptr = page_[index - 1];
        DCHECK(ptr.get() != NULL);
        page_[index]->SetLeftNeighbor(ptr.get());
      }
      if (row > 0) {
        ROAMPitchPtr ptr = page_[index - kPagePerRow];
        page_[index]->SetTopNeighbor(ptr.get());
      }
      visible_page_[cnt] = cnt;
      cnt++;
      visible_num_++;
    }
  }

  for (int i = 0; i < visible_num_; ++i) {
    int index = visible_page_[i];
    page_[index]->tessellate(camera);
  }
}

int32* LandScape::indices(int32* indicesptr) {
  int32* cur = indicesptr;
  for (int i = 0; i < visible_num_; ++i) {
    cur = page_[visible_page_[i]]->indices(cur);
  }
  return cur;
}

