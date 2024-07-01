// clang-format off
#include "test.h"
// clang-format on

#include "regex-fa/fa-graph.hpp"

using namespace regex_fa;

TEST(FaGraphGetReachable, case1) {
  FaUnweightedGraph graph = {{1, {3}}, {2, {3}}, {3, {}}};
  ASSERT_EQ((GetReachable(graph, {1})), (States{1, 3}));
}