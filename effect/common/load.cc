#include "tersbox/effect/common/load.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_number_conversions.h"
#include "base/logging.h"

float toFloat(const std::string& input) {
  float value;
  DCHECK(base::StringToFloat(input, &value));
  return value;
}

void split(const std::string& str, std::vector<std::string>* vec) {
  const char* input = str.c_str();
  ::base::CStringTokenizer t(input, input + str.length(), " ");
  while (t.GetNext()) {
    vec->push_back(t.token());
  }
}

std::vector<Vertex> loadModel(const base::FilePath& path) {
  std::vector<Vertex> ret;
  std::string content;
  std::vector<std::string> lines;
  CHECK(::base::ReadFileToString(path, &content)) << path.value().c_str();
  base::SplitString(content, '\n', &lines);
  for (auto iter = lines.begin(); iter != lines.end(); ++iter) {
    std::string str;
    TrimWhitespaceASCII(*iter, TRIM_ALL, &str);
    char c = str.c_str()[0];
    if ((c >= '0' && c <= '9') || c == '+' || c == '-') {
      std::vector<std::string> num;
      split(str, &num);
      DCHECK_EQ(num.size(), 8u) << str;
      Vertex vertex(
          azer::Vector3(toFloat(num[0]), toFloat(num[1]), toFloat(num[2])),
          azer::Vector2(toFloat(num[3]), toFloat(num[4])),
          azer::Vector3(toFloat(num[5]), toFloat(num[6]), toFloat(num[7])));
      ret.push_back(vertex);
    }
  }

  return ret;
}
