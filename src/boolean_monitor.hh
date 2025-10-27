#pragma once

//(setq flycheck-clang-language-standard "c++17")

#include "automaton.hh"
#include "non_symbolic_update.hh"
#include "observer.hh"
#include "subject.hh"
#include "timed_word_subject.hh"
#include <boost/unordered_set.hpp>

//BooleanMonitor の Printer のベース, notifyObserver で渡す型
template <class Number> struct BooleanMonitorResult {
  std::size_t index; // イベントの通し番号
  double timestamp;
  NonSymbolic::NumberValuation<Number> numberValuation;
  NonSymbolic::StringValuation stringValuation;
};

namespace NonSymbolic {
  template <typename Number>
  class BooleanMonitor : public SingleSubject<BooleanMonitorResult<Number>>, public Observer<TimedWordEvent<Number>> {
  public:
    BooleanMonitor(const NonParametricTA<Number> &automaton) : automaton(automaton) {
      configurations.clear();
      // configurations.reserve(automaton.initialStates.size());
      std::vector<double> initCVal(automaton.clockVariableSize);
      std::cout<<"clockVariableSize: "<<automaton.clockVariableSize<<std::endl;
      // by default, initSEnv is no violating set (variant)
      StringValuation initSEnv(automaton.stringVariableSize);
      // by default, initNEnv is unset (optional)
      NumberValuation<Number> initNEnv(automaton.numberVariableSize);
      for (const auto &initialState: automaton.initialStates) {
        configurations.insert({initialState, initCVal, initSEnv, initNEnv});
      }
    }
    virtual ~BooleanMonitor() = default;
    void notify(const TimedWordEvent<Number> &event) {
      const Action actionId = event.actionId;
      const std::vector<std::string> &strings = event.strings;
      const std::vector<Number> &numbers = event.numbers;
      const double timestamp = event.timestamp;
      boost::unordered_set<Configuration> nextConfigurations;
      for (const Configuration &conf: configurations) {
        // make the current env
        // TimingValuation 型
        auto clockValuation = std::get<1>(conf); // conf.clockValuation;
        for (double &d: clockValuation) {
          //前にnotifyが呼ばれたの時間 absTime から経過した時間を clockValuation に足して更新する
          d += timestamp - absTime;
        }
        auto stringEnv = std::get<2>(conf); // conf.stringEnv;
        stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
        auto numberEnv = std::get<3>(conf); // conf.numberEnv;
        numberEnv.insert(numberEnv.end(), numbers.begin(), numbers.end());
        //stringEnv(numberEnv) は option<String> な vector. ただ末尾に追加しているだけ?

        //transitionIt は std::vector<AutomatonTransition<...>> を指す iterator, next は unordered_map<Action, ...>
        auto transitionIt = std::get<0>(conf)->next.find(actionId); // conf.state->next.find(actionId);
        if (transitionIt == std::get<0>(conf)->next.end()           // ;conf.state->next.end()
        ) {
          continue; //遷移先がない
        }
        // map の iterator は key と value の pair, second で value を取り出せる.
        // transition は AutomatonTransition<...>
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextSEnv = stringEnv;
          if (eval(clockValuation, transition.guard) &&//timing_constraint.hh の eval?
              //non_symbolic_update.hh の eval?, stringEnv と numberEnv が制約を満たすかどうか
              // FIXME: string は nextSEnv だが number は元の numberEnv のまま??
              eval(transition.stringConstraints, nextSEnv, transition.numConstraints, numberEnv)) { 
            auto nextCVal = clockValuation;
            auto nextNEnv = numberEnv;
            for (const VariableID resetVar: transition.resetVars) {//clockVariable のうちリセットするもの
              nextCVal[resetVar] = 0;
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextSEnv.resize(automaton.stringVariableSize);
            nextNEnv.resize(automaton.numberVariableSize);
            nextConfigurations.insert({transition.target.lock(), std::move(nextCVal), nextSEnv, nextNEnv});
            // 非決定的な状態遷移を追加 みたいな感じと思う
            if (transition.target.lock()->isMatch) {
              //struct は BooleanMonitorResult
              this->notifyObservers({index, timestamp, nextNEnv, nextSEnv});
            }
          }
        }
      }
      absTime = timestamp;
      index++;
      configurations = std::move(nextConfigurations);
    }

  private:
    const NonParametricTA<Number> automaton;

    // (automaton の状態), (clock 変数の値), (出現した string 引数), (出現した number 引数) の tuple
    using Configuration = std::tuple<std::shared_ptr<NonParametricTAState<Number>>, std::vector<double>,
                                     StringValuation, NumberValuation<Number>>;
    // struct Configuration {
    //   std::shared_ptr<AutomatonState<Number>> state;
    //   std::vector<double> clockValuation;
    //   StringValuation stringEnv;
    //   NumberValuation<Number> numberEnv;
    //   bool operator==(const Configuration x) const {
    //     return state == x.state && clockValuation == x.clockValuation && stringEnv == x.stringEnv && numberEnv ==
    //     x.numberEnv;
    //   }
    // };
    boost::unordered_set<Configuration> configurations;
    double absTime;
    std::size_t index = 0;
  };
} // namespace NonSymbolic
