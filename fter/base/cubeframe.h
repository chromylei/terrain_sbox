#pragma once

#include <memory>
#include "base/basictypes.h"
#include "azer/render/render.h"

class CubeFrame {
 public:
  CubeFrame() {}
  void Init(azer::RenderSystem* rs);
  void Render(azer::Vector3& pos1, azer::Vector3& pos2,
              azer::Renderer* renderer, const azer::Matrix4& pv);
 private:
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(CubeFrame);
};
