#include "tersbox/base/cubeframe.h"

#include "azer/render/render.h"
#include "frame_effect.afx.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/base/"
#define SHADER_NAME "frame_effect.afx"

void CubeFrame::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  
  FrameEffect* effect = new FrameEffect(shaders.GetShaderVec(), rs);
  effect_.reset(effect);

  FrameEffect::Vertex vertices[] = {
    azer::Vector4(-0.5f, -0.5f,  0.5f, 1.0f),
    azer::Vector4( 0.5f, -0.5f,  0.5f, 1.0f),
    azer::Vector4( 0.5f, -0.5f, -0.5f, 1.0f),
    azer::Vector4(-0.5f, -0.5f, -0.5f, 1.0f),
    azer::Vector4(-0.5f,  0.5f,  0.5f, 1.0f),
    azer::Vector4( 0.5f,  0.5f,  0.5f, 1.0f),
    azer::Vector4( 0.5f,  0.5f, -0.5f, 1.0f),
    azer::Vector4(-0.5f,  0.5f, -0.5f, 1.0f),
  };

  azer::VertexData vdata(effect->GetVertexDesc(), arraysize(vertices));
  memcpy(vdata.pointer(), vertices, sizeof(vertices));

  int32 indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7,
  };
  azer::IndicesDataPtr idata_ptr(
    new azer::IndicesData(arraysize(indices), azer::IndicesData::kUint32));
  memcpy(idata_ptr->pointer(), indices, sizeof(indices));
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr.get()));
}

void CubeFrame::Render(const azer::Vector3& pos1, const azer::Vector3& pos2,
                       azer::Renderer* renderer) {
  azer::Vector3 p1 = azer::Vector3(std::min(pos1.x, pos2.x),
                                   std::min(pos1.y, pos2.y),
                                   std::min(pos1.z, pos2.z));
  azer::Vector3 p2 = azer::Vector3(std::max(pos1.x, pos2.x),
                                   std::max(pos1.y, pos2.y),
                                   std::max(pos1.z, pos2.z));
  azer::Vector3 scale = p2 - p1;
  azer::Vector3 pos = (p2 + p1) / 2.0f;
  azer::Matrix4 s = std::move(azer::Scale(scale));
  azer::Matrix4 trans = std::move(azer::Translate(pos));
  azer::Matrix4 world = std::move(world_ * std::move(trans * s));
  azer::Matrix4 pvw = std::move(pv_ * world);

  FrameEffect* effect = (FrameEffect*)effect_.get();
  effect->SetPVW(pvw);
  effect->SetDiffuse(diffuse_);
  effect->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kLineList);
}
