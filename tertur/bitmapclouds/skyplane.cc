#include "tersbox/tertur/bitmapclouds/skyplane.h"

#include "azer/util/util.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/bitmapclouds/"
#define SHADER_NAME "skyplane.afx"

bool SkyPlane::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SkyPlaneEffect(shaders.GetShaderVec(), rs));
  return true;
}

void SkyPlane::Render(const azer::Camera& camera, azer::Renderer* renderer) {
}
