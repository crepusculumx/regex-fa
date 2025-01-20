#ifndef REGEX_FA_TEST_FA_INCLUDE_HPP
#define REGEX_FA_TEST_FA_INCLUDE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace regex_fa {

using Terminal = std::string;
using Terminals = std::unordered_set<Terminal>;

using StateId = size_t;
using States = std::unordered_set<StateId>;
using OrderedStates = std::set<StateId>;

using FlatStates = std::vector<StateId>;

template <typename States>
  requires std::ranges::range<States>
FlatStates toFlatStates(const States &states) {
  auto res = FlatStates{};
  for (unsigned long state : states) {
    res.emplace_back(state);
  }
  std::ranges::sort(res);
  return res;
}

}  // namespace regex_fa

#endif  // REGEX_FA_TEST_FA_INCLUDE_HPP
