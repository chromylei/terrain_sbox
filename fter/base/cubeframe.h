#pragma once

#include <memory>
#include "base/basictypes.h"
#include "azer/render/render.h"

class CubeFrame {
 public:
  CubeFrame() : diffuse_(azer::Vector4(1.0f, 0.0f, 0.0f, 1.0f)) {}
  void Init(azer::RenderSystem* rs);
  void Render(azer::Vector3& pos1, azer::Vector3& pos2,
              azer::Renderer* renderer, const azer::Matrix4& pv);
  void SetDiffuse(const azer::Vector4& diffuse) {diffuse_ = diffuse;}
 private:
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<azer::Effect> effect_;
  azer::Vector4 diffuse_;
  DISALLOW_COPY_AND_ASSIGN(CubeFrame);
};
