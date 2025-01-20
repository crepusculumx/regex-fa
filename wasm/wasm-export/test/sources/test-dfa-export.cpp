#include "test.h"
#include "wasm-export/dfa-export.hpp"

using namespace regex_fa;

TEST(DfaMinmize, case2) {
  const std::string args =
      R"({"dfaTable":{"states":[0,1,2,3,4,5,6],"flatEdges":[{"source":0,"target":1,"terminal":"a"},{"source":0,"target":2,"terminal":"b"},{"source":1,"target":3,"terminal":"a"},{"source":1,"target":2,"terminal":"b"},{"source":2,"target":1,"terminal":"a"},{"source":2,"target":4,"terminal":"b"},{"source":3,"target":3,"terminal":"a"},{"source":3,"target":5,"terminal":"b"},{"source":4,"target":6,"terminal":"a"},{"source":4,"target":4,"terminal":"b"},{"source":5,"target":6,"terminal":"a"},{"source":5,"target":4,"terminal":"b"},{"source":6,"target":3,"terminal":"a"},{"source":6,"target":5,"terminal":"b"}]},"f":[3,4,5,6],"s":0})";
  const auto res = DfaMinmize(args);

  GTEST_LOG_(INFO) << "The res is: " << res;
}