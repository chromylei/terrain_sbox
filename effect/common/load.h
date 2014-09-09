#pragma once

#include <vector>
#include "azer/math/math.h"
#include "base/files/file_path.h"

struct Vertex {
  azer::Vector3 position;
  azer::Vector2 texcoord;
  azer::Vector3 normal;
  Vertex(const azer::Vector3& p, const azer::Vector2& t, const azer::Vector3& n)
      : position(p)
      , texcoord(t)
      , normal(n) {}
};

std::vector<Vertex> loadModel(const base::FilePath& path);

