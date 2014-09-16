#include "tersbox/tertur/bitmapclouds/skyplane.h"

#include "azer/util/util.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/bitmapclouds/"
#define SHADER_NAME "skyplane.afx"
#define CLOUNDS_PATH FILE_PATH_LITERAL("tersbox\\tertur\\media\\cloud001.dds")

const float kTexRepeat = 10.0f;
const float kSkyPlaneTop = 100.0f;
const float kSkyPlaneBottom = 0.0f;

bool SkyPlane::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SkyPlaneEffect(shaders.GetShaderVec(), rs));

  tile_.Init();
  azer::VertexData data(effect_->GetVertexDesc(), tile_.vertices().size());
  SkyPlaneEffect::Vertex* v = (SkyPlaneEffect::Vertex*)data.pointer();

  const float kSkyResolution = tile_.GetGridLineNum();
  const float fTexDelta = kTexRepeat / kSkyResolution;
  const float radius = tile_.x_range() * 0.5f;
  float constant = (kSkyPlaneTop - kSkyPlaneBottom) / (radius * radius);
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      float tu = j * fTexDelta;
      float tv = i * fTexDelta;
      azer::Vector3 pos = tile_.vertex(j, i);
      pos.y = kSkyPlaneTop - (constant * ((pos.x * pos.x) + (pos.z * pos.z)));
      v->position = pos;
      v->texcoord = azer::Vector2(tu, tv);
      v++;
    }
  }

  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));

  tex_.reset(azer::CreateShaderTexture(CLOUNDS_PATH, rs));
  return true;
}

void SkyPlane::Render(const azer::Camera& camera, azer::Renderer* renderer) {
  azer::Matrix4 world = std::move(azer::Translate(camera.position()));
  azer::CullingMode cull = renderer->GetCullingMode();
  renderer->SetCullingMode(azer::kCullNone);

  effect_->SetCloud1(tex_);
  effect_->SetApexColor(azer::Vector4(0.0f, 0.15f, 0.66f, 1.0f));
  effect_->SetCenterColor(azer::Vector4(0.81f, 0.38f, 0.66f, 1.0f));
  effect_->SetPVW(std::move(camera.GetProjViewMatrix() * world));
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);

  renderer->SetCullingMode(cull);
}
