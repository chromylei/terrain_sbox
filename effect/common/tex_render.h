#pragma once

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/files/file_path.h"
#include "base/base.h"

class TexRender {
 public:
  TexRender() {}

  void Init(azer::RenderSystem* rs);
  void BeginRender();
  void Reset(azer::Renderer* renderer);

  azer::TexturePtr GetRTTex() {
    return reflect_->GetRenderTarget()->GetTexture();
  }

  azer::Renderer* GetRenderer() { return reflect_.get();}
 private:
  std::unique_ptr<azer::Renderer> reflect_;
  DISALLOW_COPY_AND_ASSIGN(TexRender);
};

inline void TexRender::Init(azer::RenderSystem* rs) {
  azer::Texture::Options opt;
  opt.width = 800;
  opt.height = 600;
  opt.format = azer::kRGBAf;
  opt.target = (azer::Texture::BindTarget)
      (azer::Texture::kRenderTarget | azer::Texture::kShaderResource);
  reflect_.reset(rs->CreateRenderer(opt));
}

inline void TexRender::BeginRender() {
  reflect_->Use();
  reflect_->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  reflect_->Clear(azer::Vector4(0.0f, 0.0f, 0.8f, 1.0f));
  reflect_->ClearDepthAndStencil();
  reflect_->SetCullingMode(azer::kCullNone);
  reflect_->EnableDepthTest(true);
}

inline void TexRender::Reset(azer::Renderer* renderer) {
  renderer->Use();
}
