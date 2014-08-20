#include "base/base.h"
#include "tersbox/fter/base/terrain_data.h"

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");
  TerrainData data(256);
  data.Gen(255.0, 1.0);
  data.Save(::base::FilePath(FILE_PATH_LITERAL("a.bmp")));
  return 0;
}
