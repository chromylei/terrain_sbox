#pragma once

#include "azer/util/tile.h"

BiTriTreeNode {
  BiTriTreeNode* left;
  BiTriTreeNode* right;
  BiTriTreeNode* base_neighbor;
  BiTriTreeNode* left_neighbor;
  BiTriTreeNode* right_neighbor;
};

class TriTree {
 public:
  bool Create(const azer::util::Tile::Pitch& pitch);
 private:
};
