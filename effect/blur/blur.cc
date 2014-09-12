#include "tersbox/effect/blur/blur.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/blur/"
#define SHADER_NAME "blur.afx"

void BlurGrphics::Init(azer::RenderSystem* rs) {
  targets_.resize(10);
  for (size_t i = 0; i < targets_.size(); ++i) {
    targets_[i].reset(new azer::TexRenderTarget(400, 300));
    targets_[i]->Init(rs);
  }

  finaly_.reset(new azer::TexRenderTarget(800, 600));
  finaly_->Init(rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new BlurEffect(shaders.GetShaderVec(), rs));

  overlay_.reset(rs->CreateOverlay(gfx::RectF(-1.0f, -1.0f, 2.0f, 2.0f)));
}

void BlurGrphics::Render() {
  target_->Use();
  effect_->SetBlurTexture1(targets_[cnt % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture2(targets_[(cnt + 1) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture3(targets_[(cnt + 2) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture4(targets_[(cnt + 3) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture5(targets_[(cnt + 4) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture6(targets_[(cnt + 5) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture7(targets_[(cnt + 6) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture8(targets_[(cnt + 7) % targets_.size()]->GetRTTex());
  effect_->SetBlurTexture9(targets_[(cnt + 8) % targets_.size()]->GetRTTex());
  effect_->Use(renderer);
}
