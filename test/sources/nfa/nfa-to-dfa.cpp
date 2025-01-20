// clang-format off
#include "test.h"
// clang-format on
#include "regex-fa/nfa.hpp"

using namespace regex_fa;

TEST(NfaToDfa, SuccessCase1) {
  const auto nfaTable = Nfa::NfaTable{
      {0, {{"a", {0, 1}}, {"b", {0, 2}}}},
      {1, {{"a", {3}}}},
      {2, {{"b", {3}}}},
      {3, {{"a", {3}}, {"b", {3}}}},
  };

  const auto nfa = Nfa{nfaTable, 0, {3}};
  auto res = nfa.ToDfa();

  // ASSERT_EQ(res,dfa);
}
