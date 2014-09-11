#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
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
  const azer::Matrix4& GetMirroPV() const { return pv_;}

  azer::TexturePtr GetReflectTex() {
    return reflect_->GetRenderTarget()->GetTexture();
  }

  azer::Renderer* GetRenderer() { return reflect_.get();}
 private:
  azer::Matrix4 reflect_view_;
  azer::Matrix4 mirror_;
  azer::Matrix4 pv_;
  azer::Plane plane_;
  std::unique_ptr<azer::Renderer> reflect_;
  DISALLOW_COPY_AND_ASSIGN(Reflect);
};

inline void Reflect::Init(azer::RenderSystem* rs) {
  mirror_ = std::move(azer::MirrorTrans(plane_));

  azer::Texture::Options opt;
  opt.width = 800;
  opt.height = 600;
  opt.format = azer::kRGBAf;
  opt.target = (azer::Texture::BindTarget)
      (azer::Texture::kRenderTarget | azer::Texture::kShaderResource);
  reflect_.reset(rs->CreateRenderer(opt));
}

inline void Reflect::OnUpdateCamera(const azer::Camera& camera) {
  const azer::Matrix4& proj = camera.frustrum().projection();
  reflect_view_ = std::move(mirror_ * camera.GetViewMatrix());
  pv_ = std::move(proj * reflect_view_);
}

inline void Reflect::BeginRender() {
  reflect_->Use();
  reflect_->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  reflect_->Clear(azer::Vector4(0.0f, 0.0f, 0.8f, 1.0f));
  reflect_->ClearDepthAndStencil();
  reflect_->SetCullingMode(azer::kCullNone);
  reflect_->EnableDepthTest(true);
}

inline void Reflect::Reset(azer::Renderer* renderer) {
  renderer->Use();
}
