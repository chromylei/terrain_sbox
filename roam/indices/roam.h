#pragma once

#include "azer/util/tile.h"

/**
 * ROAMTree �ָ� ������ ������ indices
 * Ǳ�ڵ������ǣ�����ֱ�������� indices �� normal û�����¼��㣬������е� normal 
 * ���Ǵ���ġ�
 * ����������Ҫ�̶���Ҫ��������Ϊ error metric �ᱣ֤������ pitch �ᱻ�����ָ�
 * ���ʵ������������Ǻ�С�ģ��Ա�����ʵ�ַ�ʽ
 * ���ʹ�� normalmap ��������û����
 */
class ROAMTree {
 public:
  ROAMTree(azer::Tile* tile, azer::Tile::Pitch& patch);

  /**
   * ���������ʹ�� ROAM �㷨���зָ��Ƿǳ��򵥵ģ���������Ҫλ�����ṹ
   * ���ṹ���ڵ�Ŀ������
   * 1. ά���ھӵ���Ϣ��ȷ�������ֹ��ѵ����
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

  // ���� x, y ��� vertices �� index
  int32 get_index(int x, int y);
  /**
   * �ݹ�Ĳ��� indices
   */
  int32* indices(int node_index, int leftx, int lefty, int rightx, int righty,
                 int apexx, int apexy, int32* indices);

  /**
   * �ݹ�ķָ�ڵ�
   */
  void RecursSplit(int nodeindex, int leftx, int lefty,
                   int rightx, int righty, int apexx, int apexy);
  void SplitNode(int nodeindex);
  bool has_child(int index) const;

  /**
   * ʹ����������ʾ������ṹ
   * ���� ROAMTree ʵ���Ϸָ���һ�������Σ���������������ڵ� 1�� 2
   * �ڵ� 0 û������
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
