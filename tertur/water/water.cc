#include "tersbox/tertur/water/water.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/water/"
#define SHADER_NAME "water.afx"
#define WATER_BUMPTEX FILE_PATH_LITERAL("tersbox/tertur/media/waternormal.dds")

bool Water::Init(const azer::Vector3& pos, azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new WaterEffect(shaders.GetShaderVec(), rs));

  float repeat_num = 32.0f;
  tile_.Init();

  int cnt = 0;
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  WaterEffect::Vertex* vertex = (WaterEffect::Vertex*)vdata.pointer();
  WaterEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      v->position = tile_.vertices()[cnt] + pos;
      v->tex0 = tile_.texcoord()[cnt];
      v->tex1 = tile_.texcoord()[cnt] * repeat_num;
      v->tex2 = tile_.texcoord()[cnt] * repeat_num * 2.0f;
      cnt++;
      v++;
    }
  }

  position_ = pos;

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));

  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));

  refract_target_.reset(new azer::TexRenderTarget(800, 600));
  refract_target_->Init(rs);

  bump_tex_.reset(azer::Texture::LoadShaderTexture(WATER_BUMPTEX, rs));
  return true;
}

azer::Renderer* Water::BeginDrawRefract() {
  return refract_target_->Begin(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Water::Render(double time, const azer::Camera& camera,
                   azer::Renderer* renderer) {
  renderer->Use();
  renderer->SetCullingMode(azer::kCullBack);
  azer::Matrix4 pvw = camera.GetProjViewMatrix();
  effect_->SetPVW(pvw);
  effect_->SetDirLight(light_);
  effect_->SetCameraPos(camera.position());
  effect_->SetRefractTex(refract_target_->GetRTTex());
  effect_->SetReflectTex(reflect_->GetReflectTex());
  effect_->SetReflectPVW(reflect_->GetMirrorCamera().GetProjViewMatrix());
  effect_->SetBumpTex(bump_tex_);
  effect_->SetTime((float)time * 0.1f);
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);
}
