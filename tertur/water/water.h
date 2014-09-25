#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/util/util.h"
#include "azer/render/util/tex_render_target.h"
#include "tersbox/tertur/water/reflect.h"
#include "water.afx.h"

class Water {
 public:
  Water() : tile_(4, 8.0f) {}

  bool Init(const azer::Vector3& pos, azer::RenderSystem* rs);
  void Render(double time, const azer::Camera& camera, azer::Renderer* renderer);

  void SetDirLight(const DirLight& light) { light_ = light;}

  azer::Renderer* BeginDrawRefract();
  void SetReflect(Reflect* reflect) { reflect_ = reflect; }
 private:
  std::unique_ptr<azer::TexRenderTarget> refract_target_;
  azer::Vector3 position_;
  azer::Tile tile_;
  DirLight light_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::TexturePtr bump_tex_;
  std::unique_ptr<WaterEffect> effect_;
  Reflect* reflect_;
  DISALLOW_COPY_AND_ASSIGN(Water);
};
