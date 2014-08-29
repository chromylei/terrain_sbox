#pragma once

#include "azer/util/tile.h"

/**
 * ROAMTree 分割 三角形 并生成 indices
 * 潜在的问题是，由于直接生成了 indices 而 normal 没有重新计算，因此所有的 normal 
 * 都是错误的。
 * 这个问题的重要程度需要评估，因为 error metric 会保证近处的 pitch 会被尽量分割
 * 因此实际上这个错误是很小的，对比两种实现方式
 * 如果使用 normalmap 这个问题就没有了
 */
class ROAMTree {
 public:
  ROAMTree(azer::Tile* tile, azer::Tile::Pitch& patch);

  /**
   * 如果单纯的使用 ROAM 算法进行分割是非常简单的，甚至不需要位置树结构
   * 树结构存在的目的在于
   * 1. 维持邻居的信息以确保不出现龟裂的情况
   *
   */
  struct BiTriTreeNode {
    int base_neighbor;
    int left_neighbor;
    int right_neighbor;
    int haschild;

    BiTriTreeNode()
        : base_neighbor(0)
        , left_neighbor(0)
        , right_neighbor(0)
        , haschild(0) {
    }
  };

  void tessellate();

  bool has_left_neighbor(BiTriTreeNode* node) { return node->left_neighbor != 0;}
  bool has_right_neighbor(BiTriTreeNode* node) { return node->right_neighbor != 0;}
  bool has_base_neighbor(BiTriTreeNode* node) { return node->base_neighbor != 0;}

  int32* indices(int32* indices);
 private:
  void reset();

  // 跟据 x, y 获得 vertices 的 index
  int32 get_index(int x, int y);
  /**
   * 递归的产生 indices
   */
  int32* indices(int node_index, int leftx, int lefty, int rightx, int righty,
                 int apexx, int apexy, int32* indices);

  /**
   * 递归的分割节点
   */
  void RecursSplit(int nodeindex, int leftx, int lefty,
                   int rightx, int righty, int apexx, int apexy);
  void SplitNode(int nodeindex);
  bool has_child(int index) const;

  /**
   * 使用数组来表示这个树结构
   * 由于 ROAMTree 实际上分割了一个正方形，因此它有两个根节点 1， 2
   * 节点 0 没有作用
   */
  std::unique_ptr<BiTriTreeNode[]> nodes_;
  const azer::Tile::Pitch pitch_;
  azer::Tile* tile_;
  int node_num_;
  const int kMaxNodeNum;
  DISALLOW_COPY_AND_ASSIGN(ROAMTree);
};

inline void ROAMTree::reset() {
  node_num_ = 0;
  memset(nodes_.get(), 0 , kMaxNodeNum * sizeof(BiTriTreeNode));
}

inline bool ROAMTree::has_child(int index) const {
  return nodes_.get()[index].haschild != 0;
}

inline int32 ROAMTree::get_index(int x, int y) {
  int index = y * tile_->GetGridLineNum() + x;
  return index;
}
