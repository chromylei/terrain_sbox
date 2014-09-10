#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/base/camera_control.h"
#include "diffuse.afx.h"
#include "reflect.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/reflect/"
#define SHADER_NAME "diffuse.afx"
#define REF_SHADER_NAME "reflect.afx"
#define TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\seafloor.dds")
#define BLUE_TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\blue01.dds")
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  virtual void OnCreate() {}
  
  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitRenderSystem(azer::RenderSystem* rs);

  void InitBoxVertex(azer::RenderSystem* rs);
  void InitPlane(azer::RenderSystem* rs);
  void InitRenderTarget(azer::RenderSystem* rs);
  void RenderReflect(const azer::Matrix4& mirror);
  
  azer::VertexBufferPtr vb_;
  azer::VertexBufferPtr plane_vb_;
  std::unique_ptr<DiffuseEffect> effect_;
  std::unique_ptr<ReflectEffect> ref_effect_;
  DiffuseEffect::DirLight light_;
  azer::Matrix4 world_;
  azer::Camera camera_;
  azer::TexturePtr tex_;
  azer::TexturePtr blue_tex_;

  // render to target
  azer::TexturePtr rt_tex_;
  std::unique_ptr<azer::Renderer> sm_renderer_;
};

void MainDelegate::InitRenderTarget(azer::RenderSystem* rs) {
  azer::Texture::Options opt;
  opt.width = 800;
  opt.height = 600;
  opt.format = azer::kRGBAf;
  opt.target = (azer::Texture::BindTarget)
      (azer::Texture::kRenderTarget | azer::Texture::kShaderResource);
  sm_renderer_.reset(rs->CreateRenderer(opt));
}

void MainDelegate::RenderReflect(const azer::Matrix4& mirror) {
  sm_renderer_->Use();
  sm_renderer_->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  sm_renderer_->Clear(azer::Vector4(0.0f, 0.0f, 0.8f, 1.0f));
  sm_renderer_->ClearDepthAndStencil();
  sm_renderer_->SetCullingMode(azer::kCullNone);
  sm_renderer_->EnableDepthTest(true);
  const azer::Matrix4& proj = camera_.frustrum().projection();
  azer::Matrix4 reflect_view = std::move(mirror * camera_.GetViewMatrix());
  azer::Matrix4 pvw = std::move(proj * reflect_view * world_);
  // azer::Matrix4 pvw = std::move(camera_.GetProjViewMatrix() * world_);
  effect_->SetPVW(pvw);
  effect_->SetWorld(world_);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  effect_->SetTexture(tex_);
  effect_->Use(sm_renderer_.get());
  sm_renderer_->Draw(vb_.get(), azer::kTriangleList);
}

void MainDelegate::InitPlane(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR REF_SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR REF_SHADER_NAME ".ps", &shaders));
  ref_effect_.reset(new ReflectEffect(shaders.GetShaderVec(), rs));

  azer::VertexDataPtr data;
  std::vector<Vertex> vertices = std::move(
      loadModel(base::FilePath(TEXT("tersbox/effect/data/plane01.txt"))));
  data.reset(new azer::VertexData(effect_->GetVertexDesc(), vertices.size()));
  memcpy(data->pointer(), (uint8*)&vertices[0],
         sizeof(DiffuseEffect::Vertex) * vertices.size());
  plane_vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data));
}

void MainDelegate::InitBoxVertex(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));

  azer::VertexDataPtr data;
  std::vector<Vertex> vertices = std::move(
      loadModel(base::FilePath(TEXT("tersbox/effect/data/cube.txt"))));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  data.reset(new azer::VertexData(effect_->GetVertexDesc(), vertices.size()));
  memcpy(data->pointer(), (uint8*)&vertices[0],
         sizeof(DiffuseEffect::Vertex) * vertices.size());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data));
}

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  InitRenderSystem(rs);

  InitBoxVertex(rs);
  InitPlane(rs);
  InitRenderTarget(rs);

  azer::Texture::Options texopt;
  texopt.target = azer::Texture::kShaderResource;
  azer::ImagePtr imgptr(azer::LoadImageFromFile(TEXPATH));
  tex_.reset(rs->CreateTexture(texopt, imgptr.get()));
  azer::ImagePtr blue_imgptr(azer::LoadImageFromFile(BLUE_TEXPATH));
  blue_tex_.reset(rs->CreateTexture(texopt, blue_imgptr.get()));
  
  camera_.SetPosition(azer::Vector3(0.0f, 0.0f, 8.0f));

  light_.dir = azer::Vector4(0.0f, -0.4f, 0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);
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

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();

  azer::Plane plane(azer::Vector3(1.0f, -1.5f, 0.0f),
                    azer::Vector3(1.0f, -1.5f, 1.0f),
                    azer::Vector3(0.0f, -1.5f, 1.0f));
  azer::Matrix4 mirror = std::move(azer::MirrorTrans(plane));
  RenderReflect(mirror);

  DCHECK(NULL != rs);
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  azer::Matrix4 pvw = std::move(camera_.GetProjViewMatrix() * world_);
  renderer->Use();
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  effect_->SetPVW(pvw);
  effect_->SetWorld(world_);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  effect_->SetTexture(tex_);
  effect_->Use(renderer);
  renderer->Draw(vb_.get(), azer::kTriangleList);


  azer::Matrix4 world = std::move(azer::Translate(0.0, -1.5f, 0.0));
  pvw = std::move(camera_.GetProjViewMatrix() * world);
  azer::Matrix4 refpvw = camera_.frustrum().projection() *
      mirror * camera_.GetViewMatrix() * world;
  ref_effect_->SetReflectPVW(refpvw);
  ref_effect_->SetPVW(pvw);
  ref_effect_->SetWorld(world);
  ref_effect_->SetRefTexture(sm_renderer_->GetRenderTarget()->GetTexture());
  ref_effect_->SetTexture(blue_tex_);
  ref_effect_->Use(renderer);
  renderer->Draw(plane_vb_.get(), azer::kTriangleList);
}

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  azer::Radians camera_speed(azer::kPI / 2.0f);    
  float rspeed = 3.14f * 2.0f / 4.0f;
  UpdatedownCamera(&camera_, camera_speed, delta_time);
  world_ = azer::Translate(0.0, 0.0, 0.0)
      * azer::RotateY(azer::Radians(time * rspeed));
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
