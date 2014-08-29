#pragma once

#include "azer/util/tile.h"

/**
 * ROAMPitch �ָ� Pitch ������ indices
 * Ǳ�ڵ������ǣ�����ֱ�������� indices �� normal û�����¼��㣬������е� normal 
 * ���Ǵ���ġ�
 * ����������Ҫ�̶���Ҫ��������Ϊ error metric �ᱣ֤������ pitch �ᱻ�����ָ�
 * ���ʵ������������Ǻ�С�ģ��Ա�����ʵ�ַ�ʽ
 * ���ʹ�� normalmap ��������û����
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

  // ���� x, y ��� vertices �� index
  int32 get_index(int x, int y);
  /**
   * �ݹ�Ĳ��� indices
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
