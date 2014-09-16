#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/effect/common/object.h"
#include "tersbox/effect/multilight_shadowmap/shadowmap.h"
#include "tersbox/base/camera_control.h"
#include "diffuse.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/multilight_shadowmap/"
#define SHADER_NAME "diffuse.afx"

#define GROUND_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\ground.txt")
#define GROUND_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\metal001.dds")
#define SPHERE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\sphere.txt")
#define SPHERE_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\ice.dds")
#define CUBE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\cube.txt")
#define CUBE_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\wall01.dds")
using base::FilePath;

void Draw(const azer::Camera& camera, DiffuseEffect* effect,
          const azer::Matrix4& smpv1, azer::Renderer* renderer, Object* obj) {
  azer::Matrix4& pvw = std::move(camera.GetProjViewMatrix() * obj->world());
  azer::Matrix4& sm_pvw1 = std::move(smpv1 * obj->world());
  effect->SetPVW(pvw);
  effect->SetWorld(obj->world());
  effect->SetShadowPVW1(sm_pvw1);
  effect->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  effect->SetTexture(obj->tex());
  effect->Use(renderer);
  renderer->Draw(obj->vertex_buffer().get(), azer::kTriangleList);
}

void Draw(const azer::Camera& camera, ShadowmapEffect* effect,
          azer::Renderer* renderer, Object* obj) {
  azer::Matrix4& pvw = std::move(camera.GetProjViewMatrix() * obj->world());
  effect->SetPVW(pvw);
  effect->Use(renderer);
  renderer->Draw(obj->vertex_buffer().get(), azer::kTriangleList);
}


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
  azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                 azer::RenderSystem* rs);
  ObjectPtr cube_;
  ObjectPtr sphere_;
  ObjectPtr ground_;
  std::unique_ptr<DiffuseEffect> effect_;
  DiffuseEffect::DirLight light1_;
  azer::Camera camera_;
  ShadowmapGraphic shadowmap1_;
  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  InitRenderSystem(rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));

  cube_ = LoadObject<DiffuseEffect>(CUBE_PATH, CUBE_TEX, effect_.get(), rs);
  ground_ = LoadObject<DiffuseEffect>(GROUND_PATH, GROUND_TEX, effect_.get(), rs);
  sphere_ = LoadObject<DiffuseEffect>(SPHERE_PATH, SPHERE_TEX, effect_.get(), rs);
  
  camera_.SetPosition(azer::Vector3(0.0f, 6.0f, -8.0));
  camera_.SetLookAt(azer::Vector3(.0f, 0.0f, 0.0f));

  light1_.dir = azer::Vector4(0.0f, -0.4f, -0.4f, 1.0f);
  light1_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light1_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  azer::Plane plane(azer::Vector3(1.0f, 2.75f, 1.0f),
                    azer::Vector3(1.0f, 2.75f, 0.0f),
                    azer::Vector3(0.0f, 2.75f, 1.0f));
  shadowmap1_.Init(rs);
  shadowmap1_.GetLightCamera().SetPosition(azer::Vector3(0.0f, 4.0f, 4.0));
  shadowmap1_.GetLightCamera().SetLookAt(azer::Vector3(0.0f, 0.0f, 0.0f));

  effect_->SetDirLight1(light1_);
  azer::Matrix4 world = azer::Translate(-3.0f, 1.0f, 0.0f);
  cube_->SetWorld(world);
  world = azer::Translate(3.0f, 1.0f, 0.0f);
  sphere_->SetWorld(world);
  world = azer::Translate(0.0f, 0.0f, 0.0f) * azer::Scale(0.4f, 0.4f, 0.4f);
  ground_->SetWorld(world);
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
  DCHECK(NULL != rs);
  azer::Renderer* renderer = rs->GetDefaultRenderer();

  ShadowmapEffect* effect = shadowmap1_.GetEffect();
  azer::Renderer* shadowmap_rd = shadowmap1_.Begin(renderer);
  Draw(shadowmap1_.GetLightCamera(), effect, shadowmap_rd, cube_.get());
  Draw(shadowmap1_.GetLightCamera(), effect, shadowmap_rd, sphere_.get());
  Draw(shadowmap1_.GetLightCamera(), effect, shadowmap_rd, ground_.get());
  shadowmap1_.End();

  renderer->Use();
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  effect_->SetShadowmapTexture1(shadowmap1_.GetShadowmap());
  
  Draw(camera_, effect_.get(), shadowmap1_.GetLightPVMat(), renderer, cube_.get());
  Draw(camera_, effect_.get(), shadowmap1_.GetLightPVMat(), renderer, sphere_.get());
  Draw(camera_, effect_.get(), shadowmap1_.GetLightPVMat(), renderer, ground_.get());
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
  std::vector<Vertex> vertices = std::move(loadModel(path));
  azer::VertexData data(effect_->GetVertexDesc(), vertices.size());
  memcpy(data.pointer(), (uint8*)&vertices[0],
         sizeof(DiffuseEffect::Vertex) * vertices.size());
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data);
}