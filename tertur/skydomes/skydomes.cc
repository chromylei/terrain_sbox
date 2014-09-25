#include "tersbox/tertur/skydomes/skydomes.h"

#include "azer/math/math.h"
#include "azer/util/util.h"
#include "tersbox/effect/common/load.h"

#define SPHERE_PATH FILE_PATH_LITERAL("tersbox\\tertur\\model\\skydome.txt")
#define SKYTEX_PATH FILE_PATH_LITERAL("samples\\resources\\texture\\marble01.dds")

#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/tertur/skydomes/"
#define SHADER_NAME "skydomes.afx"

using ::base::FilePath;

bool SkyDomes::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SkyDomesEffect(shaders.GetShaderVec(), rs));

  sphere_.reset(LoadVertex(FilePath(SPHERE_PATH), rs));
  // tex_.reset(azer::CreateShaderTexture(SKYTEX_PATH, rs));
  return true;
}

void SkyDomes::Render(const azer::Camera& camera, azer::Renderer* renderer) {
  azer::CullingMode cull = renderer->GetCullingMode();
  renderer->SetCullingMode(azer::kCullNone);
  azer::Matrix4 world = std::move(azer::Translate(camera.position())
                                  * azer::Scale(10.0f, 10.0f, 10.0f));
  azer::Matrix4 pvw = std::move(camera.GetProjViewMatrix() * world);
  effect_->SetPVW(pvw);
  effect_->SetApexColor(azer::Vector4(0.0f, 0.15f, 0.66f, 1.0f));
  effect_->SetCenterColor(azer::Vector4(0.81f, 0.38f, 0.66f, 1.0f));
  effect_->Use(renderer);
  renderer->Draw(sphere_.get(), azer::kTriangleList);
  renderer->SetCullingMode(cull);
}

azer::VertexBuffer* SkyDomes::LoadVertex(const ::base::FilePath& path,
                                         azer::RenderSystem* rs) {
  std::vector<Vertex> vertices = std::move(loadModel(path));
  azer::VertexData data(effect_->GetVertexDesc(), vertices.size());
  memcpy(data.pointer(), (uint8*)&vertices[0],
         sizeof(SkyDomesEffect::Vertex) * vertices.size());
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data);
}
