#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/base/camera_control.h"

#include "skydomes.afx.h"

class SkyDomes {
 public:
  SkyDomes() {}

  bool Init(azer::RenderSystem* rs);
  void Render(const azer::Camera& camera, azer::Renderer* renderer);
 private:
  azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                 azer::RenderSystem* rs);
  
  std::unique_ptr<SkyDomesEffect> effect_;
  azer::TexturePtr tex_;
  azer::VertexBufferPtr sphere_;
  DISALLOW_COPY_AND_ASSIGN(SkyDomes);
};
