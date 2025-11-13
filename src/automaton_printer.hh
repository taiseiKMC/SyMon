#pragma once

#include "io_operators.hh"

namespace boost {
  using ::operator>>;
  using ::operator<<;
} // namespace boost

namespace std {}

//(setq flycheck-clang-language-standard "c++17")
#include "automaton.hh"
#include "common_types.hh"
#include <iostream>

static inline char to_string(const bool b) {
  return b ? '1' : '0';
}


template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
static inline std::ostream &
operator<<(std::ostream &os,
           const TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &automaton) {
  os << "digraph G {\n"
    << "graph [\n"
    << "clock_variable_size = " << automaton.clockVariableSize << "\n"
    << "string_variable_size = " << automaton.stringVariableSize << "\n"
    << "number_variable_size = " << automaton.numberVariableSize << "\n"
    << "]" << std::endl;
  // Use a C++ cast to make the conversion explicit and avoid C-style cast.
  os << static_cast<const Automaton<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>>&>(
      automaton);
  os << "}" << std::endl;
  return os;
}

template <typename StringConstraint, typename NumberConstraint, typename Update>
static inline std::ostream &
operator<<(std::ostream &os,
           const TimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> &automaton) {
  os << "digraph G {\n"
    << "graph [\n"
    << "clock_variable_size = " << automaton.clockVariableSize << "\n"
    << "string_variable_size = " << automaton.stringVariableSize << "\n"
    << "number_variable_size = " << automaton.numberVariableSize << "\n"
    << "parameter_size = " << automaton.parameterSize << "\n"
    << "]" << std::endl;
  os << (Automaton<AutomatonState<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update>>)automaton;
  os << "}" << std::endl;
  return os;
}

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
static inline std::ostream &
operator<<(std::ostream &os,
           const Automaton<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>> &automaton) {
  std::vector<std::tuple<bool, bool>> stateIds(automaton.states.size());
  for (size_t i = 0; i < automaton.states.size(); ++i) {
    os << i << " [init="
       << to_string(std::find(automaton.initialStates.begin(), automaton.initialStates.end(), automaton.states[i]) !=
                    automaton.initialStates.end())
       << "][match=" << to_string(automaton.states[i]->isMatch)
       << "]" << std::endl;
  }
  os << automaton.states << std::endl;
  return os;
}

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
static inline std::ostream &operator<<(
    std::ostream &os,
    const std::vector<std::shared_ptr<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>>>
        &states) {
  for (size_t i = 0; i < states.size(); ++i) {
    for (const auto &[action, transitions]: states[i]->next) {
      for (const auto &transition: transitions) {
        os << i << " -> "
           << std::distance( //遷移先の state の index を計算
                  states.begin(),
                  std::find_if(states.begin(), states.end(),
                               [&](const std::shared_ptr<
                                   AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>> &s) {
                                 return s == transition.target.lock();
                               }))
           << " [label=" << action
           << "][s_constraints=\"" << transition.stringConstraints
           << "\"][n_constraints=\"" << transition.numConstraints
           << "\"][guard=\"" << transition.guard
           << "\"][reset=\"" << transition.resetVars
           << "\"][s_update=\"" << UpdateTraits<Update>::stringUpdate(transition.update)
           //TODO
           << "\"][n_update=\"" << UpdateTraits<Update>::numberUpdate(transition.update)
           << "\"]" << std::endl;
      }
    }
  }
  return os;
}
