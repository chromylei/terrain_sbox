#include "azer/render/render.h"
#include "azer/math/math.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/base/camera_control.h"
#include "diffuse.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/bumpmap/"
#define SHADER_NAME "diffuse.afx"
#define TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\stone01.dds")
#define BUMP_TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\bump01.dds")
#define CUBE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\cube.txt")
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
  void InitVertex(azer::RenderSystem* rs);

  azer::VertexDataPtr data_;
  azer::VertexBufferPtr vb_;
  std::unique_ptr<DiffuseEffect> effect_;
  DiffuseEffect::DirLight light_;
  azer::Matrix4 world_;
  azer::Camera camera_;
  azer::TexturePtr tex_;
  azer::TexturePtr bump_tex_;
  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  InitRenderSystem(rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));

  InitVertex(rs);
  tex_.reset(azer::CreateShaderTexture(TEXPATH, rs));
  bump_tex_.reset(azer::CreateShaderTexture(BUMP_TEXPATH, rs));
  camera_.SetPosition(azer::Vector3(0.0f, 1.0f, 0.0f));

  light_.dir = azer::Vector4(0.4f, -0.4f, -0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);
}

void MainDelegate::InitVertex(azer::RenderSystem* rs) {
  std::vector<Vertex> v = std::move(loadModel(CUBE_PATH));
  std::vector<DiffuseEffect::Vertex> vertex;
  DiffuseEffect::Vertex tmp;
  for (size_t i = 0; i < v.size(); i+=3) {
    azer::Vector3 tang, binormal;
    azer::CalcTangentAndBinormal(v[i].position, v[i].texcoord,
                                 v[i + 1].position, v[i + 1].texcoord,
                                 v[i + 2].position, v[i + 2].texcoord,
                                 &tang, &binormal);
    for (int j = 0; j < 3; ++j) {
      tmp.position = v[i + j].position;
      tmp.texcoord = v[i + j].texcoord;
      tmp.normal = v[i + j].normal;
      tmp.tangent = tang;
      tmp.binormal = binormal;
      vertex.push_back(tmp);
    }
  }

  data_.reset(new azer::VertexData(effect_->GetVertexDesc(), vertex.size()));
  memcpy(data_->pointer(), (uint8*)&vertex[0],
         sizeof(DiffuseEffect::Vertex) * vertex.size());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data_.get()));
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
    
  azer::Matrix4 pvw = camera_.GetProjViewMatrix() * world_;
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  effect_->SetPVW(pvw);
  effect_->SetWorld(world_);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  effect_->SetTexture(tex_);
  effect_->SetBumpTexture(bump_tex_);
  effect_->Use(renderer);
  renderer->Draw(vb_.get(), azer::kTriangleList);
}

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  azer::Radians camera_speed(azer::kPI / 2.0f);    
  float rspeed = 3.14f * 2.0f / 4.0f;
  UpdatedownCamera(&camera_, camera_speed, delta_time);
  world_ = azer::Translate(0.0, 0.0, -4.0)
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
