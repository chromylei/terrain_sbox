#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/rawdata.h"
#include "tersbox/roam/indices/roam.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/roam/indices/"
#define SHADER_NAME "diffuse.afx"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/roam/data/height1024.raw")
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate()
      : tile_(8)
      , heightmap_(FilePath(HEIGHTMAP_PATH), 1024) {
    ROAMTree::Triangle l(0, tile_.GetGridLineNum() - 1,
                         tile_.GetGridLineNum() - 1, 0,
                         0, 0);
    left_tri_.reset(new ROAMTree(&tile_, l));
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
  std::unique_ptr<DiffuseEffect> effect_;
  RawHeightmap heightmap_;
  std::unique_ptr<ROAMTree> left_tri_;
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
  camera_.SetPosition(azer::Vector3(0.0f, 120.0f, -240.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));
  tile_.Init();

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  CHECK(heightmap_.Load());
  InitPhysicsBuffer(rs);
}


void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), tile_.GetVertexNum()));
  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata->pointer();
  DiffuseEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    const azer::Vector3& pos = tile_.vertices()[i];
    /*
    int tx = (pos.x - tile_.minx()) / tile_.x_range() * (heightmap_.width() - 1);
    int ty = (pos.z - tile_.minz()) / tile_.z_range() * (heightmap_.width() - 1);
    float height = heightmap_.height(tx, ty);
    v->position = azer::Vector4(pos.x, height * 0.1, pos.z, 1.0f);
    */
    v->position = azer::Vector4(pos.x, 0.0f, pos.z, 1.0f);
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
  UpdatedownCamera(&camera_, camera_speed, delta_time);

  left_tri_->tessellate();
  int32 * end = left_tri_->indices((int32*)idata_ptr_->pointer());
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

