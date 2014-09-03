#include "tersbox/roam/multitile/roam.h"

void LandScape::update(const Camera& camera) {
  visible_num_ = 0;

  for (int i = 0; i < kPagePerRow; ++i) {
    for (int j = 0; j < kPagePerRow; ++j) {
    }
  }
}

int32* LandScape::indices(int32* indicesptr) {
  int32* cur = indicesptr;
  for (int i = 0; i < visible_num_; ++i) {
    cur = page_[visible_page_[i]].indices(cur);
  }
  return cur;
}
