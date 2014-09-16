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
  heightmap_ = azer::LoadImageFromFile(FilePath(HEIGHTMAP_PATH));

  tile_.Init();
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.0f, -0.7f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  azer::Texture::Options texopt;
  texopt.target = azer::Texture::kShaderResource;
  base::FilePath color_tex_path(TEX_PATH);
  azer::ImagePtr tex(azer::LoadImageFromFile(color_tex_path));
  color_tex_.reset(rs->CreateTexture(texopt, tex.get()));

  base::FilePath detail_tex_path(DETAIL_PATH);
  azer::ImagePtr tex2(azer::LoadImageFromFile(detail_tex_path));
  detail_tex_.reset(rs->CreateTexture(texopt, tex2.get()));

  cubeframe_.Init(rs);
}


void Terrain::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
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

  ClodEffect::Vertex* vertex = (ClodEffect::Vertex*)vdata.pointer();
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

  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, &idata));
  indices_num_ = tile_.indices().size();
  indices_.resize(indices_num_);
}

void Terrain::OnUpdateScene(double time, float delta_time) {
  pitches_.clear();
  frustrum_split_.clear();
  quadtree_.Split(4, &frustrum_split_, &pitches_);
  int32* cur = &(indices_[0]);
  for (auto iter = pitches_.begin(); iter != pitches_.end(); ++iter) {
    cur = tile_.InitPitchIndices(0, *iter, cur);
  }
  indices_num_ = cur - &(indices_[0]);
  if (indices_num_ > 0u) {
    azer::HardwareBufferDataPtr data(ib_->map(azer::kWriteDiscard));
    memcpy(data->data_ptr(), &(indices_[0]), indices_num_ * sizeof(int32));
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

  cubeframe_.SetDiffuse(azer::Vector4(1.0f, 0.0f, 0.0f, 1.0f));
  RenderPitch(frustrum_split_.fully_pitches_, camera_.GetProjViewMatrix(), renderer);
  
  cubeframe_.SetDiffuse(azer::Vector4(0.0f, 1.0f, 0.0f, 1.0f));
  RenderPitch(frustrum_split_.partial_pitches_, camera_.GetProjViewMatrix(), renderer);

  cubeframe_.SetDiffuse(azer::Vector4(0.0f, 0.0f, 1.0f, 1.0f));
  RenderPitch(frustrum_split_.none_pitches_, camera_.GetProjViewMatrix(), renderer);
}

void Terrain::RenderPitch(const std::vector<azer::Tile::Pitch>& pitches,
                          const azer::Matrix4& pv, azer::Renderer* renderer) {
  for (auto iter = pitches.begin(); iter != pitches.end(); ++iter) {
    azer::Vector3& p1 = tile_.vertex(iter->left, iter->top);
    azer::Vector3& p2 = tile_.vertex(iter->right, iter->bottom);
    cubeframe_.Render(p1, p2, renderer, pv);
  }
}


using azer::Tile;

void FrustrumSplit::clear() {
  partial_pitches_.clear();
  fully_pitches_.clear();
  none_pitches_.clear();
}

Tile::QuadTree::Splitable::SplitRes FrustrumSplit::Split(
    const Tile::QuadTree::Node& node) {
  if (node.level > 3) {
    partial_pitches_.push_back(node.pitch);
    return kSplit;
  }

  const azer::Vector3& minpos = tile_->vertex(node.pitch.left, node.pitch.top);
  const azer::Vector3& maxpos = tile_->vertex(node.pitch.right,node.pitch.bottom);

  azer::AxisAlignedBox box(azer::Vector3(minpos.x, tile_->miny(), minpos.z),
                           azer::Vector3(maxpos.x, tile_->maxy(), maxpos.z));
  azer::VisibleState state = box.IsVisible(*frustrum_, azer::Matrix4::kIdentity);
  if (state == azer::kPartialVisible) {
    partial_pitches_.push_back(node.pitch);
    return kSplit;
  } else if (state == azer::kFullyVisible) {
    fully_pitches_.push_back(node.pitch);
    return kKeep;
  }

  const azer::Vector3& pos = frustrum_->camera()->position();
  if (pos.x >= minpos.x && pos.z >= minpos.z
      && pos.x <= maxpos.x && pos.z <= maxpos.z) {
    partial_pitches_.push_back(node.pitch);
    return kSplit;
  } else {
    none_pitches_.push_back(node.pitch);
    return kDrop;
  }
}
