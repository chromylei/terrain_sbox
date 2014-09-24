#pragma once

#include "azer/math/math.h"
#include "azer/render/render.h"

struct DirLight{
  azer::Vector4 dir;
  azer::Vector4 diffuse;
  azer::Vector4 ambient;
};
