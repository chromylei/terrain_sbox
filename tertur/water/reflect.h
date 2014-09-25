#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/render/util/tex_render_target.h"
#include "base/files/file_path.h"
#include "base/base.h"

class Reflect {
 public:
  Reflect(const azer::Plane& plane)
      : plane_(plane)
      , target_(800.0f, 600.0f) {
  }

  void Init(azer::RenderSystem* rs);
  azer::Renderer* Begin();
  void Reset(azer::Renderer* renderer);

  void OnCamera(const azer::Camera& camera);

  const azer::Matrix4& GetReflectView() const { return reflect_view_;}
  const azer::Matrix4& GetMirror() const { return mirror_;}
  const azer::Matrix4& GetMirrorPV() const { return pv_;}

  const azer::Camera& GetMirrorCamera() const { return camera_; }

  azer::TexturePtr GetReflectTex() {
    return target_.GetRTTex();
  }

  azer::Renderer* GetRenderer() { return target_.GetRenderer();}
 private:
  azer::Matrix4 reflect_view_;
  azer::Matrix4 mirror_;
  azer::Matrix4 pv_;
  azer::Plane plane_;
  azer::Camera camera_;
  azer::TexRenderTarget target_;
  DISALLOW_COPY_AND_ASSIGN(Reflect);
};

inline void Reflect::Init(azer::RenderSystem* rs) {
  mirror_ = std::move(azer::MirrorTrans(plane_));
  target_.Init(rs);
}

inline void Reflect::OnCamera(const azer::Camera& camera) {
  const azer::Matrix4& proj = camera.frustrum().projection();
  reflect_view_ = std::move(mirror_ * camera.GetViewMatrix());
  pv_ = std::move(proj * reflect_view_);
  camera_ = std::move(camera.mirror(plane_));
}

inline azer::Renderer* Reflect::Begin() {
  return target_.Begin(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

inline void Reflect::Reset(azer::Renderer* renderer) {
  target_.Reset(renderer);
}
