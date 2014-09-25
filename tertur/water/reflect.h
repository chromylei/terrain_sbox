#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/render/util/tex_render_target.h"
#include "tersbox/base/camera_control.h"
#include "base/files/file_path.h"
#include "base/base.h"

class Reflect {
 public:
  Reflect(FreeCamera* camera, const azer::Plane& plane)
      : plane_(plane)
      , camera_(camera, plane)
      , target_(800.0f, 600.0f) {
  }

  void Init(azer::RenderSystem* rs);
  azer::Renderer* Begin();
  void Reset(azer::Renderer* renderer);

  const azer::Camera& GetMirrorCamera() const { return camera_; }

  azer::TexturePtr GetReflectTex() {
    return target_.GetRTTex();
  }

  azer::Renderer* GetRenderer() { return target_.GetRenderer();}
 private:
  azer::Plane plane_;
  MirrorCamera camera_;
  azer::TexRenderTarget target_;
  DISALLOW_COPY_AND_ASSIGN(Reflect);
};

inline void Reflect::Init(azer::RenderSystem* rs) {
  target_.Init(rs);
}

inline azer::Renderer* Reflect::Begin() {
  return target_.Begin(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

inline void Reflect::Reset(azer::Renderer* renderer) {
  target_.Reset(renderer);
}
