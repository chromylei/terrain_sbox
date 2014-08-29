#include "tersbox/roam/indices/roam.h"

ROAMPitch::ROAMPitch(azer::Tile* tile)
    : tile_(tile)
    , node_num_(0)
    , kMaxNodeNum(std::pow(2.0f, tile_->level() + 1)) {
  nodes_.reset(new BiTriTreeNode[]);
}

void ROAMPitch::tessellate() {
}

void ROAMPitch::split() {
}

int32* ROAMPitch::indices(int node_index, int leftx, int lefty,
                          int rightx, int righty,
                          int apexx, int apexy, int32* indices_ptr) {
  
  int32* cur = indices_ptr;
  if (has_child(node_index)) {
    int centx = (leftx + rightx) >> 1;
    int centy = (lefty + righty) >> 1;
    int lchild = node_index << 2;
    int rchild = lchild + 1;
    cur = indices(lchild, apexx, apexy, leftx, lefty, centx, centy, cur);
    cur = indices(rchild, rightx, righty, apexx, apexy, centx, centy, cur);
    return cur;
  } else {
    *cur++ = get_index(leftx, lefty);
    *cur++ = get_index(rightx, righty);
    *cur++ = get_index(apexx, apexy);
    return cur;
  }
}
