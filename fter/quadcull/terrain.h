#pragma once

#include "azer/util/util.h"
#include "azer/render/render.h"
#include "base/basictypes.h"

#include "clod.afx.h"

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
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::util::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::ImagePtr heightmap_;
  azer::TexturePtr color_tex_;
  azer::TexturePtr detail_tex_;
  std::unique_ptr<ClodEffect> effect_;
  azer::util::QuadTree quadtree_;
  ClodEffect::DirLight light_;
  azer::util::FrustrumSplit frustrum_split_;
  const azer::Camera& camera_;
  const int kDetailMap;
  int32 indices_num_;
  DISALLOW_COPY_AND_ASSIGN(Terrain);
};
