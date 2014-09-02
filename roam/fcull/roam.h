#pragma once

#include <memory>
#include "azer/util/tile.h"
#include "base/basictypes.h"

/**
 * ROAMTree �ָ� ������ ������ indices
 * Ǳ�ڵ������ǣ�����ֱ�������� indices �� normal û�����¼��㣬������е� normal 
 * ���Ǵ���ġ�
 * ����������Ҫ�̶���Ҫ��������Ϊ error metric �ᱣ֤������ pitch �ᱻ�����ָ�
 * ���ʵ������������Ǻ�С�ģ��Ա�����ʵ�ַ�ʽ
 * ���ʹ�� normalmap ��������û����
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
   * ���������ʹ�� ROAM �㷨���зָ��Ƿǳ��򵥵ģ���������Ҫλ�����ṹ
   * ���ṹ���ڵ�Ŀ������
   * 1. ά���ھӵ���Ϣ��ȷ�������ֹ��ѵ����
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

  // ���� x, y ��� vertices �� index
  int32 get_index(int x, int y);
  /**
   * �ݹ�Ĳ��� indices
   */
  int32* indices(BiTriTreeNode* node, const Triangle& triangle, int32* indices);

  /**
   * �ݹ�ķָ�ڵ�
   */
  void RecursSplit(BiTriTreeNode* node, const Triangle& triangle);
  void SplitNode(BiTriTreeNode* node, const Triangle& tri);

  BiTriTreeNode* allocate();
  void CalcVariance();
  uint8 RecursCalcVariable(const Triangle& triangle, uint8* vararr);

  /**
   * ʹ����������ʾ������ṹ
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
