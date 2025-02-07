#ifndef WASM_EXPORT_DFA_EXPORT_HPP
#define WASM_EXPORT_DFA_EXPORT_HPP

#include "wasm-export-include.hpp"

namespace regex_fa {

inline void to_json(json& j, const FlatEdge& data) {
  j = json{{"source", data.source},
           {"target", data.target},
           {"terminal", data.terminal}};
}

inline void from_json(const json& j, FlatEdge& data) {
  j.at("source").get_to(data.source);
  j.at("target").get_to(data.target);
  j.at("terminal").get_to(data.terminal);
}

inline void to_json(json& j, const FlatDfa& data) {
  j = json{{"states", data.states},
           {"flatEdges", data.flatEdges},
           {"s", data.s},
           {"f", data.f}};
}

inline void from_json(const json& j, FlatDfa& data) {
  j.at("states").get_to(data.states);
  j.at("flatEdges").get_to(data.flatEdges);
  j.at("s").get_to(data.s);
  j.at("f").get_to(data.f);
}

inline void to_json(json& j, const HopcroftSplit& data) {
  j = json{{"splitId", data.splitId}, {"states", data.states}};
}

inline void from_json(const json& j, HopcroftSplit& data) {
  j.at("splitId").get_to(data.splitId);
  j.at("states").get_to(data.states);
}

inline void to_json(json& j, const HopcroftFlatSplitTable& data) {
  j = json{
      {"splits", data.splits},
  };
}

inline void from_json(const json& j, HopcroftFlatSplitTable& data) {
  j.at("splits").get_to(data.splits);
}

inline void to_json(json& j, const HopcroftSplitLog& data) {
  j = json{
      {"splitTerminal", data.splitTerminal},
      {"source", data.source},
      {"target", data.target},
      {"split", data.split},
      {"newSplits", data.newSplits},
  };
}

inline void from_json(const json& j, HopcroftSplitLog& data) {
  j.at("splitTerminal").get_to(data.splitTerminal);
  j.at("source").get_to(data.source);
  j.at("target").get_to(data.target);
  j.at("split").get_to(data.split);
  j.at("newSplits").get_to(data.newSplits);
}

inline void to_json(json& j, const HopcroftLog& data) {
  j = json{
      {"source", data.source},
      {"target", data.target},
      {"hopcroftSplitLogs", data.hopcroftSplitLogs},
  };
}

inline void from_json(const json& j, HopcroftLog& data) {
  j.at("source").get_to(data.source);
  j.at("target").get_to(data.target);
  j.at("hopcroftSplitLogs").get_to(data.hopcroftSplitLogs);
}

inline std::string DfaMinimize(const std::string& args) {
  const auto json_args = json::parse(args);
  const auto flatDfa = json_args.get<FlatDfa>();
  const auto dfa = Dfa{flatDfa};
  const auto res_dfa = dfa.Minimize();
  const json res_json = DfaLogger::GetInstance().hopcroft_log;
  return res_json.dump();
}

}  // namespace regex_fa

#endif  // WASM_EXPORT_DFA_EXPORT_HPP
