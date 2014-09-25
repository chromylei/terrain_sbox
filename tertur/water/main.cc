#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/cubeframe.h"
#include "tersbox/tertur/water/water.h"
#include "tersbox/tertur/water/skyplane.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/water/"
#define SHADER_NAME "diffuse.afx"
#define HEIGHTMAP  FILE_PATH_LITERAL("tersbox/tertur/media/heightmap03.bmp")
#define TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/dirt.dds")
#define COLORMAP_TEX  FILE_PATH_LITERAL("tersbox/tertur/media/colorm02.bmp")
#define BUMP_TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/normal.dds")
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
  MainDelegate() : tile_(8, 1.0f) {
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
  azer::ImagePtr colormap_;
  azer::TexturePtr tex_;
  azer::TexturePtr bump_tex_;
  std::unique_ptr<DiffuseEffect> effect_;
  DirLight light_;
  std::vector<azer::Tile::Pitch> pitches_;
  Water water_;
  SkyPlane skyplane_;

  void DrawScene(const azer::Camera& camera, azer::Renderer* renderer);

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
  camera_.SetPosition(azer::Vector3(-40.0f, 15.0f, -60.0f));
  camera_.SetLookAt(azer::Vector3(10.0f, 15.0f, 0.0f));
  tile_.Init();

  heightmap_.reset(azer::Image::Load(HEIGHTMAP));
  colormap_.reset(azer::Image::Load(COLORMAP_TEX));
  tex_.reset(azer::Texture::LoadShaderTexture(TERTEX, rs));
  tex_->GenerateMips(8);
  bump_tex_.reset(azer::Texture::LoadShaderTexture(BUMP_TERTEX, rs));
  bump_tex_->GenerateMips(8);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.5f, -0.75f, 0.25f, 0.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.05f, 0.05f, 0.05f, 1.0f);

  QuadTreeSplit splitable;
  azer::Tile::QuadTree tree(tile_.level());
  tree.Split(&splitable, &pitches_);

  water_.SetDirLight(light_);
  water_.Init(azer::Vector3(0.0f, 4.0f, 0.0f), rs);
  skyplane_.Init(rs);
}

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  float repeat_num = 255.0f;
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      azer::Vector3 pos = tile_.vertex(j, i);
      float x = (float)j / (float)tile_.GetGridLineNum();
      float y = (float)i / (float)tile_.GetGridLineNum();
      azer::Vector4 c = heightmap_->BoxSample(x, y);
      float height = c.x * 0.1 * 255.0f;
      tile_.SetHeight(j, i, height * 0.5f);
    }
  }

  std::vector<azer::Vector3> tangent, binormal;
  tile_.CalcNormal();
  tile_.CalcTBN(repeat_num, &tangent, &binormal);

  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata.pointer();
  DiffuseEffect::Vertex* v = vertex;
  int cnt = 0;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      v->position = tile_.vertices()[cnt];
      v->tex0 = tile_.texcoord()[cnt] * repeat_num;
      v->normal = tile_.normal()[cnt];
      v->binormal = binormal[cnt];
      v->tangent = tangent[cnt];
      float x = ((float)j / (float)tile_.GetGridLineNum());
      float y = ((float)i / (float)tile_.GetGridLineNum());
      v->color = azer::Vector3(colormap_->BoxSample(x, y));
      v++;
      cnt++;
    }
  }

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI * 10.0f / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* wrd = water_.BeginDrawRefract();
  DrawScene(camera_, wrd);

  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->Use();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  DrawScene(camera_, renderer);
  water_.Render(time, camera_, renderer);
}

void MainDelegate::DrawScene(const azer::Camera& camera, azer::Renderer* renderer) {
  azer::Matrix4 world = std::move(azer::Translate(0.0f, 0.0f, 0.0f));
  effect_->SetPVW(std::move(camera.GetProjViewMatrix() * world));
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetTexture(tex_);
  effect_->SetBumpTex(bump_tex_);
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);

  skyplane_.Render(camera, renderer);
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

