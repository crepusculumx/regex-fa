#ifndef WASM_EXPORT_NFA_EXPORT_HPP
#define WASM_EXPORT_NFA_EXPORT_HPP

#include "dfa-export.hpp"
#include "wasm-export-include.hpp"

namespace regex_fa {

inline void to_json(json& j, const ScEdge& data) {
  j = json{{"source", data.source},
           {"target", data.target},
           {"terminal", data.terminal}};
}

inline void from_json(const json& j, ScEdge& data) {
  j.at("source").get_to(data.source);
  j.at("target").get_to(data.target);
  j.at("terminal").get_to(data.terminal);
}

inline void to_json(json& j, const ScStep& data) {
  j = json{{"curSubset", data.curSubset},
           {"newSubsets", data.newSubsets},
           {"scEdges", data.scEdges},
           {"waitList", data.waitList}};
}

inline void from_json(const json& j, ScStep& data) {
  j.at("curSubset").get_to(data.curSubset);
  j.at("newSubsets").get_to(data.newSubsets);
  j.at("scEdges").get_to(data.scEdges);
  j.at("waitList").get_to(data.waitList);
}

inline void to_json(json& j, const ScLog& data) {
  j = json{
      {"source", data.source}, {"target", data.target}, {"steps", data.steps}};
}

inline void from_json(const json& j, ScLog& data) {
  j.at("source").get_to(data.source);
  j.at("target").get_to(data.target);
  j.at("steps").get_to(data.steps);
}

inline std::string NFaToDfa(const std::string& args) {
  const auto json_args = json::parse(args);
  const auto flatNfa = json_args.get<FlatNfa>();
  const auto nfa = Nfa{flatNfa};
  const auto res_dfa = nfa.ToDfa();
  const json res_json = NfaLogger::GetInstance().sc_log;
  return res_json.dump();
}
}  // namespace regex_fa
#endif  // WASM_EXPORT_NFA_EXPORT_HPP
