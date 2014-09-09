#include "tersbox/roam/landscape/roam.h"

#include <sstream>
#include <iostream>
#include "azer/render/render.h"

const int kMinVariance = 8;
#define SQR(x) ((x) * (x))

ROAMPitch::ROAMPitch(azer::Tile* tile, azer::Tile::Pitch& pitch,
                     const int minlevel)
    : tile_(tile)
    , pitch_(pitch)
    , kMinWidth(1 << minlevel) {
  int grid = tile->GetGridLineNum() + 1;
  variance_.reset(new uint8[grid * grid]);
}

#ifdef _DEBUG
void ROAMPitch::SplitNode(BiTriTreeNode* pnode, const Triangle& tri) {
#else
void ROAMPitch::SplitNode(BiTriTreeNode* pnode) {
#endif
  if (pnode->left_child != NULL) return;
  /**
  * for eaxmple, splite top triangle, the bottom triangle must be split twice
  * the following complete the code
  *    *
  *   /|\
  *  / | \
  * /__|__\
  * |     /
  * |    /
  * |   /
  * |  /
  * | /
  * |/
  */
  if (pnode->base_neighbor && pnode->base_neighbor->base_neighbor != pnode) {
#ifdef _DEBUG
    SplitNode(pnode->base_neighbor, pnode->base_neighbor->triangle);
#else
    SplitNode(pnode->base_neighbor);
#endif
  }

  pnode->left_child = allocate();
  pnode->right_child = allocate();
  BiTriTreeNode* lchild = pnode->left_child;
  BiTriTreeNode* rchild = pnode->right_child;
#ifdef _DEBUG
  Triangle l, r;
  split_triangle(tri, &l, &r);
  lchild->triangle = l;
  rchild->triangle = r;
#endif
  
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
      BiTriTreeNode* blchild = bneighbor->left_child;
      BiTriTreeNode* brchild = bneighbor->right_child;
      brchild->left_neighbor = lchild;
      blchild->right_neighbor = rchild;
      lchild->right_neighbor = bneighbor->right_child;
      rchild->left_neighbor = bneighbor->left_child;
    } else {
#ifdef _DEBUG
      SplitNode(pnode->base_neighbor, pnode->base_neighbor->triangle);
#else
      SplitNode(pnode->base_neighbor);
#endif
    }
  } else {
    lchild->right_neighbor = NULL;
    rchild->left_neighbor = NULL;
  }

  if (pnode->base_neighbor) {
    // DCHECK(pnode == pnode->base_neighbor->base_neighbor);
    if (pnode != pnode->base_neighbor->base_neighbor) {
      LOG(ERROR) << "ERROR";
    }
  }
}

void ROAMPitch::split_triangle(const Triangle& tri, Triangle* l, Triangle* r) {
  int centx = (tri.leftx + tri.rightx) >> 1;
  int centy = (tri.lefty + tri.righty) >> 1;
  l->leftx  = tri.apexx; l->lefty  = tri.apexy;
  l->rightx  = tri.leftx; l->righty = tri.lefty;
  l->apexx  = centx; l->apexy  = centy;

  r->leftx  = tri.rightx, r->lefty  = tri.righty;
  r->rightx  = tri.apexx; r->righty  = tri.apexy;
  r->apexx  = centx; r->apexy  = centy;
}

int32* ROAMPitch::indices(BiTriTreeNode* node, const Triangle& tri,
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

int32* ROAMPitch::indices(int32* indicesptr) {
  ROAMPitch::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMPitch::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);
  int32* cur = indices(&left_root_, l, indicesptr);
  return indices(&right_root_, r, cur);
}

void ROAMPitch::RecursSplit(BiTriTreeNode* pnode, const Triangle& tri,
                           const azer::Camera& camera) {
#ifdef _DEBUG
  SplitNode(pnode, tri);
#else
  SplitNode(pnode);
#endif

  if (std::abs(tri.apexx - tri.leftx) > kMinWidth
      || std::abs(tri.apexy - tri.lefty) > kMinWidth) {
    int centx = (tri.leftx + tri.rightx) >> 1;
    int centy = (tri.lefty + tri.righty) >> 1;
    uint8 var = variance(centx, centy);

    const azer::Vector3& vertex = tile_->vertex(centx, centy);
    float distance = 1.0f + sqrtf(SQR(vertex.x - camera.position().x)
                                  + SQR(vertex.z - camera.position().z));
    float tri_var = var * (1 << tile_->level()) * 2 / distance;
    if (tri_var > 50) {
      Triangle l, r;
      split_triangle(tri, &l, &r);
      RecursSplit(pnode->left_child, l, camera);
      RecursSplit(pnode->right_child, r, camera);
    }
  }
}

void ROAMPitch::tessellate(const azer::Camera& camera) {
  reset();

  ROAMPitch::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMPitch::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);

  RecursSplit(&left_root_, l, camera);
  RecursSplit(&right_root_, r, camera);
}

bool ROAMPitch::IsVisible(const azer::Camera& camera) {
  azer::Frustrum::CheckVisibleOption opt = azer::Frustrum::kCheckWithoutHeight;
  const azer::Vector3& p1 = tile_->vertex(pitch_.left, pitch_.top);
  const azer::Vector3& p2 = tile_->vertex(pitch_.right, pitch_.top);
  const azer::Vector3& p3 = tile_->vertex(pitch_.left, pitch_.bottom);
  const azer::Vector3& p4 = tile_->vertex(pitch_.right, pitch_.bottom);
  azer::VisibleState v1 = camera.frustrum().IsVisible(p1, opt);
  azer::VisibleState v2 = camera.frustrum().IsVisible(p2, opt);
  azer::VisibleState v3 = camera.frustrum().IsVisible(p3, opt);
  azer::VisibleState v4 = camera.frustrum().IsVisible(p4, opt);
  return v1 == azer::kFullyVisible
      || v2 == azer::kFullyVisible
      || v3 == azer::kFullyVisible
      || v4 == azer::kFullyVisible;
}

void ROAMPitch::Init() {
  ROAMPitch::CalcVariance();

  ROAMPitch::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMPitch::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);

  left_root_.base_neighbor = &right_root_;
  right_root_.base_neighbor = &left_root_;
#ifdef _DEBUG
  left_root_.triangle = l;
  right_root_.triangle = r;
#endif
}

void ROAMPitch::CalcVariance() {
  int grid = tile_->GetGridLineNum() + 1;
  memset(variance_.get(), 0, grid * grid * sizeof(uint8));
  ROAMPitch::Triangle l(pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.top,
                       pitch_.left, pitch_.top);
  ROAMPitch::Triangle r(pitch_.right, pitch_.top,
                       pitch_.left, pitch_.bottom,
                       pitch_.right, pitch_.bottom);
  RecursCalcVariable(l, variance_.get());
  RecursCalcVariable(r, variance_.get());

  /*
  std::stringstream ss;
  for (int i = 0; i < grid; ++i) {
    for (int j = 0; j < grid; ++j) {
      ss << (int)(variance_[i * grid + j]) << " ";
    }
    ss << "\n";
  }
  std::cout << ss.str() << std::endl;
  */
}

void ROAMPitch::set_variance(int x, int y, uint8 var) {
  int grid = tile_->GetGridLineNum() + 1;
  int index = y * grid + x;
  DCHECK_LT(index, grid * grid);
  variance_.get()[index] = var;
}

uint8 ROAMPitch::variance(int x, int y) {
  int grid = tile_->GetGridLineNum() + 1;
  int index = y * grid + x;
  DCHECK_LT(index, grid * grid);
  return variance_.get()[index];
}

uint8 ROAMPitch::RecursCalcVariable(const Triangle& tri, uint8* vararr) {  
  int centx = (tri.leftx + tri.rightx) >> 1;
  int centy = (tri.lefty + tri.righty) >> 1;
  uint8 height = (uint8)(tile_->vertex(centx, centy).y);
  uint8 lheight = (uint8)(tile_->vertex(tri.leftx, tri.lefty).y);
  uint8 rheight = (uint8)(tile_->vertex(tri.rightx, tri.righty).y);
  uint8 var = std::abs((uint8)height
                            - (((uint8)lheight + (uint8)rheight) >> 1));
  Triangle l, r;
  split_triangle(tri, &l, &r);
  if (std::abs(tri.leftx - tri.rightx) > kMinWidth
      || std::abs(tri.lefty - tri.righty) > kMinWidth) {
    var = std::max(var, RecursCalcVariable(l, vararr));
    var = std::max(var, RecursCalcVariable(r, vararr));
  }

  var = std::max(variance(centx, centy), var);
  set_variance(centx, centy, var);
  return var;
}

azer::AxisAlignedBox ROAMPitch::CalcTriAABB(const Triangle& t) {
  const azer::Vector3& pos1 = tile_->vertex(t.leftx, t.lefty);
  const azer::Vector3& pos2 = tile_->vertex(t.rightx, t.righty);
  const azer::Vector3& pos3 = tile_->vertex(t.apexx, t.apexy);
  azer::Vector3 min_vert(std::min(std::min(pos1.x, pos2.x), pos3.x),
                         std::min(std::min(pos1.y, pos2.y), pos3.y),
                         std::min(std::min(pos1.z, pos2.z), pos3.z));
  azer::Vector3 max_vert(std::max(std::max(pos1.x, pos2.x), pos3.x),
                         std::max(std::max(pos1.y, pos2.y), pos3.y),
                         std::max(std::max(pos1.z, pos2.z), pos3.z));
  azer::AxisAlignedBox aabb(min_vert, max_vert);
  return aabb;
}

ROAMPitch::Arena::~Arena() {
  for (auto iter = block_.begin(); iter != block_.end(); ++iter) {
    delete *iter;
  }
}

ROAMPitch::BiTriTreeNode* ROAMPitch::allocate() {
  BiTriTreeNode* node = arena_.allocate();
  memset(node, 0, sizeof(BiTriTreeNode));
  return node;
}

ROAMPitch::BiTriTreeNode* ROAMPitch::Arena::allocate() {
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

void ROAMPitch::Arena::reset() {
  vec_index_ = 0;
  node_num_ = 0;

  
  for (auto iter = block_.begin(); iter != block_.end(); ++iter) {
    // BiTriTreeNodeVec& vec = *(*iter);
    // for (auto viter = vec.begin(); viter != vec.end();  viter++) {
    // }
    // memset(&(vec[0]), 0, vec.size() * sizeof(BiTriTreeNode));
  }
}
