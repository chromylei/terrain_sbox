#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/util/util.h"
#include "azer/render/util/tex_render_target.h"

#include "water.afx.h"

class Water {
 public:
  Water() : tile_(4, 1.0f) {}

  virtual void DrawRefraction(azer::Renderer* renderer) = 0;
  virtual void DrawReflection(azer::Renderer* renderer) = 0;

  bool Init(azer::RenderSystem* rs);
  void Render(const azer::Camera& camera, azer::Renderer* renderer);

  void SetPosition(const azer::Vector3& position);
 private:
  std::unique_ptr<azer::TexRenderTarget> refract_target_;
  std::unique_ptr<azer::TexRenderTarget> reflect_target_;
  azer::Vector3 position_;
  azer::Tile tile_;
  DirLight light_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<WaterEffect> effect_;
  DISALLOW_COPY_AND_ASSIGN(Water);
};
