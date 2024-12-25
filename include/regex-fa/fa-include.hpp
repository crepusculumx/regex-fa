#ifndef REGEX_FA_TEST_FA_INCLUDE_HPP
#define REGEX_FA_TEST_FA_INCLUDE_HPP

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
}  // namespace regex_fa

#endif  // REGEX_FA_TEST_FA_INCLUDE_HPP
