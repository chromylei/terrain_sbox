#pragma once

#include "azer/render/render.h"
#include "azer/util/tile.h"
#include "tersbox/base/rawdata.h"
#include "tersbox/roam/landscape/landscape.h"
#include "tersbox/roam/landscape/roam.h"

#include "dirlight.afx.h"

class Terrain {
 public:
  Terrain();
  void Init(azer::RenderSystem* rs);
  void OnUpdateScene(const azer::Camera& camera);
  void OnRenderScene(azer::Renderer* renderer, azer::Camera& camera);
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::IndicesDataPtr idata_ptr_;
  std::unique_ptr<DirlightEffect> effect_;
  DirlightEffect::DirLight light_;
  RawHeightmap heightmap_;
  std::vector<ROAMPitchPtr> roam_;
  int32 indices_num_;
  DISALLOW_COPY_AND_ASSIGN(Terrain);
};

