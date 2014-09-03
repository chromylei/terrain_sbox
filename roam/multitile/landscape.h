#pragma once

#include "tersbox/roam/multitile/roam.h"
#include "base/basictypes.h"
#include "azer/render/render.h"

class LandScape {
 public:
  LandScape() {
    memset(page_, 0, sizeof(page));
    visible_num_ = 0;
  }

  void update(const Camera& camera);
  int32* indices(int32* indices);
 private:
  static const int kPagePerRow = 4;
  ROAMPitch* page_[kPagePerRow * kPagePerRow];
  int visible_page_[kPagePerRow * kPagePerRow];
  int visible_num_;
  DISALLOW_COPY_AND_ASSIGN(LandScape);
};
