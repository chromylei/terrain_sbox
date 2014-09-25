#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/util/util.h"
#include "azer/render/util/tex_render_target.h"

#include "water.afx.h"

class Water {
 public:
  Water() : tile_(4, 8.0f) {}

  bool Init(const azer::Vector3& pos, azer::RenderSystem* rs);
  void Render(double time, const azer::Camera& camera, azer::Renderer* renderer);

  void SetDirLight(const DirLight& light) { light_ = light;}

  azer::Renderer* BeginDrawRefract();
 private:
  std::unique_ptr<azer::TexRenderTarget> refract_target_;
  std::unique_ptr<azer::TexRenderTarget> reflect_target_;
  azer::Vector3 position_;
  azer::Tile tile_;
  DirLight light_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::TexturePtr bump_tex_;
  std::unique_ptr<WaterEffect> effect_;
  DISALLOW_COPY_AND_ASSIGN(Water);
};
