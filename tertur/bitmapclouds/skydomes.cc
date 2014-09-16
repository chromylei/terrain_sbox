#include "tersbox/tertur/skydomes/skydomes.h"

#define SPHERE_PATH FILE_PATH_LITERAL("tersbox\\effect\\data\\sphere.txt")
#define EFFECT_GEN_DIR "out/dbg/gen/tersbox/effect/water/"
#define SHADER_NAME "skydomes.afx"

using ::base::FilePath;

bool SkyDomes::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SkyDomesEffect(shaders.GetShaderVec(), rs));

  sphere_.reset(LoadVertex(FilePath(SPHERE_PATH), rs));
  tex_.reset(azer::CreateShaderTexture(MARBLE_TEX, rs));
}

void SkyDomes::Render(azer::Renderer* renderer) {
  effect_->Use(renderer);
  renderer->Draw(sphere_.get(), azer::kTriangleList);
}

azer::VertexBuffer* SkyDomes::LoadVertex(const ::base::FilePath& path,
                                         azer::RenderSystem* rs) {
  std::vector<Vertex> vertices = std::move(loadModel(path));
  azer::VertexData data(effect_->GetVertexDesc(), vertices.size());
  memcpy(data.pointer(), (uint8*)&vertices[0],
         sizeof(DiffuseEffect::Vertex) * vertices.size());
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data);
}
