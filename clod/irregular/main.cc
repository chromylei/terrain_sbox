#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/clod/irregular/clod.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/clod/irregular/"
#define SHADER_NAME "diffuse.afx"
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate()
      : tile_(8)
      , clod_(&tile_) {
  }
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  azer::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<DiffuseEffect> effect_;
  Clod clod_;
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
  tile_.Init();

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);
}


void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), tile_.GetVertexNum()));
  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata->pointer();
  DiffuseEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    const azer::Vector3& pos = tile_.vertices()[i];
    v->position = azer::Vector4(pos.x, 0, pos.z, 1.0f);
    v++;
  }

  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(tile_.indices().size(), azer::IndicesData::kUint32));
  int32* end = clod_.GenIndices((int32*)idata_ptr->pointer());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));

  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, idata_ptr));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
  camera_.SetPosition(azer::Vector3(0.0f, 1.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 1.0f, -5.0f));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  azer::Matrix4 world = std::move(azer::Translate(0.0f, 0.0f, 0.0f));
  effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);
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

