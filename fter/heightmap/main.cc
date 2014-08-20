#include "base/base.h"
#include "tersbox/fter/base/terrain_data.h"

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");
  TerrainData data(512, 0.5f, 128.0f);
  data.Gen();
  data.Save(::base::FilePath(FILE_PATH_LITERAL("a.bmp")));
  return 0;
}
