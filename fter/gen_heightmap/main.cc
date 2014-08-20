#include "base/base.h"
#include "tersbox/fter/base/terrain_data.h"

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");
  TerrainData data(512, 0.5f, 96.0f);
  data.GenPlasma();
  data.Save(::base::FilePath(FILE_PATH_LITERAL("a.bmp")));
  return 0;
}
