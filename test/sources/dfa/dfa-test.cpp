// clang-format off
#include "test.h"
// clang-format on

#include "regex-fa/dfa.hpp"

#include "fa-generator.h"

using namespace regex_fa;

TEST(Dfa, Init) {
  auto dfa_table = Dfa::DfaTable{
      {1, {{"a", 1}, {"b", 2}, {"c", 3}}},
      {2, {{"a", 1}}},
      {3, {}},
  };
  auto dfa = Dfa{dfa_table, 1, {1, 2}};
}

TEST(DfaGetTerminals, Case1) {
  auto dfa_table = Dfa::DfaTable{{1, {{"a", 1}, {"b", 2}, {"c", 3}}},
                                {2, {{"d", 1}, {"e", 2}}},
                                {3, {{"f", 2}, {"g", 2}}},
                                {4, {}}};
  auto dfa = Dfa{dfa_table, 1, {1, 2}};

  ASSERT_TRUE(
      (dfa.GetTerminals() == Terminals{"a", "b", "c", "d", "e", "f", "g"}));

  ASSERT_TRUE((dfa.GetTerminals(1) == Terminals{"a", "b", "c"}));
  ASSERT_TRUE((dfa.GetTerminals(4).empty()));

  ASSERT_TRUE(
      (dfa.GetTerminals(States{2, 3}) == Terminals{"d", "e", "f", "g"}));
}

TEST(DfaGetStates, Case1) {
  auto states = RandomStates(5);
  auto dfa_table = Dfa::DfaTable{};

  for (const auto &stateId : states) {
    dfa_table[stateId] = {};
  }

  auto dfa = Dfa{dfa_table, 0, {}};

  //
  ASSERT_TRUE((dfa.GetStates() == states));
}

TEST(DfaReorderStates, Case1) {
  auto dfa_table = Dfa::DfaTable{
      {2, {{"a", 2}, {"b", 4}, {"c", 10}}},
      {4, {{"a", 2}}},
      {10, {}},
  };
  auto dfa = Dfa{dfa_table, 2, {2, 4}};
  auto res = dfa.ReorderStates();

  // todo table
}
