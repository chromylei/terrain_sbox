#pragma once

#include "base/basictypes.h"
#include "azer/util/tile.h"

class Clod {
 public:
  Clod(azer::Tile* tile) : tile_(tile) {}

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
  static int32* InitPitchFan(const azer::Tile::Pitch& pitch, int kGridLine,
                             int32* indices, uint32 flags = kSplitAll);

  /**
   * generate indices for pitch
   */
  int32* GenIndices(std::vector<azer::Tile::Pitch>* pitch,
                    int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(int32* indices, uint32 flags = kSplitAll);
  int32* GenIndices(azer::Tile::Pitch& pitch, int32* indices,
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
  int32* GenIndices(const azer::Tile::Pitch& pitch,
                    int32* indices, int32* splitlevel);
  void CalcLOD();
  int level(int x, int y, int32* level);
 private:
  int left_level(int x, int y, int step, int32* level);
  int right_level(int x, int y, int step, int32* level);
  int top_level(int x, int y, int step, int32* level);
  int bottom_level(int x, int y, int step, int32* level);
  azer::Tile* tile_;
  std::unique_ptr<int32> levels_;
  DISALLOW_COPY_AND_ASSIGN(Clod);
};


inline int Clod::left_level(int x, int y, int step, int32* level) {
  int newx = (x < step ? 0 : x - step);
  return level[y * tile_->GetGridLineNum() + newx];
}

inline int Clod::right_level(int x, int y, int step, int32* level) {
  int newx = ((x < tile_->GetGridLineNum() - step) ? x + step : x);
  return level[y * tile_->GetGridLineNum() + newx];
}

inline int Clod::top_level(int x, int y, int step, int32* level) {
  int newy = (y < step ? 0 : y - step);
  return level[newy * tile_->GetGridLineNum() + x];
}

inline int Clod::bottom_level(int x, int y, int step, int32* level) {
  int newy = (y < tile_->GetGridLineNum() + step ? y + step : y);
  return level[newy * tile_->GetGridLineNum() + x];
}
