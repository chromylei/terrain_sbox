#pragma once

#include "azer/render/render.h"
#include "base/basictypes.h"
#include "azer/util/util.h"

#include "skyplane.afx.h"

class SkyPlane {
 public:
  SkyPlane() : tile_(9, 8.0f) {}
  bool Init(azer::RenderSystem* rs);
  void Render(const azer::Camera& camera, azer::Renderer* renderer);
 private:
  azer::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<SkyPlaneEffect> effect_;
  azer::TexturePtr tex_;

  static const float kSkyPlaneWidth;
  static const float kSkyPlaneResolution;
  DISALLOW_COPY_AND_ASSIGN(SkyPlane);
};
