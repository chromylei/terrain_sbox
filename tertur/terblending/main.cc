#include <cmath>

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/stringprintf.h"
#include "tersbox/base/camera_control.h"
#include "tersbox/base/cubeframe.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/terblending/"
#define SHADER_NAME "diffuse.afx"
#define HEIGHTMAP  FILE_PATH_LITERAL("tersbox/tertur/media/heightmap01.bmp")
#define TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/texture0%d.dds")
#define ALPHATEX  FILE_PATH_LITERAL("tersbox/tertur/media/alpha0%d.dds")
#define MATERIAL_TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/materialmap01.bmp")
#define COLORMAP_TERTEX  FILE_PATH_LITERAL("tersbox/tertur/media/colorm01.bmp")
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
  MainDelegate() : tile_(9, 0.2f) {
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
  std::vector<azer::TexturePtr> tex_;
  std::vector<azer::TexturePtr> alpha_;
  azer::TexturePtr material_tex_;
  azer::Texture::SamplerState material_sampler_;
  azer::Texture::SamplerState default_sampler_;
  std::unique_ptr<DiffuseEffect> effect_;
  DirLight light_;
  CubeFrame cubeframe_;
  std::vector<azer::Tile::Pitch> pitches_;

  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

float SampleHeight(azer::Image* image, float x, float y) {
  int x1 = std::max((int)std::floor(x), 0);
  int x2 = std::min((int)std::ceil(x), image->width() - 1);
  int y1 = std::max((int)std::floor(y), 0);
  int y2 = std::min((int)std::ceil(y), image->width() - 1);
  float height = (image->pixel(x1, y1) & 0x000000FF)
      + (image->pixel(x1, y2) & 0x000000FF)
      + (image->pixel(x2, y1) & 0x000000FF)
      + (image->pixel(x2, y2) & 0x000000FF);
  return height / 4.0f;
}

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  // renderer->SetFillMode(azer::kWireFrame);
  renderer->EnableDepthTest(true);
  camera_.SetPosition(azer::Vector3(0.0f, 3.0f, 3.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 3.0f, 0.0f));
  tile_.Init();

  heightmap_.reset(azer::Image::Load(HEIGHTMAP));
  material_sampler_.mag_filter = azer::Texture::kPoint;
  material_sampler_.min_filter = azer::Texture::kPoint;
  material_sampler_.mip_filter = azer::Texture::kPoint;
  default_sampler_.mag_filter = azer::Texture::kLinear;
  default_sampler_.min_filter = azer::Texture::kLinear;
  default_sampler_.mip_filter = azer::Texture::kLinear;

  for (int i = 0; i < 3; ++i) {
    FilePath::StringType path = ::base::StringPrintf(TERTEX, i + 1);
    azer::TexturePtr tex(azer::Texture::LoadShaderTexture(path, rs));
    tex->GenerateMips(7);
    tex->SetSamplerState(default_sampler_);
    tex_.push_back(tex);
  }
  for (int i = 0; i < 4; ++i) {
    FilePath::StringType path = ::base::StringPrintf(ALPHATEX, i + 1);
    azer::TexturePtr tex(azer::Texture::LoadShaderTexture(path, rs));
    tex->GenerateMips(-1);
    tex->SetSamplerState(default_sampler_);
    alpha_.push_back(tex);
    
  }
  material_tex_.reset(azer::Texture::LoadShaderTexture(MATERIAL_TERTEX, rs));
  material_tex_->SetSamplerState(material_sampler_);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(-0.5f, -1.0f, 0.0f, 0.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.05f, 0.05f, 0.05f, 1.0f);

  cubeframe_.Init(rs);

  QuadTreeSplit splitable;
  azer::Tile::QuadTree tree(tile_.level());
  tree.Split(&splitable, &pitches_);
}

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.GetVertexNum());
  azer::IndicesData idata(tile_.indices().size(), azer::IndicesData::kUint32);
  memcpy(idata.pointer(), &(tile_.indices()[0]),
         sizeof(int32) * tile_.indices().size());
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      azer::Vector3 pos = tile_.vertex(j, i);
      float x = heightmap_->width() * ((float)j / (float)tile_.GetGridLineNum());
      float y = heightmap_->width() * ((float)i / (float)tile_.GetGridLineNum());
      float height = SampleHeight(heightmap_.get(), x, y) * 0.1f;
      tile_.SetHeight(j, i, height * 0.5f);
    }
  }
  tile_.CalcNormal();

  DiffuseEffect::Vertex* vertex = (DiffuseEffect::Vertex*)vdata.pointer();
  DiffuseEffect::Vertex* v = vertex;
  int cnt = 0;
  float repeat_num = 10.0f;
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j = 0; j < tile_.GetGridLineNum(); ++j) {
      v->position = tile_.vertices()[cnt];
      v->normal = tile_.normal()[cnt];
      azer::Vector2 texcoord((float)j / (float)tile_.GetGridLineNum(),
                             (float)i / (float)tile_.GetGridLineNum());
      v->tex0 = texcoord * repeat_num;
      v->tex1 = texcoord;
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
  effect_->SetTexture0(tex_[0]);
  effect_->SetTexture1(tex_[1]);
  effect_->SetTexture2(tex_[2]);
  effect_->SetBlendTex0(alpha_[0]);
  effect_->SetBlendTex1(alpha_[1]);
  effect_->SetBlendTex2(alpha_[2]);
  effect_->SetBlendTex3(alpha_[3]);
  effect_->SetMaterialMap(material_tex_);
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);

  /*
  cubeframe_.SetWorldMatrix(azer::Matrix4::kIdentity);
  cubeframe_.SetPVMatrix(camera_.GetProjViewMatrix());
  cubeframe_.SetDiffuse(azer::Vector4(1, 0, 0, 1));
  for (auto iter = pitches_.begin(); iter != pitches_.end(); ++iter) {
    const azer::Tile::Pitch& pitch = *iter;
    azer::Vector3 minpos = tile_.vertex(pitch.left, pitch.top);
    minpos.y = 0.0f;
    azer::Vector3 maxpos = tile_.vertex(pitch.right, pitch.bottom);
    maxpos.y = 20.0f;
    cubeframe_.Render(minpos, maxpos, renderer);
  }
  */
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

