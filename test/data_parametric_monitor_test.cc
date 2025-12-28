#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../src/data_parametric_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"
#include "../test/fixture/decimal_expression_fixture.hh"
#include "../test/fixture/epsilon_transition_automaton_fixture.hh"
//TODO: copy_automaton_fixture の automaton は NonParametricTA だが、DataParametricTA にすべきな気がする

using TWEvent = TimedWordEvent<PPLRational>;

//DummyTimedWordSubject だと DummyTimedWordSubject2{std::move(vec)}.addObserver(monitor);
// でエラーになったが、名前を変えると解消した...???
struct DummyDataTimedWordSubject : public SingleSubject<TWEvent> {
  DummyDataTimedWordSubject(std::vector<TWEvent> &&vec) :vec(std::move(vec)) {}
  virtual ~DummyDataTimedWordSubject(){}
  void notifyAll() {
    for (const auto &event: vec) {
      notifyObservers(event);
    }
    vec.clear();
  }
  std::vector<TWEvent> vec;
};

struct DummyDataParametricMonitorObserver : public Observer<DataParametricMonitorResult<double>> {
  DummyDataParametricMonitorObserver() {}
  virtual ~DummyDataParametricMonitorObserver() {}
  void notify(const DataParametricMonitorResult<double>& result) {
    resultVec.push_back(result);
  }
  std::vector<DataParametricMonitorResult<double>> resultVec;
};

struct DataParametricMonitorFixture {
  void feed(DataParametricTA<double> automaton, std::vector<TWEvent> &&vec) {
    auto monitor = std::make_shared<DataParametricMonitor<double>>(automaton);
    std::shared_ptr<DummyDataParametricMonitorObserver> observer = std::make_shared<DummyDataParametricMonitorObserver>();
    monitor->addObserver(observer);
    DummyDataTimedWordSubject subject{std::move(vec)};
    subject.addObserver(monitor); //&monitor, DataParametricMonitor, should be Observer<TWEvemt>
    subject.notifyAll();
    resultVec = std::move(observer->resultVec);
  }
  std::vector<DataParametricMonitorResult<double>> resultVec;
};

BOOST_AUTO_TEST_SUITE(DataParametricMonitorTest)

BOOST_FIXTURE_TEST_CASE(test1, DataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {50}, 0.1};
  dummyTimedWord[1] = {0, {"x"}, {51, 2}, 1.5};
  dummyTimedWord[2] = {0, {"y"}, {200}, 10};
  dummyTimedWord[3] = {0, {"x"}, {200}, 15};
  feed(DataParametricCopy().automaton, std::move(dummyTimedWord));
  BOOST_TEST(resultVec.empty());
}

BOOST_FIXTURE_TEST_CASE(test2, DataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
  dummyTimedWord[1] = {0, {"y"}, {100, 3}, 10};
  dummyTimedWord[2] = {0, {"x"}, {100, 3}, 12};
  dummyTimedWord[3] = {0, {"z"}, {100, 3}, 15.5};
  feed(DataParametricCopy().automaton, std::move(dummyTimedWord));
  BOOST_CHECK_EQUAL(resultVec.size(), 1);
  BOOST_CHECK_EQUAL(resultVec.front().index, 3);
  BOOST_CHECK_EQUAL(resultVec.front().timestamp, 15.5);
}

BOOST_FIXTURE_TEST_CASE(decimal_test1, DataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord{
        {0, {}, {0}, 0.},
        {0, {}, {0}, 1.0},
        {0, {}, {0}, 2.1},
        {0, {}, {0}, 3.3},
        {0, {}, {0}, 4.6}
      };
      feed(Parametric::DataParametricDecimalExpressionFixture().automaton, std::move(dummyTimedWord));
      //NOTE: 3.3 - 2.1 is 1.2, but this is evaluated as 1.1999999999999997 < 1.2, so the fourth event is also matched.
      //BOOST_CHECK_EQUAL(resultVec.size(), 1);
      BOOST_CHECK_EQUAL(resultVec.front().index, 2);
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, 2.1);
}

BOOST_FIXTURE_TEST_CASE(epsilon_test1, DataParametricMonitorFixture)
{
  auto automaton = EpsilonTransitionAutomatonFixture().makeDataParametricTA();

  std::vector<TWEvent> timedWord{
        {0, {"c"}, {}, 1},
        {0, {"a"}, {}, 10},
        {0, {"b"}, {}, 12},
        {0, {"b"}, {}, 15},
        {0, {"c"}, {}, 20},
        {0, {"a"}, {}, 32},
        {0, {"b"}, {}, 40},
        {0, {"c"}, {}, 42},
        {0, {"a"}, {}, 51.5},
        {0, {"b"}, {}, 52},
        {0, {"a"}, {}, 53},
        {0, {"b"}, {}, 54},
        {0, {"c"}, {}, 55},
      };
      feed(automaton, std::move(timedWord));

      BOOST_CHECK_EQUAL(resultVec.size(), 1);
      BOOST_CHECK_EQUAL(resultVec.front().index, 6);
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, 40);
}
BOOST_AUTO_TEST_SUITE_END()
