#pragma once

#include "automaton.hh"
#include "observer.hh"
#include "ppl_rational.hh"
#include "subject.hh"
#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"
#include "symbolic_update.hh"
#include "timed_word_parser.hh"
#include "timed_word_subject.hh"
#include "io_operators.hh"

#include <iostream>

namespace Parma_Polyhedra_Library {
  static inline std::size_t hash_value(const Symbolic::NumberValuation &p) {
    return static_cast<std::size_t>(p.hash_code());
  }
} // namespace Parma_Polyhedra_Library

using namespace Parma_Polyhedra_Library::IO_Operators;

#include <boost/unordered_set.hpp>

template <typename Timestamp>
struct DataParametricMonitorResult {
  std::size_t index;
  Timestamp timestamp;
  Symbolic::NumberValuation numberValuation;
  Symbolic::StringValuation stringValuation;
};

template<typename Timestamp>
class DataParametricMonitor : public SingleSubject<DataParametricMonitorResult<Timestamp>>,
                              public Observer<TimedWordEvent<PPLRational, Timestamp>> {
public:
  static const constexpr std::size_t unobservableActionID = 127;
  explicit DataParametricMonitor(const DataParametricTA<Timestamp> &automaton) : automaton(automaton) {
    configurations.clear();
    // configurations.reserve(automaton.initialStates.size());
    std::vector<Timestamp> initCVal(automaton.clockVariableSize);
    // by default, initSEnv is no violating set (variant)
    Symbolic::StringValuation initSEnv(automaton.stringVariableSize);
    // by default, initNEnv is the universe of dimension automaton.numberVariableSize
    Symbolic::NumberValuation initNEnv(automaton.numberVariableSize);
    for (const auto &initialState: automaton.initialStates) {
      configurations.insert({initialState, initCVal, initSEnv, initNEnv, 0});
    }
  }

  virtual ~DataParametricMonitor() = default;

  void notify(const TimedWordEvent<PPLRational, Timestamp> &event) override {
    const Action actionId = event.actionId;
    const std::vector<std::string> &strings = event.strings;
    const std::vector<PPLRational> &numbers = event.numbers;
    const Timestamp timestamp = event.timestamp;
    std::cout<<"Event:"<<actionId<<","<<timestamp<<","<<strings<<","<<numbers<<std::endl;
    boost::unordered_set<Configuration> nextConfigurations;
    boost::unordered_set<Configuration> currentConfigurations;
    for (Configuration conf: configurations) {
      // add a new dimension for time elapse.
      currentConfigurations.insert(std::move(conf));
    }
    while (!currentConfigurations.empty()) {
      nextConfigurations.clear();
      for (const Configuration &conf: currentConfigurations) {
        // この unobservableActionID が epsilon 遷移に対応している
        auto transitionIt = std::get<0>(conf)->next.find(unobservableActionID);
        if (transitionIt == std::get<0>(conf)->next.end()) {
          continue;
        }
        // make the current env
        const auto clockValuation = std::get<1>(conf);
        const auto stringEnv = std::get<2>(conf);
        const auto numberEnv = std::get<3>(conf);
        auto absTime = std::get<4>(conf);
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextCVal = clockValuation;
          auto nextSEnv = stringEnv;
          auto nextNEnv = numberEnv;
          auto extendedGuard = transition.guard;
          
          // clock の guard に, 不等式による制約がないことを仮定する
          auto df = diff(nextCVal, extendedGuard);
          // FIXME: when(= 1), when(=2) があると, d + 1 + 2 されて absTime になってしまう
          if(!df) {
            throw std::runtime_error("DataParametricMonitor: unsupported guard with inequality constraints on unobservable transition");
          }
          for (Timestamp &d: nextCVal) {
            d += df.value();
          }
          absTime += df.value();

          if (eval(nextCVal, extendedGuard) &&
              eval(transition.stringConstraints, nextSEnv, transition.numConstraints, nextNEnv)) {
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal[resetVar] = 0;
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextConfigurations.insert({transition.target.lock(), nextCVal, nextSEnv, nextNEnv, absTime});
            if (transition.target.lock()->isMatch) {
              this->notifyObservers({index, absTime, nextNEnv, nextSEnv});
            }
            configurations.insert({transition.target.lock(), nextCVal, nextSEnv, nextNEnv, absTime});
            std::cout<<"Insert:"<<transition.target.lock()<<","
              <<nextCVal<<","
              <<nextSEnv<<","
              <<nextNEnv<<","<<absTime<<std::endl;
          }
        }
      }

      std::swap(currentConfigurations, nextConfigurations);
    }
    nextConfigurations.clear();

    for (const Configuration &conf: configurations) {
      // make the current env
      auto clockValuation = std::get<1>(conf); //.clockValuation;
      const auto absTime = std::get<4>(conf);
      std::cout<<"Trans:"<<std::get<0>(conf)<<","
              <<std::get<1>(conf)<<","
              <<std::get<2>(conf)<<","
              <<std::get<3>(conf)<<","<<absTime<<std::endl;
      if(timestamp < absTime) {
        std::cout<<"Next:"<<std::get<0>(conf)<<","
              <<std::get<1>(conf)<<","
              <<std::get<2>(conf)<<","
              <<std::get<3>(conf)<<","<<absTime<<std::endl;
        nextConfigurations.insert(conf);
        continue;
      }
      for (Timestamp &d: clockValuation) {
        d += timestamp - absTime;
      }
      auto stringEnv = std::get<2>(conf); //.stringEnv;
      stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
      auto numberEnv = std::get<3>(conf); //.numberEnv;
      // add dimension for the data in the timed word.
      assert(numberEnv.space_dimension() == automaton.numberVariableSize);
      numberEnv.add_space_dimensions_and_embed(numbers.size());
      for (std::size_t i = 0; i < numbers.size(); i++) {
        numberEnv.add_constraint(Parma_Polyhedra_Library::Variable(automaton.numberVariableSize + i) *
                                  numbers[i].getDenominator() ==
                                 numbers[i].getNumerator());
      }

      auto transitionIt = std::get<0>(conf)->next.find(actionId);
      if (transitionIt == std::get<0>(conf)->next.end()) {
        continue;
      }
      for (const auto &transition: transitionIt->second) {
        // evaluate the guards
        auto nextSEnv = stringEnv;
        auto nextNEnv = numberEnv;
        if (eval(clockValuation, transition.guard) &&
            eval(transition.stringConstraints, nextSEnv, transition.numConstraints, nextNEnv)) {
          auto nextCVal = clockValuation;
          for (const VariableID resetVar: transition.resetVars) {
            nextCVal[resetVar] = 0;
          }
          transition.update.execute(nextSEnv, nextNEnv);
          nextSEnv.resize(automaton.stringVariableSize);
          nextNEnv.remove_higher_space_dimensions(automaton.numberVariableSize);
          std::cout<<"Next:"<<transition.target.lock()<<","
              <<nextCVal<<","
              <<nextSEnv<<","
              <<nextNEnv<<","<<timestamp<<std::endl;
          nextConfigurations.insert({transition.target.lock(), std::move(nextCVal), nextSEnv, nextNEnv, timestamp});
          if (transition.target.lock()->isMatch) {
            this->notifyObservers({index, timestamp, nextNEnv, nextSEnv});
          }
        }
      }
    }
    index++;
    configurations = std::move(nextConfigurations);
  }

private:
  const DataParametricTA<Timestamp> automaton;
  using Configuration = std::tuple<std::shared_ptr<DataParametricTAState<Timestamp>>, std::vector<Timestamp>,
                                   Symbolic::StringValuation, Symbolic::NumberValuation, Timestamp>;
  // Symbolic::NumberValuation>;
  /*  struct Configuration {
      std::shared_ptr<DataParametricTAState> state;
      std::vector<Timestamp> clockValuation;
      NonSymbolic::StringValuation stringEnv;
      Symbolic::NumberValuation numberEnv;
    };*/
  boost::unordered_set<Configuration> configurations;
  std::size_t index = 0;
};
