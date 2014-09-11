#include "tersbox/effect/water/water.h"

#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/effect/water/reflect.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/water/"
#define SHADER_NAME "water.afx"

#define BLUE_TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\blue01.dds")
#define WATER_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\water.txt")
#define WATER_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\water01.dds")

void Water::OnInit(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new WaterEffect(shaders.GetShaderVec(), rs));

  light_.dir = azer::Vector4(0.0f, -0.4f, -0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  vb_.reset(LoadVertex(::base::FilePath(WATER_PATH), rs));
  blue_tex_.reset(azer::CreateShaderTexture(BLUE_TEXPATH, rs));
  bumpmap01_.reset(azer::CreateShaderTexture(WATER_TEX, rs));  
}

void Water::OnDraw(const azer::Camera& camera, Reflect* reflect,
                   azer::Renderer* renderer) {
  azer::Matrix4 world = world = azer::Translate(0.0f, 2.75f, 0.0f);
  azer::Matrix4 pvw = camera.GetProjViewMatrix() * world;

  azer::Matrix4 refpvw = reflect->GetMirrorPV() * world;
  effect_->SetPVW(pvw);
  effect_->SetReflectPVW(refpvw);
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
  // effect_->SetTexture(bumpmap01_);
  effect_->SetTexture(blue_tex_);
  effect_->SetRefTexture(reflect->GetReflectTex());
  
  // effect_->SetTexture(reflect_->GetRenderTarget()->GetTexture()));
  // effect_->SetBumpTexture(bumpmap01_);
  effect_->Use(renderer);
  renderer->Draw(vb_.get(), azer::kTriangleList);
}

azer::VertexBuffer* Water::LoadVertex(const ::base::FilePath& path,
                                      azer::RenderSystem* rs) {
  azer::VertexDataPtr data;
  std::vector<Vertex> vertices = std::move(loadModel(path));
  data.reset(new azer::VertexData(effect_->GetVertexDesc(), vertices.size()));
  memcpy(data->pointer(), (uint8*)&vertices[0],
         sizeof(WaterEffect::Vertex) * vertices.size());
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data);
}

