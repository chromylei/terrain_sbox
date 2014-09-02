#pragma once

#include <memory>
#include "azer/util/tile.h"
#include "base/basictypes.h"

/**
 * ROAMTree 分割 三角形 并生成 indices
 * 潜在的问题是，由于直接生成了 indices 而 normal 没有重新计算，因此所有的 normal 
 * 都是错误的。
 * 这个问题的重要程度需要评估，因为 error metric 会保证近处的 pitch 会被尽量分割
 * 因此实际上这个错误是很小的，对比两种实现方式
 * 如果使用 normalmap 这个问题就没有了
 *
 */

class ROAMTree {
 private:
  struct Triangle {
    int leftx;
    int lefty;
    int rightx;
    int righty;
    int apexx;
    int apexy;

    Triangle() {}
    Triangle(int lx, int ly, int rx, int ry, int ax, int ay)
        : leftx(lx), lefty(ly)
        , rightx(rx), righty(ry)
        , apexx(ax), apexy(ay) {
    }
  };
 public:
  ROAMTree(azer::Tile* tile, const int minlevel);

  /**
   * 如果单纯的使用 ROAM 算法进行分割是非常简单的，甚至不需要位置树结构
   * 树结构存在的目的在于
   * 1. 维持邻居的信息以确保不出现龟裂的情况
   *
   */
  struct BiTriTreeNode {
    BiTriTreeNode* left_child;
    BiTriTreeNode* right_child;
    BiTriTreeNode* base_neighbor;
    BiTriTreeNode* left_neighbor;
    BiTriTreeNode* right_neighbor;

#ifdef _DEBUG
    Triangle triangle;
#endif

    BiTriTreeNode()
        : left_child(NULL)
        , right_child(NULL)
        , base_neighbor(NULL)
        , left_neighbor(NULL)
        , right_neighbor(NULL) {
    }
  };


  void Init();
  void tessellate();
  int32* indices(int32* indices);
  void reset();
 private:
  void split_triangle(const Triangle& tri, Triangle* l, Triangle* r);

  // 跟据 x, y 获得 vertices 的 index
  int32 get_index(int x, int y);
  /**
   * 递归的产生 indices
   */
  int32* indices(BiTriTreeNode* node, const Triangle& triangle, int32* indices);

  /**
   * 递归的分割节点
   */
  void RecursSplit(BiTriTreeNode* node, const Triangle& triangle);
  void SplitNode(BiTriTreeNode* node, const Triangle& tri);

  BiTriTreeNode* allocate();
  void CalcVariance();
  uint8 RecursCalcVariable(const Triangle& triangle, uint8* vararr);

  /**
   * 使用数组来表示这个树结构
   */
  class Arena {
   public:
    Arena() : vec_index_(0), node_num_(0) {}
    BiTriTreeNode* allocate();
    void reset();
   private:
    typedef std::vector<BiTriTreeNode> BiTriTreeNodeVec;
    typedef std::vector<BiTriTreeNodeVec*> NodeBlockList;
    int vec_index_;
    int node_num_;
    NodeBlockList block_;
    static const int kBlockSize = 20480;
    DISALLOW_COPY_AND_ASSIGN(Arena);
  };

  void set_variance(int x, int y, uint8 var);
  uint8 variance(int x, int y);
  Arena arena_;
  azer::Tile* tile_;
  const azer::Tile::Pitch pitch_;
  BiTriTreeNode *left_root_;
  BiTriTreeNode *right_root_;
  std::unique_ptr<uint8[]> variance_;
  int node_num_;
  const int kMinWidth;
  DISALLOW_COPY_AND_ASSIGN(ROAMTree);
};

inline void ROAMTree::reset() {
  left_root_ = NULL;
  right_root_ = NULL;
  arena_.reset();
}

inline int32 ROAMTree::get_index(int x, int y) {
  int index = y * tile_->GetGridLineNum() + x;
  return index;
}
