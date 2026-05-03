// ======================================================================
/*!
 * \file
 * \brief Regression tests for date_time::TimePeriod<>
 * */
// ======================================================================

#include "DateTime.h"
#include "LocalDateTime.h"
#include <boost/test/included/unit_test.hpp>
#include <iostream>

using namespace Fmi::date_time;
using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time::TimePeriod<> tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

namespace
{
    struct TestValues1
    {
        DateTime dt1;
        DateTime dt2;
        DateTime dt3;
        DateTime dt4;

        TestValues1()
            : dt1(Date(2024, 2, 16), TimeDuration(0, 0, 0)),
              dt2(Date(2024, 2, 16), TimeDuration(1, 0, 0)),
              dt3(Date(2024, 2, 16), TimeDuration(2, 0, 0)),
              dt4(Date(2024, 2, 16), TimeDuration(3, 0, 0))
        {
        }
    };
}

BOOST_AUTO_TEST_CASE(zero_length)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: zero length");

    TestValues1 test;

    TimePeriod<DateTime> period(test.dt1, test.dt1);
    // We do not accept zero length periods
    BOOST_CHECK(period.is_null());
}

BOOST_AUTO_TEST_CASE(test_1)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: construction, detection whether value is inside interval");

    TestValues1 test;

    TimePeriod<DateTime> period12(test.dt1, test.dt2);
    BOOST_CHECK_EQUAL(period12.begin(), test.dt1);
    BOOST_CHECK_EQUAL(period12.end(), test.dt2);
    BOOST_CHECK_EQUAL(period12.length(), Hours(1));
    BOOST_CHECK(!period12.is_null());

    TimePeriod<DateTime> bad_period(test.dt2, test.dt1);
    BOOST_CHECK(bad_period.is_null());

    BOOST_CHECK(period12.contains(test.dt1));
    BOOST_CHECK(period12.contains(test.dt1 + Minutes(30)));
    BOOST_CHECK(period12.contains(test.dt2));
    BOOST_CHECK(!period12.contains(test.dt3));
    BOOST_CHECK(!period12.contains(test.dt1 - TimeDuration(1, 0, 0)));
    BOOST_CHECK(!period12.contains(test.dt2 + Minutes(30)));
}

BOOST_AUTO_TEST_CASE(local_time_interval_test_1)
{
    BOOST_TEST_MESSAGE("TimePeriod<LocalDateTime>: test with different time zones");

    TestValues1 test;
    TimeZonePtr tz1("Europe/Helsinki");
    TimeZonePtr tz2("UTC");
    TimeZonePtr tz3("Asia/Katmandu");

    LocalDateTime ldt1(test.dt1, tz1);
    LocalDateTime ldt2 = LocalDateTime(test.dt2, tz1).to_tz(tz2);

    TimePeriod<LocalDateTime> period12(ldt1, ldt2);
    BOOST_CHECK(!period12.is_null());
    BOOST_CHECK_EQUAL(period12.begin(), ldt1);
    BOOST_CHECK_EQUAL(period12.end(), ldt2);
    BOOST_CHECK_EQUAL(period12.length(), Hours(1));

    BOOST_CHECK(period12.contains((ldt1 + Minutes(30)).to_tz(tz3)));
}

BOOST_AUTO_TEST_CASE(shift_interval)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: interval shift");
    TestValues1 test;

    TimePeriod<DateTime> period(test.dt1, test.dt2);
    period.shift(Minutes(30));
    BOOST_CHECK_EQUAL(period.begin(), test.dt1 + Minutes(30));
    BOOST_CHECK_EQUAL(period.end(), test.dt2 + Minutes(30));
}

BOOST_AUTO_TEST_CASE(expand_interval)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: interval expand");
    TestValues1 test;

    TimePeriod<DateTime> period(test.dt1, test.dt2);
    period.expand(Minutes(30));
    BOOST_CHECK_EQUAL(period.begin(), test.dt1 - Minutes(30));
    BOOST_CHECK_EQUAL(period.end(), test.dt2 + Minutes(30));
}

BOOST_AUTO_TEST_CASE(contains_period)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: contains period");
    TestValues1 test;

    TimePeriod<DateTime> period1(test.dt1, test.dt2);
    TimePeriod<DateTime> period2(test.dt1 + Minutes(10), test.dt2 - Minutes(10));
    TimePeriod<DateTime> period3(test.dt1 + Minutes(90), test.dt2 + Minutes(120));

    BOOST_CHECK(period1.contains(period2));
    BOOST_CHECK(!period2.contains(period1));
    BOOST_CHECK(!period1.contains(period3));
}

BOOST_AUTO_TEST_CASE(intersects)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: intersects");
    TestValues1 test;

    TimePeriod<DateTime> period1(test.dt1, test.dt2);
    TimePeriod<DateTime> period2(test.dt1 + Minutes(10), test.dt2 - Minutes(10));
    TimePeriod<DateTime> period3(test.dt1 + Minutes(90), test.dt2 + Minutes(120));

    BOOST_CHECK(period1.intersects(period2));
    BOOST_CHECK(period2.intersects(period1));
    BOOST_CHECK(!period1.intersects(period3));
}

BOOST_AUTO_TEST_CASE(inf_1)
{
    BOOST_TEST_MESSAGE("TimePeriod<DateTime>: infinite interval limits");
    DateTime neg_inf = DateTime::NEG_INFINITY;
    DateTime pos_inf = DateTime::POS_INFINITY;
    DateTime dt1(Date(2024, 2, 16), TimeDuration(0, 0, 0));
    DateTime dt2(Date(2024, 3, 19), TimeDuration(1, 0, 0));

    TimePeriod<DateTime> period1(neg_inf, dt2);
    BOOST_CHECK(not period1.is_null());
    BOOST_CHECK(period1.contains(neg_inf));
    BOOST_CHECK(!period1.contains(pos_inf));
    BOOST_CHECK(period1.contains(dt1));

    TimePeriod<DateTime> period2(dt1, dt2);
    TimePeriod<DateTime> period3(dt1, pos_inf);
    BOOST_CHECK(not period2.is_null());
    BOOST_CHECK(period1.contains(period2));
    BOOST_CHECK(!period3.contains(period1));
    BOOST_CHECK_EQUAL(period1.intersection(period3), period2);
}
