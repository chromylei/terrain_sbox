#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/cubeframe.h"
#include "tersbox/tertur/sky/skyplane.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/sky/"
#define SHADER_NAME "diffuse.afx"
#define HEIGHTMAP  FILE_PATH_LITERAL("tersbox/tertur/media/heightmap01.bmp")
#define TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/dirt01.dds")
using base::FilePath;

class QuadTreeSplit : public azer::Tile::QuadTree::Splitable {
 public:
  virtual SplitRes Split(const azer::Tile::QuadTree::Node& node) {
    if (node.level > 7) {
      return kSplit;
    } else if (node.level == 7) {
      return kKeep;
    } else {
      return kDrop;
    }
  }
};

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : tile_(9, 0.5f) {
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
  azer::TexturePtr tex_;
  std::unique_ptr<DiffuseEffect> effect_;
  DiffuseEffect::DirLight light_;
  std::vector<azer::Tile::Pitch> pitches_;
  SkyPlane skyplane_;

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
  camera_.SetPosition(azer::Vector3(0.0f, 3.0f, 0.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 3.0f, -3.0f));
  tile_.Init();

  heightmap_.reset(azer::Image::Load(HEIGHTMAP));
  tex_.reset(azer::Texture::LoadShaderTexture(TERTEX, rs));

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(-0.5f, -1.0f, 0.0f, 0.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.05f, 0.05f, 0.05f, 1.0f);

  QuadTreeSplit splitable;
  azer::Tile::QuadTree tree(tile_.level());
  tree.Split(&splitable, &pitches_);
  skyplane_.Init(rs);
}

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      azer::Vector3 pos = tile_.vertex(j, i);
      int x = heightmap_->width() * ((float)j / (float)tile_.GetGridLineNum());
      int y = heightmap_->width() * ((float)i / (float)tile_.GetGridLineNum());
      float height = (float)(heightmap_->pixel(x, y) & 0x000000FF) * 0.1f;
      tile_.SetHeight(j, i, height * 0.5f);
    }
  }
  tile_.CalcNormal();

  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata.pointer();
  DiffuseEffect::Vertex* v = vertex;
  for (int i = 0; i < tile_.GetVertexNum(); ++i) {
    v->position = tile_.vertices()[i];
    v->normal = tile_.normal()[i];
    v++;
  }

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI * 10.0f / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);

  azer::Vector3 cpos = camera_.position();
  if (cpos.x > tile_.minx() && cpos.x < tile_.maxx()
      && cpos.z > tile_.minz() && cpos.z < tile_.maxz()) {
    int x = (cpos.x - tile_.minx()) / tile_.x_range() * tile_.GetGridLineNum();
    int z = (cpos.z - tile_.minz()) / tile_.z_range() * tile_.GetGridLineNum();
    azer::Vector3 tpos = tile_.vertex(x, z);
    tpos.x = cpos.x;
    tpos.z = cpos.z;
    tpos.y += 3.0f;
    camera_.SetPosition(tpos);
  }
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
  effect_->SetTexture(tex_);
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);

  skyplane_.Render(camera_, renderer);
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

