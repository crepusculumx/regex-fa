#ifndef REGEX_FA_FA_GRAPH_HPP
#define REGEX_FA_FA_GRAPH_HPP

#include "fa-include.hpp"

namespace regex_fa {

using FaUnweightedGraph = std::unordered_map<StateId, States>;

/**
 * Get Reachable states from graph start at s.
 * @param graph Graph's $key must contain all stateId.
 * @param start Set of starting points.
 * @return Reachable states
 */
inline States GetReachable(const FaUnweightedGraph &graph,
                           const States &start) {
  auto res = States{};

  auto q = std::queue<StateId>{};
  for (const auto &s : start) {
    q.push(s);
  }

  while (!q.empty()) {
    auto u = q.front();
    q.pop();

    res.emplace(u);

    for (const auto &v : graph.at(u)) {
      if (!res.contains(v)) {
        q.push(v);
      }
    }
  }
  return res;
}

}  // namespace regex_fa

#endif  // REGEX_FA_FA_GRAPH_HPP
