#pragma once

#include <memory>
#include "azer/util/tile.h"
#include "base/basictypes.h"

/**
 * ROAMPitch �ָ� ������ ������ indices
 * Ǳ�ڵ������ǣ�����ֱ�������� indices �� normal û�����¼��㣬������е� normal 
 * ���Ǵ���ġ�
 * ����������Ҫ�̶���Ҫ��������Ϊ error metric �ᱣ֤������ pitch �ᱻ�����ָ�
 * ���ʵ������������Ǻ�С�ģ��Ա�����ʵ�ַ�ʽ
 * ���ʹ�� normalmap ��������û����
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
 public:
  ROAMPitch(azer::Tile* tile, azer::Tile::Pitch& pitch, const int minlevel);
  
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
  void tessellate(const azer::Camera& camera);
  int32* indices(int32* indices);
  void reset();

  const azer::Tile::Pitch& pitch() const { return pitch_;}

  /**
   * left, right, top, bottom neighbor ��������һ�� ROAMPitch ��ά����
   * ���У� ROAMPitch ���໥��ϵ�� �� ROAM ��
   * left_neighbor, right_neighbor, base_neighbor �������ǲ�ͬ��Ķ���
   *
   * ע�⣺���е� SetRightNeighbor �� SetBottomNeighbor ����ʹ��
   * ԭ�����ά����ͨ���ı���˳�����
   */
  
  void SetLeftNeighbor(ROAMPitch* pitch);
  // void SetRightNeighbor(ROAMPitch* pitch);
  void SetTopNeighbor(ROAMPitch* pitch);
  // void SetBottomNeighbor(ROAMPitch* pitch);
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
  BiTriTreeNode left_root_;
  BiTriTreeNode right_root_;
  std::unique_ptr<uint8[]> variance_;
  const int kMinWidth;
  DISALLOW_COPY_AND_ASSIGN(ROAMPitch);
};

typedef std::shared_ptr<ROAMPitch> ROAMPitchPtr;

inline void ROAMPitch::reset() {
  left_root_.left_neighbor = NULL;
  left_root_.right_neighbor = NULL;
  right_root_.left_neighbor = NULL;
  right_root_.right_neighbor = NULL;
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