#include "tersbox/effect/multilight_shadowmap/shadowmap.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/multilight_shadowmap/"
#define SHADER_NAME "shadowmap.afx"

void ShadowmapGraphic::Init(azer::RenderSystem* rs) {
  target_.Init(rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new ShadowmapEffect(shaders.GetShaderVec(), rs));
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

