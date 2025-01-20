#ifndef REGEX_FA_NFA_HPP
#define REGEX_FA_NFA_HPP

#include "dfa.hpp"
#include "fa-include.hpp"

namespace regex_fa {

using FlatNfa = FlatDfa;

struct ScEdge {
  FlatStates source{};
  Terminal terminal{};
  FlatStates target{};
};

struct ScStep {
  FlatStates curSubset{};
  std::vector<ScEdge> scEdges{};
  std::vector<FlatStates> newSubsets{};
  std::vector<FlatStates> waitList{};
};

//  SC SubsetConstruction
struct ScLog {
  FlatNfa source;
  FlatDfa target;
  std::vector<ScStep> steps{};
};

class NfaLogger {
 public:
  ScLog sc_log{};

  static NfaLogger &GetInstance() {
    static NfaLogger nfa_logger{};
    return nfa_logger;
  }

 private:
  NfaLogger() = default;

 public:
  NfaLogger(const NfaLogger &) = delete;
  NfaLogger(const NfaLogger &&) = delete;
  NfaLogger &operator=(const NfaLogger &) = delete;

  void ClearLog() { sc_log.steps.clear(); }
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

#ifdef REGEX_FA_LOGGER
  NfaLogger &logger = NfaLogger::GetInstance();
#endif

 public:
  Nfa(NfaTable nfa_table, const StateId s, States f)
      : nfa_table_(std::move(nfa_table)), s_(s), f_(std::move(f)) {}

  explicit Nfa(const FlatNfa &flat_nfa)
      : s_(flat_nfa.s), f_(flat_nfa.f.begin(), flat_nfa.f.cbegin()) {
    for (auto state : flat_nfa.states) {
      nfa_table_[state] = {};
    }
    for (const auto &[source, target, terminal] : flat_nfa.flatEdges) {
      nfa_table_[source][terminal].emplace(target);
    }
  }

  [[nodiscard]] FlatNfa ToFlatNfa() const {
    auto flatNfa = FlatNfa{};
    flatNfa.s = s_;
    for (auto f : f_) {
      flatNfa.f.emplace_back(f);
    }
    for (auto &[u, transTable] : nfa_table_) {
      flatNfa.states.emplace_back(u);
      for (auto &[terminal, vStates] : transTable) {
        for (auto v : vStates) {
          flatNfa.flatEdges.emplace_back(u, v, terminal);
        }
      }
    }
    return flatNfa;
  }
  [[nodiscard]] Dfa ToDfa() const {
#ifdef REGEX_FA_LOGGER
    logger.ClearLog();
    logger.sc_log.source = ToFlatNfa();
#endif

    // subset -> {terminal, states}
    auto subset_table = std::map<OrderedStates, TransTable>{};
    auto q = std::queue<OrderedStates>{};
    q.push({s_});
    subset_table.try_emplace({s_});

    while (!q.empty()) {
      auto &[cur_subset, cur_trans_table] = *subset_table.find(q.front());
      q.pop();

      for (const auto state : cur_subset) {
        assert(nfa_table_.contains(state));
        for (auto &[terminal, states] : nfa_table_.at(state)) {
          cur_trans_table[terminal].insert(states.begin(), states.end());
        }
      }
#ifdef REGEX_FA_LOGGER
      auto step = ScStep{};
      step.curSubset.insert(step.curSubset.end(), cur_subset.begin(),
                            cur_subset.end());
      if (!logger.sc_log.steps.empty() &&
          !logger.sc_log.steps.back().waitList.empty()) {
        step.waitList.insert(step.waitList.end(),
                             logger.sc_log.steps.back().waitList.begin() + 1,
                             logger.sc_log.steps.back().waitList.end());
      }
      for (auto &[terminal, states] : cur_trans_table) {
        step.scEdges.emplace_back(toFlatStates(cur_subset), terminal,
                                  toFlatStates(states));
      }
      logger.sc_log.steps.emplace_back(std::move(step));
#endif
      for (const auto &states : cur_trans_table | std::views::values) {
        if (!subset_table.contains(states)) {
          q.push(states);
          subset_table.try_emplace(states);
#ifdef REGEX_FA_LOGGER
          auto &logger = NfaLogger::GetInstance();
          logger.sc_log.steps.back().newSubsets.emplace_back(
              toFlatStates(states));
          logger.sc_log.steps.back().waitList.emplace_back(
              toFlatStates(states));
#endif
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

    auto res = Dfa(std::move(dfa_table), new_ids[{s_}], std::move(dfa_f));
#ifdef REGEX_FA_LOGGER
    logger.sc_log.target = res.ToFlatDfa();
#endif
    return res;
  }
};

}  // namespace regex_fa

#endif  // REGEX_FA_NFA_HPP
