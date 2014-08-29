#include "tersbox/roam/indices/roam.h"

ROAMTree::ROAMTree(azer::Tile* tile)
    : tile_(tile)
    , pitch_(0, 0, tile->GetGridLineNum() - 1, tile->GetGridLineNum() - 1)
    , node_num_(0)
    , kMaxNodeNum(std::pow(2.0f, std::pow(2.0f, tile_->level() + 1)))
  nodes_.reset(new BiTriTreeNode[kMaxNodeNum]);
}

void ROAMTree::SplitNode(int pnode_index) {
  BiTriTreeNode* nodes = nodes_.get();
  BiTriTreeNode* pnode = nodes + pnode_index;

  if (pnode->haschild) return;

  pnode->haschild = true;
  const int lchild_index = (pnode_index << 1) + 1;
  const int rchild_index = (pnode_index << 1) + 2;
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
      BiTriTreeNode* blchild = nodes + (pnode->base_neighbor << 1) + 1;
      BiTriTreeNode* brchild = nodes + (pnode->base_neighbor << 1) + 2;
      blchild->left_neighbor = lchild_index;
      brchild->right_neighbor = rchild_index;
      lchild->right_neighbor = (pnode->base_neighbor << 1) + 1;
      rchild->left_neighbor = (pnode->base_neighbor << 1) + 2;
    } else {
      SplitNode(pnode->base_neighbor);
    }
  } else {
  }

  node_num_ += 2;
}

void ROAMTree::split_triangle(const Triangle& tri, Triangle* l, Triangle* r) {
  int centx = (tri.leftx + tri.rightx) >> 1;
  int centy = (tri.lefty + tri.righty) >> 1;
  l->leftx  = tri.apexx; l->lefty  = tri.apexy;
  l->rightx  = tri.leftx; l->righty = tri.lefty;
  l->apexx  = centx; l->apexy  = centy;

  r->leftx  = tri.rightx, r->lefty  = tri.righty;
  r->rightx  = tri.apexx; r->righty  = tri.apexy;
  r->apexx  = centx; r->apexy  = centy;
}

int32* ROAMTree::indices(int node_index, const Triangle& tri, int32* indices_ptr) {
  int32* cur = indices_ptr;
  if (has_child(node_index)) {
    int lchild = (node_index << 1) + 1;
    int rchild = (node_index << 1) + 2;
    Triangle l, r;
    split_triangle(tri, &l, &r);
    cur = indices(lchild, l, cur);
    cur = indices(rchild, r, cur);
    return cur;
  } else {
    *cur++ = get_index(tri.leftx, tri.lefty);
    *cur++ = get_index(tri.rightx, tri.righty);
    *cur++ = get_index(tri.apexx, tri.apexy);
    return cur;
  }
}

int32* ROAMTree::indices(int32* indicesptr) {
  ROAMTree::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMTree::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);
  int32* cur = indices(1, l, indicesptr);
  return indices(2, r, cur);
}

void ROAMTree::RecursSplit(int pnode_index, const Triangle& tri) {
  if (std::abs(tri.apexx - tri.leftx) > 1
      || std::abs(tri.apexy - tri.lefty) > 1) {
    SplitNode(pnode_index);
    Triangle l, r;
    split_triangle(tri, &l, &r);
    RecursSplit((pnode_index << 1) + 1, l);
    RecursSplit((pnode_index << 1) + 2, r);
  }
}

void ROAMTree::tessellate() {
  ROAMTree::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMTree::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);
  reset();

  node_num_ = 3;
  (nodes_.get())[1].base_neighbor = 2;
  (nodes_.get())[2].base_neighbor = 1;
  RecursSplit(1, l);
  RecursSplit(2, r);
}
