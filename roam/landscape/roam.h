#pragma once

#include <memory>
#include "azer/util/tile.h"
#include "base/basictypes.h"

/**
 * ROAMPitch 分割 三角形 并生成 indices
 * 潜在的问题是，由于直接生成了 indices 而 normal 没有重新计算，因此所有的 normal 
 * 都是错误的。
 * 这个问题的重要程度需要评估，因为 error metric 会保证近处的 pitch 会被尽量分割
 * 因此实际上这个错误是很小的，对比两种实现方式
 * 如果使用 normalmap 这个问题就没有了
 *
 */

class ROAMPitch {
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

  /**
   * 如果单纯的使用 ROAM 算法进行分割是非常简单的，甚至不需要位置树结构
   * 树结构存在的目的在于
   * 1. 维持邻居的信息以确保不出现龟裂的情况
   *
   */
  class Arena;
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

  /**
   * 使用数组来表示这个树结构
   */
  class Arena {
   public:
    Arena() : vec_index_(0), node_num_(0) {}
    ~Arena();
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
 public:
  ROAMPitch(azer::Tile* tile, azer::Tile::Pitch& pitch, const int minlevel);


  void Init();
  void tessellate(const azer::Camera& camera);
  int32* indices(int32* indices);
  void reset();

  const azer::Tile::Pitch& pitch() const { return pitch_;}

  /**
   * left, right, top, bottom neighbor 描述的是一个 ROAMPitch 二维数组
   * 当中， ROAMPitch 的相互关系， 与 ROAM 的
   * left_neighbor, right_neighbor, base_neighbor 描述的是不同类的东西
   *
   * 注意：其中的 SetRightNeighbor 和 SetBottomNeighbor 不会使用
   * 原因与二维数组通常的遍历顺序相关
   */
  
  void SetLeftNeighbor(ROAMPitch* pitch);
  // void SetRightNeighbor(ROAMPitch* pitch);
  void SetTopNeighbor(ROAMPitch* pitch);
  // void SetBottomNeighbor(ROAMPitch* pitch);

  bool IsVisible(const azer::Camera& camera);
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
  void RecursSplit(BiTriTreeNode* node, const Triangle& triangle,
                   const azer::Camera& camera);
#ifdef _DEBUG
  void SplitNode(BiTriTreeNode* node, const Triangle& tri);
#else
  void SplitNode(BiTriTreeNode* node);
#endif

  BiTriTreeNode* allocate();
  void CalcVariance();
  uint8 RecursCalcVariable(const Triangle& triangle, uint8* vararr);
  bool IsTriangleVisible(const azer::Camera& camera, const Triangle& tri);

  azer::AxisAlignedBox CalcTriAABB(const Triangle& triangle);

  void set_variance(int x, int y, uint8 var);
  uint8 variance(int x, int y);
  Arena arena_;
  azer::Tile* tile_;
  const azer::Tile::Pitch pitch_;
  BiTriTreeNode left_root_;
  BiTriTreeNode right_root_;
  std::unique_ptr<uint8[]> variance_;
  const int kMinWidth;
  DISALLOW_COPY_AND_ASSIGN(ROAMPitch);
};

typedef std::shared_ptr<ROAMPitch> ROAMPitchPtr;

inline void ROAMPitch::reset() {
  arena_.reset();
}

inline int32 ROAMPitch::get_index(int x, int y) {
  int index = y * tile_->GetGridLineNum() + x;
  return index;
}

inline void ROAMPitch::SetLeftNeighbor(ROAMPitch* pitch) {
  left_root_.left_neighbor = &pitch->right_root_;
  pitch->right_root_.left_neighbor = &left_root_;
}

inline void ROAMPitch::SetTopNeighbor(ROAMPitch* pitch) {
  left_root_.right_neighbor = &pitch->right_root_;
  pitch->right_root_.right_neighbor = &left_root_;
}
