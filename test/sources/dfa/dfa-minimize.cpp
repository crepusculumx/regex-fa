// clang-format off
#include "test.h"
// clang-format on
#include "regex-fa/dfa.hpp"

using namespace regex_fa;

TEST(DfaHopcroftOne, SuccessCase1) {
  auto dfaTable = Dfa::DfaTable{
      {1, {{"a", 3}}},
      {2, {{"a", 1}}},
      {3, {}},
  };

  auto dfa = Dfa{dfaTable, 1, {3}};

  auto splitIndexTable = Dfa::SplitIndexTable{};
  splitIndexTable[1] = splitIndexTable[2] = 0;
  splitIndexTable[3] = 1;

  auto freeSplitId = Dfa::SplitId{2};
  auto res = dfa.HopcroftSplit(splitIndexTable, {1, 2}, freeSplitId);

  ASSERT_EQ(res, (Dfa::SplitTalbe{{2, {1}}, {3, {2}}}));
}
TEST(DfaHopcroftOne, NoNewSplitCase1) {
  auto dfaTable = Dfa::DfaTable{
      {1, {{"a", 3}}},
      {2, {{"a", 3}}},
      {3, {}},
  };

  auto dfa = Dfa{dfaTable, 1, {3}};

  auto splitIndexTable = Dfa::SplitIndexTable{};
  splitIndexTable[1] = splitIndexTable[2] = 0;
  splitIndexTable[3] = 1;

  auto freeSplitId = Dfa::SplitId{2};
  auto res = dfa.HopcroftSplit(splitIndexTable, {1, 2}, freeSplitId);

  ASSERT_EQ(res, (Dfa::SplitTalbe{{0, {1, 2}}}));
}

TEST(DfaHopcroft, Case1) {
  auto dfaTable = Dfa::DfaTable{
      {1, {{"a", 2}}},
      {2, {{"a", 2}}},
      {2, {}},
  };

  auto dfa = Dfa{dfaTable, 1, {1, 2}};

  auto res = dfa.Hopcroft();

  auto resDfaTable = Dfa::DfaTable{
      {0, {{"a", 0}}},
  };

  ASSERT_EQ(res.GetDfaTable(), resDfaTable);
}