#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/rawdata.h"
#include "tersbox/roam/err_metric/roam.h"

#include <tchar.h>

#include "dirlight.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/roam/err_metric/"
#define SHADER_NAME "dirlight.afx"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/roam/data/height1024.raw")
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate()
      : tile_(8, 2.0f)
      , heightmap_(FilePath(HEIGHTMAP_PATH), 1024)
      , roam_(&tile_, 2) {
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
  azer::IndicesDataPtr idata_ptr_;
  std::unique_ptr<DirlightEffect> effect_;
  DirlightEffect::DirLight light_;
  RawHeightmap heightmap_;
  ROAMTree roam_;
  int32 indices_num_;
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
  camera_.SetPosition(azer::Vector3(0.0f, 800.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));
  tile_.Init();
  

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DirlightEffect(shaders.GetShaderVec(), rs));
  CHECK(heightmap_.Load());
  InitPhysicsBuffer(rs);

  roam_.Init();

  light_.dir = azer::Vector4(0.0f, -0.4f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.1f, 0.10f, 0.10f, 1.0f);
}


void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), tile_.GetVertexNum()));
  int cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      const azer::Vector3& pos = tile_.vertices()[cnt];
      int tx = (pos.x - tile_.minx()) / tile_.x_range() * (heightmap_.width() - 1);
      int ty = (pos.z - tile_.minz()) / tile_.z_range() * (heightmap_.width() - 1);
      float height = heightmap_.height(tx, ty);
      tile_.SetHeight(i, j, height);
      cnt++;
    }
  }
  CHECK_EQ(cnt, tile_.GetVertexNum());
  tile_.CalcNormal();

  DirlightEffect::Vertex* vertex = (DirlightEffect::Vertex*)vdata->pointer();
  DirlightEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    v->position = tile_.vertices()[i];
    // v->position.y = 0.0f;
    v->normal = tile_.normal()[i];
    v++;
  }

  idata_ptr_.reset(new azer::IndicesData(tile_.indices().size(),
                                         azer::IndicesData::kUint32));
  memcpy(idata_ptr_->pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));

  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, idata_ptr_));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();

  UpdatedownCamera(&camera_, camera_speed, delta_time);
  RendererControl(renderer, time);

  roam_.tessellate();
  int32 * end = roam_.indices((int32*)idata_ptr_->pointer());
  indices_num_ = end - (int32*)idata_ptr_->pointer();
  azer::HardwareBufferDataPtr data(ib_->map(azer::kWriteDiscard));
  memcpy(data->data_ptr(), idata_ptr_->pointer(), indices_num_ * sizeof(int32));
  ib_->unmap();
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
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.6f, 0.6f, 0.6f, 1.0f));
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList, indices_num_);
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

