#include "tersbox/effect/shadowmap/shadowmap.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/shadowmap/"
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
  return target_.GetRenderer();
}

void ShadowmapGraphic::End() {
  DCHECK(prev_ != NULL);
  prev_->Use();
}

