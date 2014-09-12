#include "tersbox/effect/blur/blur.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/blur/"
#define SHADER_NAME "blur.afx"

void BlurGraphics::Init(azer::RenderSystem* rs) {
  targets_.resize(10);
  for (size_t i = 0; i < targets_.size(); ++i) {
    targets_[i].reset(new azer::TexRenderTarget(400, 300));
    targets_[i]->Init(rs);
  }

  finaly_.reset(new azer::TexRenderTarget(800, 600));
  finaly_->Init(rs);
  finaly_->GetRenderer()->EnableDepthTest(true);
  finaly_->GetRenderer()->SetCullingMode(azer::kCullNone);

  overlay_.reset(rs->CreateOverlay(gfx::RectF(-1.0f, -1.0f, 2.0f, 2.0f)));

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new BlurEffect(shaders.GetShaderVec(), rs));
  overlay_->SetEffect(effect_);
}

void BlurGraphics::Render() {
  int cnt = cnt_;
  azer::Renderer* renderer = finaly_->Begin(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  BlurEffect* effect = (BlurEffect*)effect_.get();
  // effect->SetTexture(targets_[cnt % targets_.size()]->GetRTTex());
  effect->SetBlurTexture1(targets_[cnt % targets_.size()]->GetRTTex());
  effect->SetBlurTexture2(targets_[(cnt + 1) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture3(targets_[(cnt + 2) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture4(targets_[(cnt + 3) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture5(targets_[(cnt + 4) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture6(targets_[(cnt + 5) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture7(targets_[(cnt + 6) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture8(targets_[(cnt + 7) % targets_.size()]->GetRTTex());
  effect->SetBlurTexture9(targets_[(cnt + 8) % targets_.size()]->GetRTTex());
  overlay_->Render(renderer);
}

azer::Renderer* BlurGraphics::Begin() {
  int cnt = cnt_;
  azer::TexRenderTargetPtr& ptr = targets_[cnt];
  ptr->Begin(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  return ptr->GetRenderer();
}

void BlurGraphics::End() {
  next();
}
