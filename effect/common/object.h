#pragma once

#include "azer/render/render.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/effect/common/tex_render.h"

class Object {
 public:
  Object(azer::VertexBufferPtr vb, azer::TexturePtr tex)
      : vb_(vb), tex_(tex) {}

  void SetMatrix(const azer::Matrix4& world) {world_ = world;}

  template<class T>
  void Draw(const azer::Camera& camera, azer::Effect* effect,
              azer::Renderer* renderer);

 private:
  azer::Matrix4 world_;
  azer::VertexBufferPtr vb_;
  azer::TexturePtr tex_;
};

typedef std::shared_ptr<Object> ObjectPtr;

template<class T>
inline azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                        azer::RenderSystem* rs) {
  std::vector<Vertex> vertices = std::move(loadModel(path));
  azer::VertexData data(effect_->GetVertexDesc(), vertices.size());
  T::Vertex* v = (T::Vertex*)data.pointer();
  for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
    v->position = iter->position;
    v->texcoord = iter->texcoord;
    v->normal = iter->normal;
  }
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data);
}

template<class T>
inline ObjectPtr LoadObject(const ::base::FilePath::StringType& path,
                            const ::base::FilePath::StringType& texpath,
                            azer::RenderSystem* rs) {
  azer::VertexBufferPtr vb;
  azer::TexturePtr tex;

  vb.reset(LoadVertex<T>(FilePath(path), rs));
  tex.reset(azer::CreateShaderTexture(CUBE_TEX, rs));
  ObjectPtr obj(new Object(vb, tex));
  return obj;
}
