#pragma once

#include <memory>
#include "base/basictypes.h"
#include "azer/render/render.h"

class FrustrumFrame {
 public:
  FrustrumFrame() {}
  void Init(azer::RenderSystem* rs);
  void Update(const azer::Camera& camera);
  void Render(const azer::Matrix4& pv, azer::Renderer* renderer);
 private:
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<azer::Effect> effect_;
  azer::Matrix4 inverse_mat_;
  DISALLOW_COPY_AND_ASSIGN(FrustrumFrame);
};
