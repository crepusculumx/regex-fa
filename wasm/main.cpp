#include <string>

#include "nlohmann/json.hpp"
#include "wasm-export/dfa-export.hpp"

using namespace regex_fa;
using json = nlohmann::json;

extern "C" {
char* LibCall(const char* funcName, const char* args) {
  static char* buffer = new char[1];

  const auto funcNameStr = std::string{funcName};
  const auto argsStr = std::string{args};

  using LibFunc = std::function<std::string(const std::string&)>;

  auto funcTable = std::unordered_map<std::string, LibFunc>{
      {"DfaMinimize", LibFunc{DfaMinmize}}};

  const auto res = funcTable[funcNameStr](argsStr);

  if (strlen(buffer) < strlen(res.c_str())) {
    delete[] buffer;
    buffer = new char[strlen(res.c_str()) + 1];
  }
  strcpy(buffer, res.c_str());
  return buffer;
}
}

// int main() {
//   std::string s = "{\"a\":[1,2,3,4]}";
//   LibCall(s.c_str());
// }