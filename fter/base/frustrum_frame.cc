#include "tersbox/fter/base/frustrum_frame.h"

#include "azer/render/render.h"

#include "frustrum_frame_effect.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/fter/base/"
#define SHADER_NAME "frustrum_frame_effect.afx"

void FrustrumFrame::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  
  FrustrumFrameEffect* effect =
      new FrustrumFrameEffect(shaders.GetShaderVec(), rs);
  effect_.reset(effect);

  FrustrumFrameEffect::Vertex vertices[] = {
    azer::Vector4(-1.0f,  1.0f, 0.0f, 1.0f),
    azer::Vector4( 1.0f,  1.0f, 0.0f, 1.0f),
    azer::Vector4( 1.0f, -1.0f, 0.0f, 1.0f),
    azer::Vector4(-1.0f, -1.0f, 0.0f, 1.0f),
    azer::Vector4(-1.0f,  1.0f, 0.999f, 1.0f),
    azer::Vector4( 1.0f,  1.0f, 0.999f, 1.0f),
    azer::Vector4( 1.0f, -1.0f, 0.999f, 1.0f),
    azer::Vector4(-1.0f, -1.0f, 0.999f, 1.0f),
    azer::Vector4(-1.0f,  1.0f, 0.95f, 1.0f),
    azer::Vector4( 1.0f,  1.0f, 0.95f, 1.0f),
    azer::Vector4( 1.0f, -1.0f, 0.95f, 1.0f),
    azer::Vector4(-1.0f, -1.0f, 0.95f, 1.0f),
  };
  azer::VertexDataPtr vdata(new azer::VertexData(effect->GetVertexDesc(),
                                                 arraysize(vertices)));
  memcpy(vdata->pointer(), vertices, sizeof(vertices));

  int32 indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7,
    8, 9, 9, 10, 10, 11, 11, 8,
  };
  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(arraysize(indices), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr->pointer(), indices, sizeof(indices));
  int32 indices_plane[] = {
    4, 0, 1, 4, 3, 0,  // top plane
    4, 1, 5, 5, 1, 2,  // right plane
    7, 6, 2, 7, 2, 3,  // bottom plane
    0, 4, 7, 0, 7, 3,  // left plane
  };

  azer::IndicesDataPtr idata_ptr2(
      new azer::IndicesData(arraysize(indices_plane), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr2->pointer(), indices_plane, sizeof(indices_plane));

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
  ibplane_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(),
                                         idata_ptr2));
}

void FrustrumFrame::Update(const azer::Camera& camera) {
  inverse_mat_ = std::move(camera.GetProjViewMatrix().Inverse());
}

void FrustrumFrame::Render(const azer::Matrix4& pv, azer::Renderer* renderer) {
  FrustrumFrameEffect* effect = (FrustrumFrameEffect*)effect_.get();
  effect->SetPVW(pv);
  effect->SetInverse(inverse_mat_);
  effect->SetDiffuse(azer::Vector4(0.0f, 0.8f, 0.8f, 1.0f));
  effect->Use(renderer);
  azer::CullingMode cm = renderer->GetCullingMode();
  renderer->SetCullingMode(azer::kCullNone);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kLineList);
  renderer->SetCullingMode(cm);
  // renderer->DrawIndex(vb_.get(), ibplane_.get(), azer::kTriangleList);
}
