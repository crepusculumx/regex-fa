#ifndef REGEX_FA_DFA_HPP
#define REGEX_FA_DFA_HPP

#include "fa-include.hpp"

namespace regex_fa {

struct FlatEdge {
  StateId source{};
  StateId target{};
  Terminal terminal{};
};

struct FlatDfaTable {
  std::vector<StateId> states{};
  std::vector<FlatEdge> flatEdges{};
};

struct FlatDfa {
  FlatDfaTable dfaTable{};
  StateId s{};
  std::vector<StateId> f{};
};

struct HopcroftSplit {
  StateId splitId{};
  std::vector<StateId> states{};

  HopcroftSplit() = default;

  HopcroftSplit(const StateId splitId, const States &states)
      : splitId(splitId) {
    for (auto stateId : states) {
      this->states.emplace_back(stateId);
    }
  }
};

struct HopcroftFlatSplitTable {
  std::vector<HopcroftSplit> splits{};
};

struct HopcroftSplitLog {
  Terminal splitTerminal{};
  HopcroftFlatSplitTable source{};
  HopcroftFlatSplitTable target{};
  HopcroftSplit split{};
  std::vector<HopcroftSplit> newSplits{};
};

struct HopcroftLog {
  FlatDfa source{};
  FlatDfa target{};
  std::vector<HopcroftSplitLog> hopcroftSplitLogs{};
};

class DfaLogger {
 public:
  HopcroftLog hopcroft_log{};

  static DfaLogger &GetInstance() {
    static DfaLogger dfa_logger{};
    return dfa_logger;
  }

 private:
  DfaLogger() = default;

 public:
  DfaLogger(const DfaLogger &) = delete;
  DfaLogger(const DfaLogger &&) = delete;
  DfaLogger &operator=(const DfaLogger &) = delete;

  void ClearHopcroftLog() { hopcroft_log = {}; }
};

class Dfa {
 public:
  /**
   * DfaTable = map<u, map<t, v>>.
   * Warning! For all u --t-> v, v must be DfaTable's key.
   */
  using TransTable = std::unordered_map<Terminal, StateId>;
  using DfaTable = std::unordered_map<StateId, TransTable>;

 private:
  DfaTable dfa_table_;
  StateId s_;
  States f_;

 public:
  Dfa(DfaTable table, StateId s, States f)
      : dfa_table_(std::move(table)), s_(s), f_(std::move(f)) {
    FixDfaTable();
  }

  explicit Dfa(const FlatDfa &flat_dfa) {
    s_ = flat_dfa.s;
    for (auto f : flat_dfa.f) {
      f_.insert(f);
    }
    for (auto &[u, v, terminal] : flat_dfa.dfaTable.flatEdges) {
      if (!dfa_table_.contains(u)) {
        dfa_table_[u] = {};
      }
      dfa_table_[u][terminal] = v;
    }
    FixDfaTable();
  }

  /**
   * Warning! For all u --t-> v, v must be DfaTable's key.
   * Fix v is not in key.
   */
  void FixDfaTable() {
    for (auto stateId : GetStates()) {
      if (!dfa_table_.contains(stateId)) {
        dfa_table_[stateId] = {};
      }
    }
  }

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
      for (const TransTable &old_trans_table = dfa_table_.find(old_id)->second;
           const auto &[terminal, next_id] : old_trans_table) {
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

  [[nodiscard]] FlatDfa ToFlatDfa() const {
    auto flatDfa = FlatDfa();
    flatDfa.s = s_;
    for (auto f : f_) {
      flatDfa.f.emplace_back(f);
    }
    for (auto state : GetStates()) {
      flatDfa.dfaTable.states.emplace_back(state);
    }

    for (auto &[u, transTable] : dfa_table_) {
      for (auto &[terminal, v] : transTable) {
        flatDfa.dfaTable.flatEdges.emplace_back(u, v, terminal);
      }
    }
    return flatDfa;
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

#ifdef REGEX_FA_LOGGER
  static HopcroftFlatSplitTable ToHopcroftFlatSplitTable(
      const SplitTalbe &split_talbe) {
    auto res = HopcroftFlatSplitTable{};
    for (const auto &[split_id, split] : split_talbe) {
      res.splits.emplace_back(split_id, split);
    }
    return res;
  }
#endif

  /**
   * Try to make new splits from split.
   * @param split_index_table
   * @param split The split works on. Split must not be empty.
   * @param free_split_id SplitId free to use.
   * @return Return new splits. If there is no new split, return split
   * itself.
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
        auto &trans_table = dfa_table_.at(u);

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
#ifdef REGEX_FA_LOGGER
        auto hopcroft_split_log = HopcroftSplitLog{};
        hopcroft_split_log.splitTerminal = terminal;
        DfaLogger::GetInstance().hopcroft_log.hopcroftSplitLogs.emplace_back(
            hopcroft_split_log);
#endif
        auto res = SplitTalbe{};
        for (const auto &newSplit : curSplitTable | std::views::values) {
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
#ifdef REGEX_FA_LOGGER
    DfaLogger::GetInstance().ClearHopcroftLog();
    DfaLogger::GetInstance().hopcroft_log.source = ToFlatDfa();
#endif

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

    InsertSplit(none_final_states);
    InsertSplit(final_states);

    while (!work_queue.empty()) {
      auto cur_split = std::move(work_queue.front());
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
        continue;
      }

#ifdef REGEX_FA_LOGGER
      auto &hopcroft_split_log =
          DfaLogger::GetInstance().hopcroft_log.hopcroftSplitLogs.back();
      hopcroft_split_log.split = {cur_split_id, cur_split};
      hopcroft_split_log.source = ToHopcroftFlatSplitTable(split_table);
#endif

      // New split
      split_table.erase(cur_split_id);  // Remove old split.
      // Add new splits.
      for (const auto &new_split : new_split_table | std::views::values) {
        InsertSplit(new_split);
#ifdef REGEX_FA_LOGGER
        hopcroft_split_log.newSplits.emplace_back(
            split_index_table[*new_split.begin()], new_split);
#endif
      }

#ifdef REGEX_FA_LOGGER
      hopcroft_split_log.target = ToHopcroftFlatSplitTable(split_table);
#endif

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

    auto res = Dfa{dfa_table, s, f};
#ifdef REGEX_FA_LOGGER
    DfaLogger::GetInstance().hopcroft_log.target = res.ToFlatDfa();
#endif
    return res;
  }

  /**
   * Get all states from dfa table.
   * @return
   */
  [[nodiscard]] States GetStates() const {
    auto res = States();
    for (auto &[u, transTable] : dfa_table_) {
      for (const auto &v : transTable | std::views::values) {
        res.insert(u);
        res.insert(v);
      }
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
    for (const auto &terminal : dfa_table_.at(state_id) | std::views::keys) {
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
   * @param trans_table
   * @return
   */
  [[nodiscard]] static Terminals GetTerminals(const TransTable &trans_table) {
    auto res = Terminals{};
    for (const auto &t : trans_table | std::views::keys) {
      res.emplace(t);
    }
    return res;
  }

  /**
   * Get all terminals from dfa table.
   * @param
   * @return
   */
  [[nodiscard]] Terminals GetTerminals() {
    auto res = Terminals{};
    for (const auto &trans_table : dfa_table_ | std::views::values) {
      for (const auto &t : trans_table | std::views::keys) {
        res.emplace(t);
      }
    }
    return res;
  }
};

}  // namespace regex_fa

#endif  // REGEX_FA_DFA_HPP
