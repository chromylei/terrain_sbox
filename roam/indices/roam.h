#pragma once

#include "azer/util/tile.h"

/**
 * ROAMPitch 分割 Pitch 并生成 indices
 * 潜在的问题是，由于直接生成了 indices 而 normal 没有重新计算，因此所有的 normal 
 * 都是错误的。
 * 这个问题的重要程度需要评估，因为 error metric 会保证近处的 pitch 会被尽量分割
 * 因此实际上这个错误是很小的，对比两种实现方式
 * 如果使用 normalmap 这个问题就没有了
 */
class ROAMPitch {
 public:
  ROAMPitch(azer::Tile* tile);

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
 private:
  void reset();

  // 跟据 x, y 获得 vertices 的 index
  int32 get_index(int x, int y);
  /**
   * 递归的产生 indices
   */
  int32* indices(int node_index, int leftx, int lefty, int rightx, int righty,
                 int apexx, int apexy, int32* indices);
  void split();
  bool has_child(int index) const;
  
  std::unique_ptr<BiTriTreeNode[]> nodes_;
  azer::Tile* tile_;
  int node_num_;
  const int kMaxNodeNum;
  DISALLOW_COPY_AND_ASSIGN(ROAMPitch);
};

inline void ROAMPitch::reset() {
  node_num_ = 0;
  memset(nodes_.get(), 0 , kMaxNodeNum * sizeof(BiTriTreeNode));
}

inline bool ROAMPitch::has_child(int index) const {
  return nodes_.get()[index].haschild != 0;
}

inline int32 ROAMPitch::get_index(int x, int y) {
  int index = y * tile_->GetGridLineNum() + x;
  return index;
}
