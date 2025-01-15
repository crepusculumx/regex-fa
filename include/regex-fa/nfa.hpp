#ifndef REGEX_FA_NFA_HPP
#define REGEX_FA_NFA_HPP

#include "dfa.hpp"
#include "fa-include.hpp"

namespace regex_fa {
struct SubsetConstructionStep {
  FlatStates states;
  Terminal terminal;
};

struct SubsetConstructionLog {};

class NfaLogger {
 public:
  static NfaLogger &GetInstance() {
    static NfaLogger dfa_logger{};
    return dfa_logger;
  }

 private:
  NfaLogger() = default;

 public:
  NfaLogger(const NfaLogger &) = delete;
  NfaLogger(const NfaLogger &&) = delete;
  NfaLogger &operator=(const NfaLogger &) = delete;
};

class Nfa {
 public:
  /**
   * NfaTable = map<u, map<t, set<v> > >.
   * Warning! For all u --t-> {v}, v must be NfaTable's key.
   */
  using TransTable = std::unordered_map<Terminal, OrderedStates>;
  using NfaTable = std::unordered_map<StateId, TransTable>;

 private:
  NfaTable nfa_table_;
  StateId s_;
  States f_;

 public:
  Nfa(NfaTable nfa_table, const StateId s, States f)
      : nfa_table_(std::move(nfa_table)), s_(s), f_(std::move(f)) {}

  [[nodiscard]] Dfa ToDfa() {
    // subset -> {terminal, states}
    auto subset_table = std::map<OrderedStates, TransTable>{};
    auto q = std::queue<OrderedStates>{};
    q.push({s_});

    while (!q.empty()) {
      auto [it, _] = subset_table.try_emplace(std::move(q.front()));
      q.pop();
      auto &[cur_subset, cur_trans_table] = *it;

      for (const auto state : cur_subset) {
        assert(nfa_table_.contains(state));
        for (auto &[terminal, states] : nfa_table_[state]) {
          cur_trans_table[terminal].insert(states.begin(), states.end());
        }
      }

      for (const auto &states : cur_trans_table | std::views::values) {
        if (!subset_table.contains(states)) {
          q.push(states);
        }
      }
    }

    // subset states -> new id
    auto new_ids = std::map<OrderedStates, StateId>{};
    StateId free_id = 0;
    for (const auto &states : subset_table | std::views::keys) {
      new_ids[states] = free_id++;
    }

    auto dfa_table = Dfa::DfaTable{};
    for (StateId i = 0; i < free_id; ++i) {
      dfa_table.emplace(i, Dfa::TransTable());
    }
    for (const auto &[u_subset, trans_table] : subset_table) {
      for (const auto &[terminal, v_subset] : trans_table)
        dfa_table[new_ids[u_subset]].emplace(terminal, new_ids[v_subset]);
    }

    auto dfa_f = States{};
    for (const auto &subset : subset_table | std::ranges::views::keys) {
      for (const auto &f : f_) {
        if (subset.contains(f)) {
          dfa_f.insert(new_ids[subset]);
          break;
        }
      }
    }

    return Dfa(std::move(dfa_table), new_ids[{s_}], std::move(dfa_f));
  }
};

}  // namespace regex_fa

#endif  // REGEX_FA_NFA_HPP
