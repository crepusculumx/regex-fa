#include "test.h"
#include "wasm-export/nfa-export.hpp"

using namespace regex_fa;

TEST(NfaToDfa, case1) {
  const std::string args = R"({"states":[0],"flatEdges":[],"f":[],"s":0})";
  const auto res = NFaToDfa(args);

  GTEST_LOG_(INFO) << "The res is: " << res;
}