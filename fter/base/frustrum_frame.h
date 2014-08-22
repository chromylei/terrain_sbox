#pragma once

#include <memory>
#include "base/basictypes.h"
#include "azer/render/render.h"

class FrustrumFrame {
 public:
  FrustrumFrame(const azer::Camera& camera)
      : camera_(camera) {
  }

  void Init(azer::RenderSystem* rs);
  void Update(azer::RenderSystem* rs);
  void Render(azer::Matrix4& pv, azer::Renderer* renderer);
 private:
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  const azer::Camera& camera_;
  std::unique_ptr<azer::Effect> effect_;
  azer::Matrix4 inverse_mat_;
  DISALLOW_COPY_AND_ASSIGN(FrustrumFrame);
};
