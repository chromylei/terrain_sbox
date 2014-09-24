#include "tersbox/tertur/water/water.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/water/"
#define SHADER_NAME "water.afx"

bool Water::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new WaterEffect(shaders.GetShaderVec(), rs));

  tile_.Init();
  int cnt = 0;
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  WaterEffect::Vertex* vertex = (WaterEffect::Vertex*)vdata.pointer();
  WaterEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      v->position = tile_.vertices()[cnt];
      v->tex0 = tile_.texcoord()[cnt];
      cnt = 0;
    }
  }

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));

  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));

  refract_target_.reset(new azer::TexRenderTarget(800, 600));
  refract_target_->Init(rs);

  reflect_target_.reset(new azer::TexRenderTarget(800, 600));
  reflect_target_->Init(rs);
  return true;
}

void Water::Render(const azer::Camera& camera, azer::Renderer* renderer) {
  DrawRefraction(refract_target_->GetRenderer());
  DrawReflection(reflect_target_->GetRenderer());
  renderer->Use();

  azer::Matrix4 world = azer::Translate(position_);
  azer::Matrix4 pvw = std::move(camera.GetProjViewMatrix() * world);
  effect_->SetPVW(pvw);
  effect_->SetDirLight(light_);
  effect_->SetRefractTex(refract_target_->GetRTTex());
  PrepareEffect(effect_.get());
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);
}
