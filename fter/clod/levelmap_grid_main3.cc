#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/fter/base/camera_control.h"
#include "tersbox/fter/clod/clod.h"

#include <tchar.h>

#include "dirlight.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/fter/clod/"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/fter/res/heightmap001.bmp")
#define SHADER_NAME "dirlight.afx"
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate()
      : tile_(6)
      , clod_(&tile_) {
    int size = tile_.GetGridLineNum() * tile_.GetGridLineNum();
    levels_.reset(new int32[size]);
    memset(levels_.get(), 0, size);
  }
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  azer::util::Tile tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<DirlightEffect> effect_;
  DirlightEffect::DirLight light_;
  Clod clod_;
  std::unique_ptr<int32[]> levels_;
  int32 indices_num_;
  azer::ImagePtr heightmap_;
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
  tile_.Init();

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DirlightEffect(shaders.GetShaderVec(), rs));

  heightmap_ = azer::util::LoadImageFromFile(::base::FilePath(HEIGHTMAP_PATH));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.0f, -0.6f, 0.4f, 1.0f);
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
      int tx = (pos.x - tile_.minx()) / tile_.x_range() * heightmap_->width();
      int ty = (pos.z - tile_.minz()) / tile_.z_range() * heightmap_->width();
      uint32 rgba = heightmap_->pixel(tx, ty);
      float height = (rgba & 0x0000FF00) >> 8;
      tile_.SetHeight(i, j, height * 0.1f);
      cnt++;
    }
  }
  CHECK_EQ(cnt, tile_.GetVertexNum());
  // calc normal
  tile_.CalcNormal();
  DirlightEffect::Vertex* vertex = (DirlightEffect::Vertex*)vdata->pointer();
  DirlightEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    v->position = tile_.vertices()[i];
    v->normal = tile_.normal()[i];
    v++;
  }

  int level0 = tile_.GetGridLineNum() / 8;
  int level1 = tile_.GetGridLineNum() / 4;
  int level2 = tile_.GetGridLineNum() / 2;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      if (i < level0 && j < level0) {
        (levels_.get())[i * tile_.GetGridLineNum() + j] = 0;
      } else if (i < level1 && j < level1) {
        (levels_.get())[i * tile_.GetGridLineNum() + j] = 1;
      } else if (i < level2 && j < level2) {
        (levels_.get())[i * tile_.GetGridLineNum() + j] = 2;
      } else {
        (levels_.get())[i * tile_.GetGridLineNum() + j] = 3;
      }
    }
  }

  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(tile_.indices().size(), azer::IndicesData::kUint32));
  int32* begin = (int32*)idata_ptr->pointer();
  int32* end = clod_.GenIndices(begin, levels_.get());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  indices_num_ = end - begin;

  azer::IndicesBuffer::Options ibopt;
  ibopt.cpu_access = azer::kCPUWrite;
  ibopt.usage = azer::GraphicBuffer::kDynamic;
  ib_.reset(rs->CreateIndicesBuffer(ibopt, idata_ptr));
  camera_.SetPosition(azer::Vector3(0.0f, 10.0f, -15.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f;
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

  azer::Matrix4 world = std::move(azer::Scale(0.5f, 0.5f, 0.5f));
  effect_->SetWorld(world);
  effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
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

