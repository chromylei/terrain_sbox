#pragma once

#include "azer/render/render.h"
#include "base/basictypes.h"

#include "skyplane.afx.h"

class SkyPlane {
 public:
  SkyPlane() {}
  bool Init(azer::RenderSystem* rs);
  void Render(const azer::Camera& camera, azer::Renderer* renderer);
 private:
  azer::VertexBufferPtr vb_;
  std::unique_ptr<SkyPlaneEffect> effect_;
  DISALLOW_COPY_AND_ASSIGN(SkyPlane);
};
