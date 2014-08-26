#pragma once

#include "azer/util/util.h"
#include "azer/render/render.h"
#include "base/basictypes.h"
#include "tersbox/fter/base/cubeframe.h"

#include "clod.afx.h"

class FrustrumSplit : public azer::util::QuadTree::Splitable {
 public:
  FrustrumSplit(const azer::util::Tile* tile, const azer::Frustrum* frustrum)
    : tile_(tile), frustrum_(frustrum) {}
  virtual SplitRes Split(const azer::util::QuadTree::Node& node) OVERRIDE;
  void clear();
 public:
  std::vector<azer::util::Tile::Pitch> partial_pitches_;
  std::vector<azer::util::Tile::Pitch> fully_pitches_;
  std::vector<azer::util::Tile::Pitch> none_pitches_;
 private:
  const azer::util::Tile* tile_;
  const azer::Frustrum* frustrum_;
  DISALLOW_COPY_AND_ASSIGN(FrustrumSplit);
};

class Terrain {
 public:
  Terrain(const azer::Camera& camera)
      : tile_(8)
      , quadtree_(tile_.level())
      , camera_(camera)
      , kDetailMap(256 / 32)
      , frustrum_split_(&tile_, &camera.frustrum()) {
  }

  void Init(azer::RenderSystem* rs);
  void OnUpdateScene(double time, float delta_time);
  void OnRenderScene(azer::Renderer* renderer);

  const std::vector<azer::util::Tile::Pitch>& pitches() const { return pitches_;};

  azer::util::Tile& tile() { return tile_;}
  const azer::util::Tile& tile() const { return tile_;}
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  void RenderPitch(const std::vector<azer::util::Tile::Pitch>& pitches,
                   const azer::Matrix4& pv, azer::Renderer* renderer);
  CubeFrame cubeframe_;

  std::vector<azer::util::Tile::Pitch> pitches_;
  azer::util::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::ImagePtr heightmap_;
  azer::TexturePtr color_tex_;
  azer::TexturePtr detail_tex_;
  std::unique_ptr<ClodEffect> effect_;
  azer::util::QuadTree quadtree_;
  ClodEffect::DirLight light_;
  FrustrumSplit frustrum_split_;
  const azer::Camera& camera_;
  const int kDetailMap;
  int32 indices_num_;
  std::vector<int32> indices_;
  DISALLOW_COPY_AND_ASSIGN(Terrain);
};

void RenderPitch(const std::vector<azer::util::Tile::Pitch>& pitches,
                 const azer::Matrix4& pv, azer::Renderer* renderer);
