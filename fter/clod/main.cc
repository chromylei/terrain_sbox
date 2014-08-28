#include "azer/render/render.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/frustrum_frame.h"
#include "tersbox/base/cubeframe.h"
#include "tersbox/fter/clod/terrain.h"

#include <tchar.h>

using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : terrain_(camera_) {}
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  azer::Camera camera_;
  Terrain terrain_;
  FrustrumFrame frame_;
  CubeFrame cubeframe_;
  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  // renderer->SetFillMode(azer::kWireFrame);
  renderer->EnableDepthTest(true);
  camera_.SetPosition(azer::Vector3(0.0f, 20.0f, -5.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 20.0f, 0.0f));
  camera_.frustrum().set_near(0.1f);

  terrain_.Init(rs);
  frame_.Init(rs);
  cubeframe_.Init(rs);
  cubeframe_.SetDiffuse(azer::Vector4(0.0f, 1.0f, 0.0f, 1.0f));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(5.0f * azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);

  
  // if( ::GetAsyncKeyState('Z') & 0x8000f || time < 0.001) {
    frame_.Update(camera_);
    terrain_.OnUpdateScene(time, delta_time);
    // }
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  terrain_.OnRenderScene(renderer);

  frame_.Render(camera_.GetProjViewMatrix(), renderer);

/*
  for (auto iter = terrain_.pitches().begin();
       iter != terrain_.pitches().end(); ++iter) {
    azer::Vector3& p1 = terrain_.tile().vertex(iter->left, iter->top);
    azer::Vector3& p2 = terrain_.tile().vertex(iter->right, iter->bottom);
    cubeframe_.Render(p1, p2, renderer, camera_.GetProjViewMatrix());
  }
*/
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

