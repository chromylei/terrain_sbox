#include "tersbox/roam/multitile/roam.h"

void LandScape::update(const Camera& camera) {
  visible_num_ = 0;

  int cnt = 0;
  for (int i = 0; i < kPagePerRow; ++i) {
    for (int j = 0; j < kPagePerRow; ++j) {
      int index = i * kPagePerRow +j;
      if (j > 0) {
        page_[index]->SetLeftNeighbor(page_[index - 1]);
      }
      if (i > 0) {
        page_[index]->SetTopNeighbor(page_[index - kPagePerRow]);
      }
      page_[cnt] = cnt;
      cnt++;
      visible_num_++;
    }
  }

  for (int i = 0; i < visible_num_; ++i) {
    page_[visible_page_[i]]->tesselate(camera);
  }
}

int32* LandScape::indices(int32* indicesptr) {
  int32* cur = indicesptr;
  for (int i = 0; i < visible_num_; ++i) {
    cur = page_[visible_page_[i]]->indices(cur);
  }
  return cur;
}
