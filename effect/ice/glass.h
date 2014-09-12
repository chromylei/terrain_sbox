#pragma once

#include "glass.afx.h"

class Glass {
 public:
  Glass() {}

  void OnInit(azer::RenderSystem* rs);
  void OnDraw(const azer::Camera& camera, azer::TexturePtr refract,
              azer::Renderer* renderer);
  void OnUpdate(double time, float delta_time);
 private:
  void InitVertex(const ::base::FilePath& path, azer::RenderSystem* rs);
  azer::VertexBufferPtr vb_;
  azer::TexturePtr tex_;
  azer::TexturePtr perturb_tex_;
  azer::TexturePtr bumpmap_tex_;
  azer::Matrix4 world_;
  std::unique_ptr<GlassEffect> effect_;
  GlassEffect::DirLight light_;
  DISALLOW_COPY_AND_ASSIGN(Glass);
};
