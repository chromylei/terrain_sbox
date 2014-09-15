#pragma once

#include "azer/render/render.h"
#include "base/basictypes.h"
#include "tersbox/effect/common/tex_render.h"

#include "shadowmap.afx.h"


class ShadowmapGraphic {
 public:
  ShadowmapGraphic() {}
  void Init(azer::RenderSystem* rs);
  azer::Renderer* Begin(azer::Renderer* renderer);
  void End();

  const azer::Camera& GetLightCamera() const { return light_view_;}
  azer::Camera& GetLightCamera() { return light_view_;}
  const azer::Matrix4& GetLightViewMat() { return light_view_.GetViewMatrix();}
  const azer::Matrix4& GetLightPVMat() { return light_view_.GetProjViewMatrix();}
  azer::TexturePtr GetShadowmap() { return target_.GetRTTex();}
  ShadowmapEffect* GetEffect()  { return effect_.get();}
 private:
  azer::Camera light_view_;
  TexRender target_;
  azer::Renderer* prev_;
  std::unique_ptr<ShadowmapEffect> effect_;
  DISALLOW_COPY_AND_ASSIGN(ShadowmapGraphic);
};
