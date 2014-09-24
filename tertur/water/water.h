#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/util/util.h"
#include "water.afx.h"

class Water {
 public:
  Water() : tile_(4, 1.0f) {}

  virtual void DrawRefraction(azer::Renderer* renderer);
  virtual void DrawReflection(azer::Renderer* renderer);
  void Init();

  void SetPosition(const azer::Vector3& position);
  void Render(azer::Renderer* renderer);
 private:
  azer::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<WaterEffect> effect_;
  DISALLOW_COPY_AND_ASSIGN(Water);
};
