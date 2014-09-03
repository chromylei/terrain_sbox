#pragma once

#include "tersbox/roam/landscape/roam.h"
#include "base/basictypes.h"
#include "azer/render/render.h"

class LandScape {
 public:
  LandScape() {
    memset(page_, 0, sizeof(page_));
    visible_num_ = 0;
  }

  void update(const azer::Camera& camera);
  int32* indices(int32* indices);

  void SetROAMPitch(int row, int col, ROAMPitchPtr ptr);
 private:
  static const int kPagePerRow = 8;
  ROAMPitchPtr page_[kPagePerRow * kPagePerRow];
  int visible_page_[kPagePerRow * kPagePerRow];
  int visible_num_;
  DISALLOW_COPY_AND_ASSIGN(LandScape);
};

inline void LandScape::SetROAMPitch(int row, int col, ROAMPitchPtr ptr) {
  int index = kPagePerRow * row + col;
  page_[index] = ptr;
}
