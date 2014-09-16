#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"

#include <tchar.h>

#include "simpletex.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/fter/simpletex/"
#define SHADER_NAME "simpletex.afx"
#define HEIGHTMAP_PATH FILE_PATH_LITERAL("tersbox/fter/res/heightmap002.bmp")
#define TEX_PATH FILE_PATH_LITERAL("tersbox/fter/res/tex/grass.tga")
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : tile_(8) {
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
  azer::ImagePtr heightmap_;
  azer::TexturePtr color_tex_;
  std::unique_ptr<SimpleTexEffect> effect_;
  SimpleTexEffect::DirLight light_;

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
  tile_.Init();

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SimpleTexEffect(shaders.GetShaderVec(), rs));
  heightmap_ = azer::LoadImageFromFile(FilePath(HEIGHTMAP_PATH));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.0f, -0.6f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.1f, 0.10f, 0.10f, 1.0f);

  azer::Texture::Options texopt;
  texopt.target = azer::Texture::kShaderResource;
  base::FilePath color_tex_path(TEX_PATH);
  azer::ImagePtr tex(azer::LoadImageFromFile(color_tex_path));
  azer::SaveImage(tex.get(), FilePath(FILE_PATH_LITERAL("tmp.bmp")));
  color_tex_.reset(rs->CreateTexture(texopt, tex.get()));
}

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  int cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      const azer::Vector3& pos = tile_.vertices()[cnt];
      int tx = (pos.x - tile_.minx()) / tile_.x_range() * heightmap_->width();
      int ty = (pos.z - tile_.minz()) / tile_.z_range() * heightmap_->width();
      uint32 rgba = heightmap_->pixel(tx, ty);
      float height = (rgba & 0x0000FF00) >> 8;
      tile_.SetHeight(i, j, height * 0.5f);
      cnt++;
    }
  }
  CHECK_EQ(cnt, tile_.GetVertexNum());
  tile_.CalcNormal();

  SimpleTexEffect::Vertex* vertex = (SimpleTexEffect::Vertex*)vdata.pointer();
  float cell = 1.0f / (float)tile_.GetGridLineNum();
  cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      SimpleTexEffect::Vertex* v = vertex + cnt;
      v->position = tile_.vertices()[cnt];
      v->normal = tile_.normal()[cnt];
      v->texcoord0 = azer::Vector2(cell * i, cell * j);
      cnt++;
    }
  }

  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  azer::Matrix4 world = std::move(azer::Scale(0.5f, 0.5f, 0.5f));
  effect_->SetWorld(world);
  effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
  effect_->SetDirLight(light_);
  effect_->SetColorTex(color_tex_);
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

