#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/files/file_path.h"
#include "base/base.h"
#include "water.afx.h"

class Reflect;

class Water {
 public:
  Water() {}
  void OnInit(azer::RenderSystem* rs);
  void OnDraw(const azer::Camera& camera, Reflect* reflect,
              azer::Renderer* renderer);  
 private:
  azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                 azer::RenderSystem* rs);
  void InitRenderTarget(azer::RenderSystem* rs);
  void RenderReflect(const azer::Camera& camera); 

  azer::VertexBufferPtr vb_;
  azer::TexturePtr blue_tex_;
  azer::TexturePtr bumpmap01_;
  std::unique_ptr<WaterEffect> effect_;

  WaterEffect::DirLight light_;
  DISALLOW_COPY_AND_ASSIGN(Water);
};
