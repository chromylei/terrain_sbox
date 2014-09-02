#include "tersbox/roam/err_metric/roam.h"

ROAMTree::ROAMTree(azer::Tile* tile, const int minlevel)
    : tile_(tile)
    , pitch_(0, 0, tile->GetGridLineNum() - 1, tile->GetGridLineNum() - 1)
    , left_root_(NULL)
    , right_root_(NULL)
    , node_num_(0)
    , kMinWidth(1 << minlevel) {
  int grid = tile->GetGridLineNum() + 1;
  variance_.reset(new uint8[grid * grid]);
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
  if (std::abs(tri.apexx - tri.leftx) > kMinWidth
      || std::abs(tri.apexy - tri.lefty) > kMinWidth) {
    int centx = (tri.leftx + tri.rightx) >> 1;
    int centy = (tri.lefty + tri.righty) >> 1;
    int index = centy * tile_->GetGridLineNum() + centx;
    uint8 variance = variance_[index];
    if (variance < 4) { return;}
    
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

void ROAMTree::Init() {
  ROAMTree::CalcVariance();
}

void ROAMTree::CalcVariance() {
  int grid = tile_->GetGridLineNum() + 1;
  memset(variance_.get(), 0, grid * grid * sizeof(uint8));
  ROAMTree::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMTree::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);
  RecursCalcVariable(l, variance_.get());
  RecursCalcVariable(r, variance_.get());
}

uint8 ROAMTree::RecursCalcVariable(const Triangle& tri, uint8* vararr) {  
  int centx = (tri.leftx + tri.rightx) >> 1;
  int centy = (tri.lefty + tri.righty) >> 1;
  uint8 height = (uint8)(tile_->vertex(centx, centy).y);
  uint8 lheight = (uint8)(tile_->vertex(tri.leftx, tri.lefty).y);
  uint8 rheight = (uint8)(tile_->vertex(tri.rightx, tri.righty).y);
  uint8 variance = std::abs((uint8)height
                            - (((uint8)lheight + (uint8)rheight) >> 1));

  Triangle l, r;
  split_triangle(tri, &l, &r);
  if (std::abs(tri.leftx - tri.rightx) > kMinWidth
      || std::abs(tri.lefty - tri.righty) > kMinWidth) {
    variance = std::max(variance, RecursCalcVariable(l, vararr));
    variance = std::max(variance, RecursCalcVariable(r, vararr));
  }

  int grid = tile_->GetGridLineNum() + 1;
  int index = centy * grid + centx;
  DCHECK_LT(index, grid * grid);
  variance = std::max(vararr[index], variance);
  vararr[index] = variance;
  return variance;
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
