#ifndef REGEX_FA_DFA_HPP
#define REGEX_FA_DFA_HPP

#include <optional>
#include <queue>
#include <unordered_map>
#include <utility>

#include "fa-include.hpp"

namespace regex_fa {

class Dfa {
 public:
  /**
   * DfaTable = map<u, map<t, v>>.
   * For all u --t-> v, v must be DfaTable's key.
   */
  using TransTable = std::unordered_map<Terminal, StateId>;
  using DfaTable = std::unordered_map<StateId, TransTable>;

 private:
  DfaTable dfa_table_;
  StateId s_;
  States f_;

 public:
  Dfa(DfaTable table, StateId s, States f)
      : dfa_table_(std::move(table)), s_(s), f_(std::move(f)) {}

  [[nodiscard]] const DfaTable &GetDfaTable() const { return dfa_table_; }
  [[nodiscard]] StateId GetS() const { return s_; }
  [[nodiscard]] const States &GetF() const { return f_; }

  [[nodiscard]] Dfa Minimize() const { return Hopcroft(); }

  /*
   * Rename state id in bfs order.
   */
  [[nodiscard]] Dfa ReorderStates() const {
    std::unordered_map<StateId, StateId> new_id_table;  // old id -> new id
    std::unordered_map<StateId, StateId> old_id_table;  // new id -> old id
    DfaTable dfa_table;

    new_id_table[s_] = 0;
    old_id_table[0] = s_;

    StateId free_id = 1;
    for (int cur_id = 0; cur_id < free_id; cur_id++) {
      auto old_id = old_id_table[cur_id];
      TransTable new_trans_table;

      assert(dfa_table_.contains(old_id));
      const TransTable &old_trans_table = dfa_table_.find(old_id)->second;

      for (const auto &[terminal, next_id] : old_trans_table) {
        if (!new_id_table.contains(next_id)) {
          new_id_table[next_id] = free_id;
          old_id_table[free_id] = next_id;
          ++free_id;
        }
        new_trans_table.insert(std::make_pair(terminal, new_id_table[next_id]));
      }

      dfa_table.insert(std::make_pair(cur_id, new_trans_table));
    }

    States f{};
    for (const auto &state_id : f_) {
      f.insert(new_id_table[state_id]);
    }

    return {dfa_table, new_id_table[s_], f};
  }

 private:
  using SplitId = StateId;

  /**
   * Split should not be empty.
   */
  using Split = States;

  /**
   * For $key state, it is in $value split.
   */
  using SplitIndexTable = std::unordered_map<StateId, SplitId>;

  /**
   * For $key split, it contains $value states.
   */
  using SplitTalbe = std::unordered_map<SplitId, Split>;

  /**
   * Try to make new splits from split.
   * @param split_index_table
   * @param split The split works on. Split should not be empty.
   * @param free_split_id SplitId free to use.
   * @return Return new splits. If there is no new split, return split itself.
   */
  [[nodiscard]] SplitTalbe HopcroftSplit(
      const SplitIndexTable &split_index_table, const Split &split,
      SplitId &free_split_id) const {
    // Get terminals used by current split.
    auto terminals = GetTerminals(split);

    // For each terminal, see if it can make new split.
    for (const auto &terminal : terminals) {
      auto curSplitTable = SplitTalbe{};

      // For u --t-> u, u may goto empty.
      // Goto empty is different from goto v, which will need a split_id.
      auto emptySplitId = std::optional<SplitId>{std::nullopt};

      for (const auto &u : split) {
        auto trans_table = dfa_table_.at(u);

        // If goto v.
        if (trans_table.contains(terminal)) {
          auto v = trans_table.at(terminal);
          auto vSplitId = split_index_table.at(v);
          if (!curSplitTable.contains(vSplitId)) {
            curSplitTable[vSplitId] = {};
          }
          curSplitTable[vSplitId].emplace(u);
        }
        // If goto empty.
        else {
          if (!emptySplitId.has_value()) {
            // Use free_split_id as SplitId for empty split temporarily.
            emptySplitId.emplace(free_split_id);
            curSplitTable[emptySplitId.value()] = {};
          }
          curSplitTable[emptySplitId.value()].emplace(u);
        }
      }

      // Success
      if (curSplitTable.size() > 1) {
        auto res = SplitTalbe{};
        for (const auto &[split_id, newSplit] : curSplitTable) {
          res[free_split_id++] = newSplit;
        }
        return res;
      }
    }

    // Return split itself if there is no new split.
    auto res = SplitTalbe{};
    res.emplace(split_index_table.at(*split.begin()), split);
    return res;
  }

  [[nodiscard]] Dfa Hopcroft() const {
    auto split_table = SplitTalbe{};
    auto split_index_table = SplitIndexTable{};
    SplitId free_split_id{0};

    // split states into final states and non-final states
    Split final_states, none_final_states;
    for (const auto &state_id : GetStates()) {
      f_.contains(state_id) ? final_states.emplace(state_id)
                            : none_final_states.emplace(state_id);
    }

    auto work_queue = std::queue<Split>{};
    auto wait_queue = std::queue<Split>{};

    /**
     * Insert new split into split_table, split_index_table and work_queue.
     */
    auto InsertSplit = [&split_table, &split_index_table, &free_split_id,
                        &work_queue](const Split &split) -> void {
      if (split.empty()) {
        return;
      }
      auto split_id = free_split_id++;
      split_table[split_id] = split;

      for (const auto &state_id : split) {
        split_index_table[state_id] = split_id;
      }

      work_queue.emplace(split);
    };

    InsertSplit(final_states);
    InsertSplit(none_final_states);

    while (!work_queue.empty()) {
      auto cur_split = work_queue.front();
      work_queue.pop();

      // Split is not empty, use first state to find split_id.
      auto cur_split_id = split_index_table[*cur_split.begin()];

      // Split with size 1 can not make new cur_split.
      if (cur_split.size() == 1) {
        continue;
      }

      auto new_split_table =
          HopcroftSplit(split_index_table, cur_split, free_split_id);

      // No new spilt.
      if (new_split_table.size() == 1) {
        // Move to wait Queue. It may make new split when other new split occur.
        wait_queue.emplace(std::move(cur_split));
        break;
      }

      // New split
      split_table.erase(cur_split_id);  // Remove old split.
      // Add new splits.
      for (const auto &[split_id, new_split] : new_split_table) {
        InsertSplit(new_split);
      }

      // When new split occurs, previous split may also be able to be split.
      while (!wait_queue.empty()) {
        work_queue.emplace(std::move(wait_queue.front()));
        wait_queue.pop();
      }
    }

    // Build new dfa from split table.
    auto dfa_table = DfaTable{};
    auto s = StateId{split_index_table[s_]};
    auto f = States{};

    // Build dfa_table.
    for (const auto &[split_id, split] : split_table) {
      dfa_table[split_id] = {};

      // Split is not empty.
      // Use the first element as a representative of the split.
      auto u = *split.cbegin();

      // Add to dfa table
      for (const auto &[t, v] : dfa_table_.at(u)) {
        dfa_table[split_id][t] = split_index_table[v];
      }
    }

    // Build f.
    for (const auto &state_id : f_) {
      f.emplace(split_index_table[state_id]);
    }

    return Dfa{dfa_table, s, f};
  }

  /**
   * Get all states from dfa table.
   * @return
   */
  [[nodiscard]] States GetStates() const {
    auto res = States();
    for (const auto &[state_id, _] : dfa_table_) {
      res.emplace(state_id);
    }
    return res;
  }

  /**
   * Get terminals from dfa table used by state_id.
   * @param state_id
   * @return
   */
  [[nodiscard]] Terminals GetTerminals(StateId state_id) const {
    auto res = Terminals{};
    for (const auto &[terminal, _] : dfa_table_.at(state_id)) {
      res.emplace(terminal);
    }
    return res;
  }

  /**
   * Get terminals from dfa table used by states.
   * @param states
   * @return
   */
  [[nodiscard]] Terminals GetTerminals(const States &states) const {
    auto res = Terminals{};
    for (const auto &state : states) {
      res.merge(GetTerminals(state));
    }
    return res;
  }

  /**
   * Get all terminals from dfa trans table.
   * @param states
   * @return
   */
  [[nodiscard]] static Terminals GetTerminals(const TransTable &trans_table) {
    auto res = Terminals{};
    for (const auto &[t, v] : trans_table) {
      res.emplace(t);
    }
    return res;
  }

  /**
   * Get all terminals from dfa table.
   * @param states
   * @return
   */
  [[nodiscard]] Terminals GetTerminals() {
    auto res = Terminals{};
    for (const auto &[u, trans_table] : dfa_table_) {
      for (const auto &[t, v] : trans_table) {
        res.emplace(t);
      }
    }
    return res;
  }
};

}  // namespace regex_fa

#endif  // REGEX_FA_DFA_HPP
