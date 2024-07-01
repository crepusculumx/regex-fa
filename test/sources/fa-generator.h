#ifndef REGEX_FA_TEST_FA_GENERATOR_H
#define REGEX_FA_TEST_FA_GENERATOR_H

#include <random>

#include "regex-fa/regex-fa.hpp"

using namespace regex_fa;

[[nodiscard]] StateId RandomStateId() {
  static std::random_device dev{};
  static std::default_random_engine e{dev()};
  static std::uniform_int_distribution<StateId> u{
      std::numeric_limits<StateId>::min(), std::numeric_limits<StateId>::max()};
  return u(e);
}

[[nodiscard]] Terminal RandomTerminal() {
  static std::random_device dev{};
  static std::default_random_engine e{dev()};
  static std::uniform_int_distribution<char> u{
      std::numeric_limits<char>::min(),
      std::numeric_limits<char>::max()};
  auto res = Terminal{};
  res.push_back(u(e));
  return res;
}

[[nodiscard]] States RandomStates(size_t size) {
  auto res = States{};
  while (res.size() < size) {
    res.emplace(RandomStateId());
  }
  return res;
}

[[nodiscard]] Terminals RandomTerminals(size_t size) {
  auto res = Terminals{};
  while (res.size() < size) {
    res.emplace(RandomTerminal());
  }
  return res;
}

#endif  // REGEX_FA_TEST_FA_GENERATOR_H
