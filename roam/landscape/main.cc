#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/frustrum_frame.h"
#include "tersbox/roam/landscape/roam.h"
#include "tersbox/roam/landscape/terrain.h"

#include <tchar.h>
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate(){}
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  azer::Camera camera_;

  azer::Camera camera_cull_;
  FrustrumFrame frustrum_frame_;
  Terrain terrain_;
  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  renderer->SetFillMode(azer::kWireFrame);
  renderer->EnableDepthTest(true);
  camera_.frustrum().set_far(4000.0f);
  // camera_.SetPosition(azer::Vector3(0.0f, 800.0f, 0.0f));
  camera_.SetPosition(azer::Vector3(0.0f, 400.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));

  camera_cull_.SetPosition(azer::Vector3(0.0f, 0.0f, 0.0f));
  camera_cull_.SetLookAt(azer::Vector3(0.0f, 0.0f, -50.0f));
  frustrum_frame_.Init(rs);

  terrain_.Init(rs);
}

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();

  UpdatedownCamera(&camera_, camera_speed, delta_time);
  RendererControl(renderer, time);

  camera_cull_.yaw(camera_speed * delta_time * 0.3);
  camera_cull_.Update();
  frustrum_frame_.Update(camera_cull_);
  terrain_.OnUpdateScene(camera_cull_);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  terrain_.OnRenderScene(renderer, camera_);
  frustrum_frame_.Render(camera_.GetProjViewMatrix(), renderer);
}

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");
  
  MainDelegate delegate;
  azer::WindowHost win(azer::WindowHost::Options(), &delegate);
  win.Init();
  CHECK(azer::LoadRenderSystem(&win));
  LOG(ERROR) << "Current RenderSystem: " << azer::RenderSystem::Current()->name();
  delegate.Init();
  win.Show();
  azer::MainRenderLoop(&win);
  return 0;
}

