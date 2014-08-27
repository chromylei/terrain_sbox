#pragma once

#include "base/basictypes.h"
#include "azer/util/tile.h"

class Clod {
 public:
  Clod(azer::util::Tile* tile) : tile_(tile) {}

  /**
   *  init indices for following geo
   *       *-----*-----*
   *       | \   |   / |
   *       |   \ | /   |
   *       *-----*-----*
   *       |   / | \   |
   *       | /   |   \ |
   *       *-----*-----*
   */

  enum PitchFanSplit {
    kSplitTop    = 0x00000001,
    kSplitBottom = 0x00000002,
    kSplitLeft   = 0x00000004,
    kSplitRight  = 0x00000008,
    kSplitOne    = 0x00000010,
    kSplitAll    = 0x0000000F,
  };
  static int32* InitPitchFan(const azer::util::Tile::Pitch& pitch, int kGridLine,
                             int32* indices, uint32 flags = kSplitAll);

  /**
   * generate indices for pitch
   */
  int32* GenIndices(std::vector<azer::util::Tile::Pitch>* pitch,
                    int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(azer::util::Tile::Pitch& pitch, int32* indices,
                    uint32 flags = kSplitAll);

  /**
   * ���ݸ����ķָ����зָ�
   * ע���ڷָ��ʱ����뱣֤�ָʽ�ܹ��к��������� tile, ���ܳ��ַָ��������
   * ���� �ڱ�Ե���� level == 2
   * ���� splitlevel ���㷨��ʵ�ܼ򵥾���
   *  level = 0 ������ 4 * 4 �ķ�ʽ����
   *  level = 1 ������ 2 * 2 �ķ�ʽ����
   *  level = 2 ������ν
   */
  int32* GenIndices(int32* indices, int32* splitlevel);
  int32* GenIndices(const azer::util::Tile::Pitch& pitch,
                    int32* indices, int32* splitlevel);
  void CalcLOD();
 private:
  azer::util::Tile* tile_;
  std::unique_ptr<int32> levels_;
  DISALLOW_COPY_AND_ASSIGN(Clod);
};


