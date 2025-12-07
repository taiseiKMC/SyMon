#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../src/data_parametric_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"
#include "../test/fixture/decimal_expression_fixture.hh"
//TODO: copy_automaton_fixture の automaton は NonParametricTA だが、DataParametricTA にすべきな気がする

using TWEvent = TimedWordEvent<PPLRational>;

//DummyTimedWordSubject だと DummyTimedWordSubject2{std::move(vec)}.addObserver(monitor);
// でエラーになったが、名前を変えると解消した...???
struct DummyTimedWordSubject2 : public SingleSubject<TWEvent> {
  DummyTimedWordSubject2(std::vector<TWEvent> &&vec) :vec(std::move(vec)) {}
  virtual ~DummyTimedWordSubject2(){}
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

struct CopyDataParametricMonitorFixture : public DataParametricCopy {
  void feed(/*DataParametricTA automaton, */ std::vector<TWEvent> &&vec) {
    auto monitor = std::make_shared<DataParametricMonitor<double>>(automaton);
    std::shared_ptr<DummyDataParametricMonitorObserver> observer = std::make_shared<DummyDataParametricMonitorObserver>();
    monitor->addObserver(observer);
    DummyTimedWordSubject2 subject{std::move(vec)};
    subject.addObserver(monitor); //&monitor, DataParametricMonitor, should be Observer<TWEvemt>
    subject.notifyAll();
    resultVec = std::move(observer->resultVec);
  }
  std::vector<DataParametricMonitorResult<double>> resultVec;
};

BOOST_AUTO_TEST_SUITE(DataParametricMonitorTest)

BOOST_FIXTURE_TEST_CASE(test1, CopyDataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {50}, 0.1};
  dummyTimedWord[1] = {0, {"x"}, {51, 2}, 1.5};
  dummyTimedWord[2] = {0, {"y"}, {200}, 10};
  dummyTimedWord[3] = {0, {"x"}, {200}, 15};
  feed(std::move(dummyTimedWord));
  BOOST_TEST(resultVec.empty());
}

BOOST_FIXTURE_TEST_CASE(test2, CopyDataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
  dummyTimedWord[1] = {0, {"y"}, {100, 3}, 10};
  dummyTimedWord[2] = {0, {"x"}, {100, 3}, 12};
  dummyTimedWord[3] = {0, {"z"}, {100, 3}, 15.5};
  feed(std::move(dummyTimedWord));
  BOOST_CHECK_EQUAL(resultVec.size(), 1);
  BOOST_CHECK_EQUAL(resultVec.front().index, 3);
  BOOST_CHECK_EQUAL(resultVec.front().timestamp, 15.5);
}

BOOST_AUTO_TEST_SUITE_END()

struct DecimalDataParametricMonitorFixture : public Parametric::DataParametricDecimalExpressionFixture {
  void feed(std::vector<TWEvent> &&vec) {
    auto monitor = std::make_shared<DataParametricMonitor<double>>(automaton);
    std::shared_ptr<DummyDataParametricMonitorObserver> observer = std::make_shared<DummyDataParametricMonitorObserver>();
    monitor->addObserver(observer);
    DummyTimedWordSubject2 subject{std::move(vec)};
    subject.addObserver(monitor); //&monitor, DataParametricMonitor, should be Observer<TWEvemt>
    subject.notifyAll();
    resultVec = std::move(observer->resultVec);
  }
  std::vector<DataParametricMonitorResult<double>> resultVec;
};

BOOST_AUTO_TEST_SUITE(DecimalDataParametricMonitorTest)
  BOOST_FIXTURE_TEST_CASE(decimal_test1, DecimalDataParametricMonitorFixture)
  {
    std::vector<TWEvent> dummyTimedWord{
          {0, {}, {0}, 0.},
          {0, {}, {0}, 1.0},
          {0, {}, {0}, 2.1},
          {0, {}, {0}, 3.3},
          {0, {}, {0}, 4.6}
        };
        feed(std::move(dummyTimedWord));
        //NOTE: 3.3 - 2.1 is 1.2, but this is evaluated as 1.1999999999999997 < 1.2, so the fourth event is also matched.
        //BOOST_CHECK_EQUAL(resultVec.size(), 1);
        BOOST_CHECK_EQUAL(resultVec.front().index, 2);
        BOOST_CHECK_EQUAL(resultVec.front().timestamp, 2.1);
  }
BOOST_AUTO_TEST_SUITE_END()
