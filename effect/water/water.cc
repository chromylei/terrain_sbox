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

  InitVertex(::base::FilePath(WATER_PATH), rs);
  blue_tex_.reset(azer::CreateShaderTexture(BLUE_TEXPATH, rs));
  bumpmap01_.reset(azer::CreateShaderTexture(WATER_TEX, rs));  
}

void Water::OnDraw(const azer::Camera& camera, Reflect* reflect,
                   azer::TexturePtr refracttex, azer::Renderer* renderer) {
  azer::Matrix4 world = world = azer::Translate(0.0f, 2.75f, 0.0f);
  azer::Matrix4 pvw = camera.GetProjViewMatrix() * world;

  azer::Matrix4 refpvw = reflect->GetMirrorPV() * world;
  effect_->SetPVW(pvw);
  effect_->SetReflectPVW(refpvw);
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetBumpTexture(bumpmap01_);
  effect_->SetReflectTexture(reflect->GetReflectTex());
  effect_->SetRefractTexture(refracttex);

  // effect_->SetTexture(reflect_->GetRenderTarget()->GetTexture()));
  // effect_->SetBumpTexture(bumpmap01_);
  effect_->Use(renderer);
  renderer->Draw(vb_.get(), azer::kTriangleList);
}

void Water::InitVertex(const ::base::FilePath& path, azer::RenderSystem* rs) {
  std::vector<Vertex> v = std::move(loadModel(path));
  std::vector<WaterEffect::Vertex> vertex;
  WaterEffect::Vertex tmp;
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

  azer::VertexDataPtr data;
  data.reset(new azer::VertexData(effect_->GetVertexDesc(), vertex.size()));
  memcpy(data->pointer(), (uint8*)&vertex[0],
         sizeof(WaterEffect::Vertex) * vertex.size());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data));
}

