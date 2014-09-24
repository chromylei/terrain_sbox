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
  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
           sizeof(int32) * tile_.indices().size());

  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata.pointer();
  DiffuseEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      v->position = tile_.vertices()[cnt];
      v->tex0 = tile_.texcoord()[cnt] * repeat_num;
      v->normal = tile_.normal()[cnt];
      cnt = 0;
    }
  }
}
