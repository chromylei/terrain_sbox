#include "tersbox/roam/indices/roam.h"

ROAMTree::ROAMTree(azer::Tile* tile)
    : tile_(tile)
    , pitch_(0, 0, tile->GetGridLineNum() - 1, tile->GetGridLineNum() - 1)
    , left_root_(NULL)
    , right_root_(NULL)
    , node_num_(0) {
}

void ROAMTree::SplitNode(BiTriTreeNode* pnode) {
  if (pnode->left_child != NULL) return;

  pnode->left_child = allocate();
  pnode->right_child = allocate();
  BiTriTreeNode* lchild = pnode->left_child;
  BiTriTreeNode* rchild = pnode->right_child;
  lchild->base_neighbor = pnode->left_neighbor;
  lchild->left_neighbor = rchild;

  rchild->base_neighbor = pnode->right_neighbor;
  rchild->right_neighbor = lchild;
  
  if (pnode->left_neighbor) {
    BiTriTreeNode* lneighbor = pnode->left_neighbor;
    if (lneighbor->base_neighbor == pnode) {
      lneighbor->base_neighbor = lchild;
    } else if (lneighbor->left_neighbor == pnode) {
      lneighbor->left_neighbor = lchild;
    } else if (lneighbor->right_neighbor == pnode) {
      lneighbor->right_neighbor = lchild;
    } else {
      NOTREACHED();
    }
  }

  if (pnode->right_neighbor) {
    BiTriTreeNode* rneighbor = pnode->right_neighbor;
    if (rneighbor->base_neighbor == pnode) {
      rneighbor->base_neighbor = rchild;
    } else if (rneighbor->left_neighbor == pnode) {
      rneighbor->left_neighbor = rchild;
    } else if (rneighbor->right_neighbor == pnode) {
      rneighbor->right_neighbor = rchild;
    } else {
      NOTREACHED();
    }
  }

  if (pnode->base_neighbor) {
    BiTriTreeNode* bneighbor = pnode->base_neighbor;
    if (bneighbor->left_child) {
      BiTriTreeNode* blchild = pnode->base_neighbor->left_child;
      BiTriTreeNode* brchild = pnode->base_neighbor->right_child;
      blchild->left_neighbor = lchild;
      brchild->right_neighbor = rchild;
      lchild->right_neighbor = pnode->base_neighbor->left_child;
      rchild->left_neighbor = pnode->base_neighbor->right_child;
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

int32* ROAMTree::indices(BiTriTreeNode* node, const Triangle& tri,
                         int32* indices_ptr) {
  int32* cur = indices_ptr;
  if (node->left_child) {
    Triangle l, r;
    split_triangle(tri, &l, &r);
    cur = indices(node->left_child, l, cur);
    cur = indices(node->right_child, r, cur);
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
  int32* cur = indices(left_root_, l, indicesptr);
  return indices(right_root_, r, cur);
}

void ROAMTree::RecursSplit(BiTriTreeNode* pnode, const Triangle& tri) {
  if (std::abs(tri.apexx - tri.leftx) > 1
      || std::abs(tri.apexy - tri.lefty) > 1) {
    SplitNode(pnode);
    Triangle l, r;
    split_triangle(tri, &l, &r);
    RecursSplit(pnode->left_child, l);
    RecursSplit(pnode->right_child, r);
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
  left_root_ = allocate();
  right_root_ = allocate();
  RecursSplit(left_root_, l);
  RecursSplit(right_root_, r);
}

ROAMTree::BiTriTreeNode* ROAMTree::allocate() {
  return arena_.allocate();
}

ROAMTree::BiTriTreeNode* ROAMTree::Arena::allocate() {
  if (vec_index_ < block_.size()) {
    if (node_num_ < kBlockSize) {
      BiTriTreeNodeVec& vec = *block_[vec_index_];
      return &(vec[node_num_++]);
    } else {
      vec_index_++;
      node_num_ = 0;
      return allocate();
    }
  } else {
    block_.push_back(new BiTriTreeNodeVec(kBlockSize));
    return allocate();
  }
}

void ROAMTree::Arena::reset() {
  vec_index_ = 0;
  node_num_ = 0;
}
