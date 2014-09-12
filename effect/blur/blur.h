#pragma once

#include "azer/render/render.h"
#include "azer/render/util.h"

#include "blur.afx.h"

class BlurGrphics {
 public:
  BlurGrphics();

  void Init(azer::RenderSystem* rs);

  azer::TexRenderTargetPtr& GetCurrentTex() {
    return targets_[cnt_ % targets_.size()];
  }

  void next() { cnt_++;}
  void Render(azer::Renderer* renderer);
 private:
  int cnt_;
  std::unique_ptr<BlurEffect> effect_;
  std::vector<azer::TexRenderTargetPtr> targets_;
  azer::TexRenderTargetPtr finaly_;
  azer::OverlayPtr overlay_;
  DISALLOW_COPY_AND_ASSIGN(BlurGrphics);
};

