#pragma once

#include "azer/render/render.h"
#include "azer/render/util.h"

#include "blur.afx.h"

class BlurGraphics {
 public:
  BlurGraphics() : cnt_(0) {}

  void Init(azer::RenderSystem* rs);

  azer::TexRenderTargetPtr& GetCurrentTex() {
    return targets_[cnt_ % targets_.size()];
  }

  azer::TexRenderTargetPtr& GetTarget(int cnt) {
    return targets_[cnt];
  }

  azer::TexturePtr GetBlurRTTex() {
    return finaly_->GetRTTex();
  }
  
  azer::Renderer* Begin();
  void End();
  void Render();
 private:
  void next() { cnt_ = (++cnt_) % targets_.size();}

  int cnt_;
  azer::EffectPtr effect_;
  std::vector<azer::TexRenderTargetPtr> targets_;
  azer::TexRenderTargetPtr finaly_;
  azer::OverlayPtr overlay_;
  DISALLOW_COPY_AND_ASSIGN(BlurGraphics);
};

