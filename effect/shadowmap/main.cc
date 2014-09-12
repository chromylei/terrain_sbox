#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/effect/common/tex_render.h"
#include "diffuse.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/shadowmap/"
#define SHADER_NAME "diffuse.afx"

#define GROUND_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\ground.txt")
#define GROUND_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\metal001.dds")
#define SPHERE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\sphere.txt")
#define SPHERE_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\ice.dds")
#define CUBE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\cube.txt")
#define CUBE_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\wall01.dds")
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() {}
  virtual void OnCreate() {}
  
  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitRenderSystem(azer::RenderSystem* rs);
  void RenderScene(const azer::Matrix4& pv, azer::Renderer* renderer, bool rb);
  azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                   azer::RenderSystem* rs);
  azer::VertexBufferPtr cube_;
  azer::TexturePtr cube_tex_;
  azer::VertexBufferPtr sphere_;
  azer::TexturePtr sphere_tex_;
  azer::VertexBufferPtr ground_;
  azer::TexturePtr ground_tex_;
  std::unique_ptr<DiffuseEffect> effect_;
  DiffuseEffect::DirLight light_;
  azer::Camera camera_;

  TexRender target_;
  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  InitRenderSystem(rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));

  cube_.reset(LoadVertex(FilePath(CUBE_PATH), rs));
  cube_tex_.reset(azer::CreateShaderTexture(CUBE_TEX, rs));
  
  ground_.reset(LoadVertex(FilePath(GROUND_PATH), rs));
  ground_tex_.reset(azer::CreateShaderTexture(GROUND_TEX, rs));
  
  sphere_.reset(LoadVertex(FilePath(SPHERE_PATH), rs));
  sphere_tex_.reset(azer::CreateShaderTexture(SPHERE_TEX, rs));
  camera_.SetPosition(azer::Vector3(0.0f, 6.0f, -8.0));
  camera_.SetLookAt(azer::Vector3(.0f, 0.0f, 0.0f));

  light_.dir = azer::Vector4(0.0f, -0.4f, -0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  azer::Plane plane(azer::Vector3(1.0f, 2.75f, 1.0f),
                    azer::Vector3(1.0f, 2.75f, 0.0f),
                    azer::Vector3(0.0f, 2.75f, 1.0f));
  target_.Init(rs);
}

void MainDelegate::InitRenderSystem(azer::RenderSystem* rs) {
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  // CHECK(renderer->GetCullingMode() == azer::kCullFront);
  // renderer->SetFillMode(azer::kWireFrame);
  renderer->SetCullingMode(azer::kCullBack);
  renderer->EnableDepthTest(true);
}

void MainDelegate::RenderScene(const azer::Matrix4& pv, azer::Renderer* renderer,
                               bool rb) {
  azer::Matrix4 world = azer::Translate(-3.0f, 1.0f, 0.0f);
  azer::Matrix4 pvw = pv * world;
  effect_->SetPVW(pvw);
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  effect_->SetTexture(cube_tex_);
  effect_->Use(renderer);
  renderer->Draw(cube_.get(), azer::kTriangleList);

  world = azer::Translate(3.0f, 1.0f, 0.0f);
  pvw = pv * world;
  effect_->SetPVW(pvw);
  effect_->SetWorld(world);
  effect_->SetTexture(sphere_tex_);
  effect_->Use(renderer);
  renderer->Draw(sphere_.get(), azer::kTriangleList);

  if (rb) {
    world = azer::Translate(0.0f, 0.0f, 0.0f) * azer::Scale(0.4f, 0.4f, 0.4f);
    pvw = pv * world;
    effect_->SetPVW(pvw);
    effect_->SetWorld(world);
    effect_->SetTexture(ground_tex_);
    effect_->Use(renderer);
    renderer->Draw(ground_.get(), azer::kTriangleList);
  }
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  DCHECK(NULL != rs);
  azer::Renderer* renderer = rs->GetDefaultRenderer();

  target_.BeginRender();
  RenderScene(camera_.GetProjViewMatrix(), target_.GetRenderer(), true);
  target_.Reset(renderer);

  renderer->Use();
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  RenderScene(camera_.GetProjViewMatrix(), renderer, true);
}

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  azer::Radians camera_speed(azer::kPI / 2.0f);    
  float rspeed = 3.14f * 2.0f / 4.0f;
  UpdatedownCamera(&camera_, camera_speed, delta_time);
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

azer::VertexBuffer* MainDelegate::LoadVertex(const ::base::FilePath& path,
                                             azer::RenderSystem* rs) {
  azer::VertexDataPtr data;
  std::vector<Vertex> vertices = std::move(loadModel(path));
  data.reset(new azer::VertexData(effect_->GetVertexDesc(), vertices.size()));
  memcpy(data->pointer(), (uint8*)&vertices[0],
         sizeof(DiffuseEffect::Vertex) * vertices.size());
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data);
}
