#include "tersbox/fter/base/cubeframe.h"


#include "frame_effect.afx.h"

void CubeFrame::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader("out/dbg/gen/tersbox/fter/base/frame_effect.afx.vs",
                               &shader));
  CHECK(azer::LoadPixelShader("out/dbg/gen/tersbox/fter/base/frame_effect.afx.ps",
                              &shader));
  
  FrameEffect* effect = new FameEffect(shaders.GetShaderVec(), rs);
  effect_.reset(effect);

  FrustrumFrameEffect::Vertex vertices[] = {
    azer::Vector4(0.0f, 0.0f,  0.0f, 1.0f),
    azer::Vector4(1.0f, 0.0f,  0.0f, 1.0f),
    azer::Vector4(1.0f, 0.0f, -1.0f, 1.0f),
    azer::Vector4(0.0f, 0.0f, -1.0f, 1.0f),
    azer::Vector4(0.0f, 1.0f,  0.0f, 1.0f),
    azer::Vector4(1.0f, 1.0f,  0.0f, 1.0f),
    azer::Vector4(1.0f, 1.0f, -1.0f, 1.0f),
    azer::Vector4(0.0f, 1.0f, -1.0f, 1.0f),
  };

  azer::VertexDataPtr vdata(new azer::VertexData(effect->GetVertexDesc(),
                                                 arraysize(vertices)));
  memcpy(vdata->pointer(), vertices, sizeof(vertices));

  int32 indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7,
  };
  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(arraysize(indices), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr->pointer(), indices, sizeof(indices));
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
}

void CubeFrame::Render(azer::Vector3& pos1, azer::Vector3& pos2) {
}
