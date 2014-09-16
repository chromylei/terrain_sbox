#include "tersbox/effect/softshadow/shadowmap.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/softshadow/"
#define SHADER_NAME "shadowmap.afx"
#define SHADER_NAME2 "hard_shadowmap.afx"

void ShadowmapGraphic::Init(azer::RenderSystem* rs) {
  target_.Init(rs);
  hard_target_.Init(rs);

  {
    azer::ShaderArray shaders;
    CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
    CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
    effect_.reset(new ShadowmapEffect(shaders.GetShaderVec(), rs));
  }

  {
    azer::ShaderArray shaders;
    CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME2 ".vs", &shaders));
    CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME2 ".ps", &shaders));
    hard_effect_.reset(new HardShadowmapEffect(shaders.GetShaderVec(), rs));
  }
}

azer::Renderer* ShadowmapGraphic::Begin(azer::Renderer* renderer) {
  prev_ = renderer;
  target_.BeginRender();
  azer::Renderer* r = target_.GetRenderer();
  r->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  r->ClearDepthAndStencil();
  return r;
}

void ShadowmapGraphic::End() {
  DCHECK(prev_ != NULL);
  prev_->Use();
}

azer::Renderer* ShadowmapGraphic::BeginHard(azer::Renderer* renderer) {
  prev_ = renderer;
  hard_target_.BeginRender();
  azer::Renderer* r = hard_target_.GetRenderer();
  r->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  r->ClearDepthAndStencil();
  return r;
}

void ShadowmapGraphic::EndHard() {
  DCHECK(prev_ != NULL);
  prev_->Use();
}

