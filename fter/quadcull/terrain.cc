#include "tersbox/fter/quadcull/terrain.h"

#include "clod.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/fter/quadcull/"
#define SHADER_NAME "clod.afx"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/fter/res/heightmap003.bmp")
#define TEX_PATH FILE_PATH_LITERAL("tersbox/fter/res/tex/colorm001.bmp")
#define DETAIL_PATH FILE_PATH_LITERAL("tersbox/fter/res/tex/detailmap.tga")

using base::FilePath;

void Terrain::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new ClodEffect(shaders.GetShaderVec(), rs));
  heightmap_ = azer::util::LoadImageFromFile(FilePath(HEIGHTMAP_PATH));

  tile_.Init();
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.0f, -0.7f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  azer::Texture::Options texopt;
  texopt.target = azer::Texture::kShaderResource;
  base::FilePath color_tex_path(TEX_PATH);
  azer::ImagePtr tex(azer::util::LoadImageFromFile(color_tex_path));
  color_tex_.reset(rs->CreateTexture(texopt, tex.get()));

  base::FilePath detail_tex_path(DETAIL_PATH);
  azer::ImagePtr tex2(azer::util::LoadImageFromFile(detail_tex_path));
  detail_tex_.reset(rs->CreateTexture(texopt, tex2.get()));
}


void Terrain::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), tile_.GetVertexNum()));
  int cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      const azer::Vector3& pos = tile_.vertices()[cnt];
      int tx = (pos.x - tile_.minx()) / tile_.x_range() * (heightmap_->width() - 1);
      int ty = (pos.z - tile_.minz()) / tile_.z_range() * (heightmap_->height() - 1);
      uint32 rgba = heightmap_->pixel(tx, ty);
      float height = (rgba & 0x0000FF00) >> 8;
      tile_.SetHeight(i, j, height * 0.5f);
      cnt++;
    }
  }
  CHECK_EQ(cnt, tile_.GetVertexNum());
  tile_.CalcNormal();

  ClodEffect::Vertex* vertex = (ClodEffect::Vertex*)vdata->pointer();
  float cell = 1.0f / (float)tile_.GetGridLineNum();
  cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      ClodEffect::Vertex* v = vertex + cnt;
      v->position = tile_.vertices()[cnt];
      v->normal = tile_.normal()[cnt];
      v->texcoord0 = azer::Vector2(cell * i, cell * j);
      v->texcoord1 = azer::Vector2(kDetailMap * cell * i, kDetailMap * cell * j);
      cnt++;
    }
  }

  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(tile_.indices().size(), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr->pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, idata_ptr));
  indices_num_ = tile_.indices().size();
}

void Terrain::OnUpdateScene(double time, float delta_time) {
  std::vector<azer::util::Tile::Pitch> pitches;
  quadtree_.Split(4, &frustrum_split_, &pitches);
  std::vector<int32> indices;
  for (auto iter = pitches.begin(); iter != pitches.end(); ++iter) {
    tile_.InitPitchIndices(0, *iter, &indices);
  }
  indices_num_ = indices.size();
  if (indices_num_ > 0u) {
    azer::LockDataPtr data(ib_->map(azer::kWriteDiscard));
    memcpy(data->data_ptr(), &(indices[0]), indices.size() * sizeof(int32));
    ib_->unmap();
  }
}


void Terrain::OnRenderScene(azer::Renderer* renderer) {
  if (indices_num_ > 0) {
    azer::Matrix4 world = std::move(azer::Scale(0.5f, 0.5f, 0.5f));
    effect_->SetWorld(world);
    effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
    effect_->SetDirLight(light_);
    effect_->SetColorTex(color_tex_);
    effect_->SetDetailTex(detail_tex_);
    effect_->Use(renderer);
    renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList,
                        indices_num_, 0, 0);
  }
}
