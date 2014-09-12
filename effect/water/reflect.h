#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "tersbox/effect/common/tex_render.h"
#include "base/files/file_path.h"
#include "base/base.h"

class Reflect {
 public:
  Reflect(const azer::Plane& plane) : plane_(plane) {}

  void Init(azer::RenderSystem* rs);
  void BeginRender();
  void Reset(azer::Renderer* renderer);

  void OnUpdateCamera(const azer::Camera& camera);

  const azer::Matrix4& GetReflectView() const { return reflect_view_;}
  const azer::Matrix4& GetMirror() const { return mirror_;}
  const azer::Matrix4& GetMirrorPV() const { return pv_;}

  azer::TexturePtr GetReflectTex() {
    return target_.GetRTTex();
  }

  azer::Renderer* GetRenderer() { return target_.GetRenderer();}
 private:
  azer::Matrix4 reflect_view_;
  azer::Matrix4 mirror_;
  azer::Matrix4 pv_;
  azer::Plane plane_;
  TexRender target_;
  DISALLOW_COPY_AND_ASSIGN(Reflect);
};

inline void Reflect::Init(azer::RenderSystem* rs) {
  mirror_ = std::move(azer::MirrorTrans(plane_));
  target_.Init(rs);
}

inline void Reflect::OnUpdateCamera(const azer::Camera& camera) {
  const azer::Matrix4& proj = camera.frustrum().projection();
  reflect_view_ = std::move(mirror_ * camera.GetViewMatrix());
  pv_ = std::move(proj * reflect_view_);
}

inline void Reflect::BeginRender() {
  target_.BeginRender();
}

inline void Reflect::Reset(azer::Renderer* renderer) {
  target_.Reset(renderer);
}
