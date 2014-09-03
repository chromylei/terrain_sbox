#include "tersbox/roam/landscape/terrain.h"

#include "dirlight.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/roam/landscape/"
#define SHADER_NAME "dirlight.afx"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/roam/data/height1024.raw")

using base::FilePath;

Terrain::Terrain()
    : tile_(8, 2.0f)
    , heightmap_(FilePath(HEIGHTMAP_PATH), 1024) {
}

void Terrain::Init(azer::RenderSystem* rs) {
  tile_.Init();

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DirlightEffect(shaders.GetShaderVec(), rs));
  CHECK(heightmap_.Load());
  InitPhysicsBuffer(rs);

  const int kcell = 32;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      azer::Tile::Pitch pitch(i * 32, j * 32, (i + 1) * 32, (j + 1) * 32);
      ROAMPitchPtr ptr(new ROAMPitch(&tile_, pitch, 2));
      ptr->Init();
      roam_.push_back(ptr);
    }
  }

  light_.dir = azer::Vector4(0.0f, -0.4f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.1f, 0.10f, 0.10f, 1.0f);
}

void Terrain::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), tile_.GetVertexNum()));
  int cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      const azer::Vector3& pos = tile_.vertices()[cnt];
      int tx = (pos.x - tile_.minx()) / tile_.x_range() * (heightmap_.width() - 1);
      int ty = (pos.z - tile_.minz()) / tile_.z_range() * (heightmap_.width() - 1);
      float height = heightmap_.height(tx, ty);
      tile_.SetHeight(i, j, height);
      cnt++;
    }
  }
  CHECK_EQ(cnt, tile_.GetVertexNum());
  tile_.CalcNormal();

  DirlightEffect::Vertex* vertex = (DirlightEffect::Vertex*)vdata->pointer();
  DirlightEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    v->position = tile_.vertices()[i];
    // v->position.y = 0.0f;
    v->normal = tile_.normal()[i];
    v++;
  }

  idata_ptr_.reset(new azer::IndicesData(tile_.indices().size(),
                                         azer::IndicesData::kUint32));
  memcpy(idata_ptr_->pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));

  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, idata_ptr_));
}

void Terrain::OnUpdateScene(const azer::Camera& camera) {

  int32* beg = (int32*)idata_ptr_->pointer();
  int32* cur = beg;
  for (int i = 0; i < roam_.size(); ++i) {
    ROAMPitchPtr& ptr = roam_[i];
    ptr->tessellate(camera);
    cur = ptr->indices(cur);
  }
  indices_num_ = cur - (int32*)idata_ptr_->pointer();
  azer::HardwareBufferDataPtr data(ib_->map(azer::kWriteDiscard));
  memcpy(data->data_ptr(), idata_ptr_->pointer(), indices_num_ * sizeof(int32));
  ib_->unmap();
}

void Terrain::OnRenderScene(azer::Renderer* renderer, azer::Camera& camera) {
  azer::Matrix4 world = std::move(azer::Translate(0.0f, 0.0f, 0.0f));
  effect_->SetPVW(std::move(camera.GetProjViewMatrix() * world));
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.6f, 0.6f, 0.6f, 1.0f));
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList, indices_num_);
}
