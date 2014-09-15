#pragma once

#include "azer/render/render.h"
#include "tersbox/effect/common/load.h"
#include "tersbox/effect/common/tex_render.h"

class Object {
 public:
  Object(azer::VertexBufferPtr vb, azer::TexturePtr tex)
      : vb_(vb), tex_(tex) {}

  void SetWorld(const azer::Matrix4& world) {world_ = world;}

  const azer::Matrix4&  world() const { return world_;};
  azer::VertexBufferPtr& vertex_buffer() { return vb_;}
  azer::TexturePtr& tex() { return tex_;}
 private:
  azer::Matrix4 world_;
  azer::VertexBufferPtr vb_;
  azer::TexturePtr tex_;

  DISALLOW_COPY_AND_ASSIGN(Object);
};

typedef std::shared_ptr<Object> ObjectPtr;

template<class T>
inline azer::VertexBuffer* LoadVertex(const ::base::FilePath& path,
                                      T* effect, azer::RenderSystem* rs) {
  std::vector<Vertex> vertices = std::move(loadModel(path));
  azer::VertexData data(effect->GetVertexDesc(), vertices.size());
  T::Vertex* v = (T::Vertex*)data.pointer();
  for (auto iter = vertices.begin(); iter != vertices.end(); ++iter) {
    v->position = iter->position;
    v->texcoord = iter->texcoord;
    v->normal = iter->normal;
    v++;
  }
  return rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &data);
}

template<class T>
inline ObjectPtr LoadObject(const ::base::FilePath::StringType& path,
                            const ::base::FilePath::StringType& texpath,
                            T* effect, azer::RenderSystem* rs) {
  azer::VertexBufferPtr vb;
  azer::TexturePtr tex;

  vb.reset(LoadVertex<T>(FilePath(path), effect, rs));
  tex.reset(azer::CreateShaderTexture(texpath, rs));
  ObjectPtr obj(new Object(vb, tex));
  return obj;
}
