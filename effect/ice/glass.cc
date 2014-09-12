#include "tersbox/effect/ice/glass.h"

#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/ice/"
#define SHADER_NAME "glass.afx"


#define GLASS_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\square.txt")
#define GLASS_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\glass01.dds")
#define GLASSBUMP_TEX FILE_PATH_LITERAL("samples\\resources\\texture\\icebump01.dds")
#define PERTURB_TEXPATH FILE_PATH_LITERAL("samples\\resources\\texture\\ice.dds")

void Glass::OnInit(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new GlassEffect(shaders.GetShaderVec(), rs));

  perturb_tex_.reset(azer::CreateShaderTexture(PERTURB_TEXPATH, rs));
  tex_.reset(azer::CreateShaderTexture(GLASS_TEX, rs));
  bumpmap_tex_.reset(azer::CreateShaderTexture(GLASSBUMP_TEX, rs));

  light_.dir = azer::Vector4(0.0f, -0.4f, -0.4f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.3f, 0.30f, 0.30f, 1.0f);

  InitVertex(::base::FilePath(GLASS_PATH), rs);
}

void Glass::OnUpdate(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 2.0f;
  world_ = std::move(azer::Translate(0.0f, 0.0f, -4.0f)
                                   * azer::RotateY(azer::Radians(time * rspeed)));
  world_ = std::move(azer::Translate(0.0f, 0.0f, -4.0f));
}

void Glass::OnDraw(const azer::Camera& camera, azer::TexturePtr refract,
                   azer::Renderer* renderer) {
  azer::Matrix4 pvw = camera.GetProjViewMatrix() * world_;

  effect_->SetPVW(pvw);
  effect_->SetWorld(world_);
  effect_->SetDirLight(light_);
  effect_->SetTexture(tex_);
  // effect_->SetPerturbTexture(perturb_tex_);
  effect_->SetBumpTexture(bumpmap_tex_);
  effect_->SetRefractTexture(refract);
  effect_->Use(renderer);
  renderer->Draw(vb_.get(), azer::kTriangleList);
}


void Glass::InitVertex(const ::base::FilePath& path, azer::RenderSystem* rs) {
  std::vector<Vertex> v = std::move(loadModel(path));
  std::vector<GlassEffect::Vertex> vertex;
  GlassEffect::Vertex tmp;
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
         sizeof(GlassEffect::Vertex) * vertex.size());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data));
}

