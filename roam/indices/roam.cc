#include "tersbox/roam/indices/roam.h"

ROAMPitch::ROAMPitch(azer::Tile* tile)
    : tile_(tile)
    , node_num_(0)
    , kMaxNodeNum(std::pow(2.0f, tile_->level() + 1)) {
  nodes_.reset(new BiTriTreeNode[]);
}

void ROAMPitch::tessellate() {
}

void ROAMPitch::RecursSplit(int pnode_index, int leftx, int lefty,
                            int rightx, int righty, int apexx, int apexy) {
  BiTriTreeNode* nodes = nodes_.get();
  BiTriTreeNode* pnode = nodes + pnode_index;
  pnode->haschild = true;
  const int lchild_index = pnode_index << 2;
  const int rchild_index = lchild_index + 1;
  BiTriTreeNode* lchild = nodes + lchild_index;
  BiTriTreeNode* rchild = lchild + 1;
  lchild->base_neighbor = pnode->left_neighbor;
  lchild->left_neighbor = rchild_index;

  rchild->base_neighbor = pnode->right_neighbor;
  rchild->right_neighbor = lchild_index;
  
  if (has_left_neighbor(pnode)) {
    BiTriTreeNode* lneighbor = nodes + pnode->left_neighbor;
    if (lneighbor->base_neighbor == pnode_index) {
      lneighbor->base_neighbor = lchild_index;
    } else if (lneighbor->left_neighbor == pnode_index) {
      lneighbor->left_neighbor = lchild_index;
    } else if (lneighbor->right_neighbor == pnode_index) {
      lneighbor->right_neighbor = lchild_index;
    } else {
      NOTREACHED();
    }
  }

  if (has_right_neighbor(pnode)) {
    BiTriTreeNode* rneighbor = nodes + pnode->right_neighbor;
    if (rneighbor->base_neighbor == pnode_index) {
      rneighbor->base_neighbor = rchild_index;
    } else if (rneighbor->left_neighbor == pnode_index) {
      rneighbor->left_neighbor = rchild_index;
    } else if (rneighbor->right_neighbor == pnode_index) {
      rneighbor->right_neighbor = rchild_index;
    } else {
      NOTREACHED();
    }
  }

  if (has_base_neighbor(pnode)) {
    BiTriTreeNode* bneighbor = nodes + pnode->base_neighbor;
    if (bneighbor->haschild) {
      BiTriTreeNode* blchild = nodes + pnode->base_neighbor >> 2;
      BiTriTreeNode* brchild = nodes + (pnode->base_neighbor >> 2) + 1;
      blchild->left_neighbor = lchild_index;
      brchild->right_neighbor = rchild_index;
      lchild->right_neighbor = pnode->base_neighbor >> 2;
      rchild->left_neighbor = (pnode->base_neighbor >> 2) + 1;
    } else {
    }
  } else {
  }
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
