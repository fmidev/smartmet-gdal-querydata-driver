// ======================================================================
/*!
 * \file
 * \brief Regression tests for date_time::LocalDateTime
 * */
// ======================================================================

#include "date_time/LocalDateTime.h"
#include "date_time/TimeZonePtr.h"
#include "Exception.h"
#include "DebugTools.h"
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <random>

#include <boost/date_time/local_time/local_time.hpp>

using namespace boost::unit_test;
using namespace std::string_literals;
namespace g = boost::gregorian;

#define DEBUG(x) SHOW_EXCEPTIONS(x)
//#define DEBUG(x) x

static const char* default_regions = "/usr/share/smartmet/timezones/date_time_zonespec.csv";

namespace g = boost::gregorian;
namespace pt = boost::posix_time;
namespace lt = boost::local_time;

namespace
{
    boost::local_time::tz_database itsRegions;

    lt::time_zone_ptr get_boost_tz(const std::string& name)
    {
        auto ptr = itsRegions.time_zone_from_region(name);
        if (!ptr)
            throw Fmi::Exception(BCP, "Unknown timezone").addParameter("Name", name);
        return ptr;
    }
}

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time::LocalDateTime tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  try
  {
    itsRegions.load_from_file(default_regions);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Failed to load time zone database: " << e.what() << std::endl;
    throw;
  }
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(construct_and_extract_1)
{
    using namespace Fmi::date_time;
    BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime: construction and extraction (1)");

    TimeZonePtr tz1("Europe/Helsinki");
    TimeZonePtr tz2("Etc/UTC");
    DateTime dtm1 = SHOW_EXCEPTIONS(time_from_string("2014-02-01 12:34:56.789"));
    DateTime dtm2 = SHOW_EXCEPTIONS(time_from_string("2014-05-01 12:34:56.789"));
    LocalDateTime ldt1(dtm1.date(), dtm1.time_of_day(), tz1);

    BOOST_CHECK_EQUAL(ldt1.local_time().date().year(), 2014);
    BOOST_CHECK_EQUAL(ldt1.local_time().date().month(), 2);
    BOOST_CHECK_EQUAL(ldt1.local_time().date().day(), 1);
    BOOST_CHECK_EQUAL(ldt1.local_time().time_of_day().hours(), 12);
    BOOST_CHECK_EQUAL(ldt1.local_time().time_of_day().minutes(), 34);
    BOOST_CHECK_EQUAL(ldt1.local_time().time_of_day().seconds(), 56);
    BOOST_CHECK_EQUAL(ldt1.local_time().time_of_day().fractional_seconds(), 789000);

    BOOST_CHECK(!ldt1.dst_on());
    BOOST_CHECK_EQUAL(ldt1.offset(), Hours(2));

    BOOST_CHECK_EQUAL(ldt1.utc_time(), dtm1 - Fmi::date_time::hours(2));

    LocalDateTime ldt1b(ldt1.utc_time(), tz1);
    BOOST_CHECK_EQUAL(ldt1b, ldt1);


    //std::cout << ldt1.get_sys_info() << std::endl;

    LocalDateTime ldt2(dtm2.date(), dtm2.time_of_day(), tz1);

    BOOST_CHECK_EQUAL(ldt2.local_time().date().year(), 2014);
    BOOST_CHECK_EQUAL(ldt2.local_time().date().month(), 5);
    BOOST_CHECK_EQUAL(ldt2.local_time().date().day(), 1);
    BOOST_CHECK_EQUAL(ldt2.local_time().time_of_day().hours(), 12);
    BOOST_CHECK_EQUAL(ldt2.local_time().time_of_day().minutes(), 34);
    BOOST_CHECK_EQUAL(ldt2.local_time().time_of_day().seconds(), 56);
    BOOST_CHECK_EQUAL(ldt2.local_time().time_of_day().fractional_seconds(), 789000);

    BOOST_CHECK(ldt2.dst_on());
    BOOST_CHECK_EQUAL(ldt2.offset(), Hours(3));
    BOOST_CHECK_EQUAL(ldt2.utc_time(), dtm2 - Fmi::date_time::hours(3));

    //std::cout << ldt2.get_sys_info() << std::endl;

    LocalDateTime ldt3(dtm2, tz2);

    BOOST_CHECK_EQUAL(ldt3.local_time().date().year(), 2014);
    BOOST_CHECK_EQUAL(ldt3.local_time().date().month(), 5);
    BOOST_CHECK_EQUAL(ldt3.local_time().date().day(), 1);
    BOOST_CHECK_EQUAL(ldt3.local_time().time_of_day().hours(), 12);
    BOOST_CHECK_EQUAL(ldt3.local_time().time_of_day().minutes(), 34);
    BOOST_CHECK_EQUAL(ldt3.local_time().time_of_day().seconds(), 56);
    BOOST_CHECK_EQUAL(ldt3.local_time().time_of_day().fractional_seconds(), 789000);

    BOOST_CHECK(!ldt3.dst_on());
    BOOST_CHECK_EQUAL(ldt3.offset(), Hours(0));
    BOOST_CHECK_EQUAL(ldt3.utc_time(), ldt3.local_time());

    //std::cout << ldt3.get_sys_info() << std::endl;
}

BOOST_AUTO_TEST_CASE(dst_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime: handling begin of DST)");
    Fmi::date_time::TimeZonePtr tz("Europe/Helsinki");

    Fmi::date_time::Date date(2023, 3, 26);
    Fmi::date_time::TimeDuration time(2, 59, 59);
    Fmi::date_time::DateTime dt1(date, time);                     // 02:59:59
    Fmi::date_time::DateTime dt2 = DEBUG(dt1 + Fmi::date_time::seconds(2));         // 03:00:01
    Fmi::date_time::DateTime dt3 = DEBUG(dt2 + Fmi::date_time::hours(1));           // 04:00:01

    // 1 second before begin of summertime
    Fmi::date_time::LocalDateTime ldt1(date, time, tz);
    BOOST_CHECK_EQUAL(ldt1.is_special(), false);
    BOOST_CHECK_EQUAL(ldt1.local_time(), dt1);
    BOOST_CHECK_EQUAL(ldt1.dst_on(), false);

    // 1 second in gap (begin of summertime) -> invalid local time
    Fmi::date_time::LocalDateTime ldt2(dt2.date(), dt2.time_of_day(), tz,
        Fmi::date_time::LocalDateTime::NOT_DATE_TIME_ON_ERROR,
        Fmi::date_time::LocalDateTime::Choose::NONE);
    BOOST_CHECK_EQUAL(ldt2.is_special(), true);

    Fmi::date_time::LocalDateTime ldt3(dt3.date(), dt3.time_of_day(), tz);
    BOOST_CHECK_EQUAL(ldt3.is_special(), false);
    BOOST_CHECK_EQUAL(ldt3.local_time(), dt3);
    BOOST_CHECK_EQUAL(ldt3.dst_on(), true);
}

BOOST_AUTO_TEST_CASE(dst_test_2)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime: handling end of DST");
    Fmi::date_time::TimeZonePtr tz("Europe/Helsinki");

    Fmi::date_time::Date date(2023, 10, 29);
    Fmi::date_time::TimeDuration time(2, 59, 59);
    Fmi::date_time::DateTime dt1(date, time);                     // 02:59:59
    Fmi::date_time::DateTime dt2 = dt1 + Fmi::date_time::seconds(2);         // 03:00:01
    Fmi::date_time::DateTime dt3 = dt2 + Fmi::date_time::hours(1);           // 04:00:01

    // 1 second before end of summertime
    Fmi::date_time::LocalDateTime ldt1(dt1.date(), dt1.time_of_day(), tz);
    Fmi::date_time::DateTime dt1_utc = ldt1.utc_time();
    BOOST_CHECK_EQUAL(dt1_utc, dt1 - Fmi::date_time::hours(3));
    Fmi::date_time::LocalDateTime ldt1_b = ldt1.to_tz(tz);
    BOOST_CHECK_EQUAL(ldt1_b, ldt1);

    Fmi::date_time::LocalDateTime ldt3(dt3.date(), dt3.time_of_day(), tz);
    Fmi::date_time::DateTime dt3_utc = ldt3.utc_time();
    BOOST_CHECK_EQUAL(dt3_utc, dt3 - Fmi::date_time::hours(2));
    Fmi::date_time::LocalDateTime ldt3_b = ldt3.to_tz(tz);
    BOOST_CHECK_EQUAL(ldt3_b, ldt3);
}

BOOST_AUTO_TEST_CASE(tz_convert_1)
{
  BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime: conversion between timezones");
  Fmi::date_time::TimeZonePtr tz1("Europe/Helsinki");
  Fmi::date_time::TimeZonePtr tz2("Europe/Riga");
  Fmi::date_time::TimeZonePtr tz3("UTC");
  Fmi::date_time::TimeZonePtr tz4("Asia/Kathmandu");

  Fmi::date_time::DateTime dt1(Fmi::date_time::Date(1961, 12, 31), Fmi::date_time::TimeDuration(22, 30, 00));
  Fmi::date_time::DateTime dt2(Fmi::date_time::Date(1961, 12, 31), Fmi::date_time::TimeDuration(23, 30, 00));
  Fmi::date_time::DateTime dt3(Fmi::date_time::Date(1961, 12, 31), Fmi::date_time::TimeDuration(20, 30, 00));
  Fmi::date_time::DateTime dt4(Fmi::date_time::Date(1962,  1,  1), Fmi::date_time::TimeDuration( 2, 00, 00));

  Fmi::date_time::LocalDateTime ldt2(dt2.date(), dt2.time_of_day(), tz2);
  BOOST_REQUIRE(!ldt2.is_special());
  Fmi::date_time::LocalDateTime ldt1 = ldt2.to_tz(tz1);
  Fmi::date_time::LocalDateTime ldt3 = ldt2.to_tz(tz3);
  Fmi::date_time::LocalDateTime ldt4 = ldt2.to_tz(tz4);

  BOOST_CHECK_EQUAL(ldt2.is_special(), false);

  BOOST_CHECK_EQUAL(Fmi::date_time::to_iso_extended_string(ldt1), "1961-12-31T22:30:00+02:00"s);
  BOOST_CHECK_EQUAL(Fmi::date_time::to_iso_extended_string(ldt2), "1961-12-31T23:30:00+03:00"s);
  BOOST_CHECK_EQUAL(Fmi::date_time::to_iso_extended_string(ldt3), "1961-12-31T20:30:00Z"s);
  BOOST_CHECK_EQUAL(Fmi::date_time::to_iso_extended_string(ldt4), "1962-01-01T02:00:00+05:30"s);
}

BOOST_AUTO_TEST_CASE(advance_1)
{
  BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime::advance(): begin of DST");

  Fmi::date_time::TimeZonePtr tz("Europe/Helsinki");

  Fmi::date_time::Date date(2023, 3, 26);
  Fmi::date_time::TimeDuration time(2, 59, 59);
  Fmi::date_time::DateTime dt1(date, time);                     // 02:59:59
  Fmi::date_time::LocalDateTime ldt0(dt1.date(), dt1.time_of_day(), tz);

  Fmi::date_time::LocalDateTime ldt1(ldt0);
  ldt1.advance(Fmi::date_time::seconds(2));

  // Jump over gap due to DST begin
  BOOST_CHECK_EQUAL(ldt1.is_special(), false);
  BOOST_CHECK_EQUAL(ldt1.local_time(), dt1 + Fmi::date_time::seconds(3602));

  Fmi::date_time::TimeDuration diff = ldt1 - ldt0;
  BOOST_CHECK_EQUAL(diff, Fmi::date_time::seconds(2));

  ldt1.advance(-Fmi::date_time::seconds(3));
  BOOST_CHECK_EQUAL(ldt1.is_special(), false);
  BOOST_CHECK_EQUAL(ldt1.local_time(), dt1 - Fmi::date_time::seconds(1));

  // Jump forward exactly to the begin of the DST gap
  Fmi::date_time::LocalDateTime ldt2(ldt0);
  SHOW_EXCEPTIONS(ldt2.advance(Fmi::date_time::seconds(1)));
  BOOST_CHECK_EQUAL(ldt2.is_special(), false);
  BOOST_CHECK_EQUAL(ldt2.local_time(), dt1 + Fmi::date_time::seconds(3601));

  // Jump backward exactly to the end of the DST gap
  Fmi::date_time::LocalDateTime ldt3(dt1.date(), dt1.time_of_day() + Fmi::date_time::seconds(3602), tz);
  SHOW_EXCEPTIONS(ldt3.advance(-Fmi::date_time::seconds(1)));
  BOOST_CHECK_EQUAL(ldt3.is_special(), false);
  // NOTE: we prefer DST instead of ambiguity
  BOOST_CHECK_EQUAL(ldt3.local_time(), dt1 + Fmi::date_time::seconds(3601));
}

BOOST_AUTO_TEST_CASE(advance_comparision_with_boost_1)
{
    using namespace Fmi::date_time;
    namespace pt = boost::posix_time;
    namespace lt = boost::local_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: advance and comparision with boost (1)");

    const auto s_dt1 = "2024-Jan-01 00:00:01"s;
    const auto s_dt2 = "2025-Jan-01 00:00:01"s;
    const auto s_inc = "00:37:34.654789"s;

    TimeZonePtr tz1("Europe/Helsinki");
    lt::time_zone_ptr b_tz1 = get_boost_tz(tz1->name());

    const auto inc1 = duration_from_string(s_inc);
    const auto b_inc = boost::posix_time::duration_from_string(s_inc);

    DateTime dt1 = time_from_string(s_dt1);
    pt::ptime b_pt1 = pt::time_from_string(s_dt1);

    LocalDateTime ldt1(time_from_string(s_dt1), tz1);
    LocalDateTime ldt2(time_from_string(s_dt2), tz1);
    LocalDateTime ldt_start = ldt1;

    lt::local_date_time b_ldt1(b_pt1, b_tz1);

    while (ldt1 < ldt2)
    {
        ldt1.advance(inc1);
        b_ldt1 += b_inc;
        const std::string s1 = ldt1.local_time().to_simple_string();
        const std::string s2 = pt::to_simple_string(b_ldt1.local_time());
        //std::cout << s1 << " == " << s2 << std::endl;
        BOOST_REQUIRE_EQUAL(s1, s2);
    }

    while (ldt1 > ldt_start)
    {
        ldt1.advance(-inc1);
        b_ldt1 -= b_inc;
        const std::string s1 = ldt1.local_time().to_simple_string();
        const std::string s2 = pt::to_simple_string(b_ldt1.local_time());
        //std::cout << s1 << " == " << s2 << std::endl;
        BOOST_REQUIRE_EQUAL(s1, s2);
    }
}

BOOST_AUTO_TEST_CASE(comparison_with_boost_2)
{
    namespace g = boost::gregorian;
    namespace pt = boost::posix_time;
    namespace lt = boost::local_time;

    const auto s_tz = "Europe/Helsinki"s;
    const auto s_dt1 = "2024-Feb-20 08:48:57"s;

    Fmi::date_time::TimeZonePtr tz1(s_tz);
    Fmi::date_time::DateTime dt1 = Fmi::date_time::time_from_string(s_dt1);
    Fmi::date_time::LocalDateTime ldt1(dt1, tz1);

    const auto b_tz = get_boost_tz(s_tz);
    const auto b_pt1 = pt::time_from_string(s_dt1);
    const auto b_ldt1 = lt::local_date_time(b_pt1, b_tz);

    std::ostringstream str1, str2;
    str1 << ldt1;
    str2 << b_ldt1;
    BOOST_CHECK_EQUAL(str1.str(), str2.str());
    BOOST_CHECK_EQUAL(ldt1.local_time().to_simple_string(), pt::to_simple_string(b_ldt1.local_time()));

    BOOST_CHECK_EQUAL(
        ldt1.local_time().to_iso_string(),
        pt::to_iso_string(b_ldt1.local_time()));

    BOOST_CHECK_EQUAL(
        ldt1.utc_time().to_iso_string(),
        pt::to_iso_string(b_ldt1.utc_time()));

    BOOST_CHECK_EQUAL(
        ldt1.date().to_iso_string(),
        g::to_iso_string(b_ldt1.date()));

    BOOST_CHECK_EQUAL(
        ldt1.time_of_day().to_iso_string(),
        pt::to_iso_string(b_ldt1.time_of_day()));

    BOOST_CHECK_EQUAL(
        ldt1.local_time().to_simple_string(),
        "2024-Feb-20 10:48:57"s);

    BOOST_CHECK_EQUAL(
        ldt1.utc_time().to_simple_string(),
        s_dt1);

    Fmi::date_time::LocalDateTime ldt2(
        dt1.date(),
        dt1.time_of_day() + Fmi::date_time::hours(2),
        tz1);

    lt::local_date_time b_ldt2(
        b_pt1.date(),
        b_pt1.time_of_day() + pt::hours(2),
        b_tz,
        lt::local_date_time::NOT_DATE_TIME_ON_ERROR);

    BOOST_CHECK_EQUAL(ldt1, ldt2);

    BOOST_CHECK_EQUAL(b_ldt1, b_ldt2);
}

BOOST_AUTO_TEST_CASE(comparison_with_boost_3)
{
    namespace g = boost::gregorian;
    namespace pt = boost::posix_time;
    namespace lt = boost::local_time;

    const auto s_dt1 = "2024-Feb-20 08:48:57"s;

    Fmi::date_time::DateTime dt1 = Fmi::date_time::time_from_string(s_dt1);
    Fmi::date_time::LocalDateTime ldt1(dt1);

    const auto b_pt1 = pt::time_from_string(s_dt1);
    const auto b_ldt1 = lt::local_date_time(b_pt1, lt::time_zone_ptr());

    std::ostringstream tmp;
    tmp << b_ldt1;

    BOOST_CHECK_EQUAL(
        Fmi::date_time::to_simple_string(ldt1),
        tmp.str());
}

BOOST_AUTO_TEST_CASE(test_make_date_1)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: make_date (DST: EEST -> EET transition 2024 Oct 27)");

    Fmi::date_time::Date date(2024, 10, 27);
    TimeZonePtr tz1("Europe/Helsinki");

    int num_err = 0;
    for (int i = 0; i < 33; i++)
    {
        Fmi::date_time::LocalDateTime ldt1;
        const auto time = Fmi::date_time::Minutes(10*i);
        ldt1 = SHOW_EXCEPTIONS(make_time(date, time, tz1));
        const std::string exp_abbrev = time.hours() < 4 ? "EEST" : "EET";
        if (ldt1.abbrev() != exp_abbrev)
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << ldt1.abbrev() << " != " << exp_abbrev
                          << " at " << Fmi::date_time::to_iso_extended_string(ldt1)
                          << std::endl;
            }
            num_err++;
        }
        const Fmi::date_time::TimeDuration td_actual = ldt1.local_time().time_of_day();
        if (time != td_actual)
        {
            std::cout << "INFO: Local time "
                << Fmi::date_time::to_iso_extended_string(td_actual)
                << " is not the same as original one: "
                << Fmi::date_time::to_iso_extended_string(time)
                << std::endl;
        }
    }
    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(test_make_date_2)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: make_date (DST: EET -> EEST transition 2024 Mar 31)");

    Fmi::date_time::Date date(2024, 3, 31);
    TimeZonePtr tz1("Europe/Helsinki");

    int num_err = 0;
    for (int i = 0; i < 33; i++)
    {
        Fmi::date_time::LocalDateTime ldt1;
        const auto time = Fmi::date_time::Minutes(10*i);
        ldt1 = SHOW_EXCEPTIONS(make_time(date, time, tz1));
        const std::string exp_abbrev = time.hours() < 3 ? "EET" : "EEST";
        if (ldt1.abbrev() != exp_abbrev)
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << ldt1.abbrev() << " != " << exp_abbrev
                          << " at " << Fmi::date_time::to_iso_extended_string(ldt1)
                          << std::endl;
            }
            num_err++;
        }
        const Fmi::date_time::TimeDuration td_actual = ldt1.local_time().time_of_day();
        if (time != td_actual)
        {
            std::cout << "INFO: Local time "
                << Fmi::date_time::to_iso_extended_string(td_actual)
                << " is not the same as original one: "
                << Fmi::date_time::to_iso_extended_string(time)
                << std::endl;
        }
    }
    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(to_wintertime_1)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: to_wintertime (DST: EEST -> EET transition 2024 Oct 27) part 1/2");

    int num_err = 0;
    Fmi::date_time::Date date(2024, 10, 27);
    TimeZonePtr tz_utc("UTC");
    TimeZonePtr tz1("Europe/Helsinki");

    for (int seconds = -10; seconds < 10810; seconds += 10)
    {
        int expected_seconds = seconds + 3600 * (seconds < 3600 ? 3 : 2);
        Fmi::date_time::DateTime dt1 = Fmi::date_time::DateTime(date, Fmi::date_time::Seconds(seconds));
        Fmi::date_time::LocalDateTime utc(dt1, tz_utc,
            Fmi::date_time::LocalDateTime::NOT_DATE_TIME_ON_ERROR);
        Fmi::date_time::LocalDateTime ldt1(dt1, tz1,
            Fmi::date_time::LocalDateTime::NOT_DATE_TIME_ON_ERROR);

        if (ldt1.is_special())
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << dt1 << " --> " << ldt1 << std::endl;
            }
            num_err++;
            continue;
        }

        int actual_seconds = ldt1.local_time().time_of_day().total_seconds();
        //std::cout << dt1 << " --> " << ldt1 << "  ("
        //     << seconds << " : " << expected_seconds << " : " << actual_seconds << ")"
        //     << std::endl;
        if (actual_seconds != expected_seconds)
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << dt1 << " --> " << ldt1 << std::endl;
            }
            num_err++;
        }

        Fmi::date_time::LocalDateTime ldt2;
        try
        {
            ldt2 = utc.to_tz(tz1);
            if (ldt2.is_special() or (ldt2 != ldt1))
            {
                if (num_err < 10)
                {
                    std::cout << "Error: " << utc << " --> " << ldt2 << " <=> " << ldt1 << std::endl;
                }
                num_err++;
                continue;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error: " << utc << " --> " << ldt2 << " : exception: " << e.what() << std::endl;
            num_err++;
            continue;
        }
    }

    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(to_wintertime_2)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: to_wintertime (DST: EEST -> EET transition 2024 Oct 27) part 2/2");

    int num_err = 0;
    Fmi::date_time::Date date(2024, 10, 27);
    TimeZonePtr tz1("Europe/Helsinki");

    const int begin_seconds = Fmi::date_time::hours(3).total_seconds() - 10;
    const int end_seconds = Fmi::date_time::hours(4).total_seconds() + 10;
    for (int seconds = begin_seconds; seconds <= end_seconds; seconds += 10)
    {
        int expected_seconds = seconds;
        Fmi::date_time::TimeDuration time = Fmi::date_time::seconds(seconds);
        Fmi::date_time::LocalDateTime ldt1(
            date,
            time,
            tz1,
            Fmi::date_time::LocalDateTime::NOT_DATE_TIME_ON_ERROR,
            Fmi::date_time::LocalDateTime::Choose::AUTO);

        if (ldt1.is_special())
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << time << " --> " << ldt1 << std::endl;
            }
            num_err++;
            continue;
        }

        int actual_seconds = ldt1.local_time().time_of_day().total_seconds();
        //std::cout << Fmi::date_time::seconds(seconds)
        //     << seconds << " : " << expected_seconds << " : " << actual_seconds
        //     << std::endl;
        if (actual_seconds != expected_seconds)
        {
            if (num_err < 10)
            {
                std::cout << "Error: " << time << " -> " << ldt1 << std::endl;
            }
            num_err++;
        }
    }

    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(compare_1)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: compare (same time zone)");

    Fmi::date_time::Date date(2024, 10, 27);
    TimeZonePtr tz1("Europe/Helsinki");

    Fmi::date_time::LocalDateTime ldt1(date, Fmi::date_time::seconds(3600), tz1);

    BOOST_CHECK_EQUAL(ldt1, ldt1);
    BOOST_CHECK_LT(ldt1, ldt1 + Fmi::date_time::seconds(1));
    BOOST_CHECK_GT(ldt1 + Fmi::date_time::seconds(1), ldt1);
}

BOOST_AUTO_TEST_CASE(compare_2)
{
    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: compare (different time zones)");

    Fmi::date_time::Date date(2024, 10, 27);
    TimeZonePtr tz1("Europe/Helsinki");
    TimeZonePtr tz2("Europe/Stockholm");

    Fmi::date_time::LocalDateTime ldt1(date, Fmi::date_time::seconds(7200), tz1);
    Fmi::date_time::LocalDateTime ldt2(date, Fmi::date_time::seconds(3600), tz2);

    BOOST_CHECK_EQUAL(ldt1, ldt2);
    BOOST_CHECK_LT(ldt1, ldt2 + Fmi::date_time::seconds(1));
    BOOST_CHECK_GT(ldt2 + Fmi::date_time::seconds(1), ldt1);
    BOOST_CHECK_LT(ldt2, ldt1 + Fmi::date_time::seconds(1));
    BOOST_CHECK_GT(ldt1 + Fmi::date_time::seconds(1), ldt2);
}

BOOST_AUTO_TEST_CASE(timing_1)
{
    constexpr const std::size_t size = 1000000;

    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::LocalDateTime: timing test for effectiveness");
    BOOST_CHECK(true);  // We are interested how long it takes

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist(1, 1000);

    TimeZonePtr tz1("Europe/Helsinki");
    Date date(2024, 10, 27);
    TimeDuration time(2, 59, 59);
    DateTime dt1(date, time);                     // 02:59:59
    LocalDateTime ldt1(dt1.date(), dt1.time_of_day(), tz1);

    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<LocalDateTime> v1;
    for (std::size_t i = 0; i < size; ++i)
    {
        ldt1.advance(seconds(1));
        v1.push_back(ldt1);
    }
    std::shuffle(v1.begin(), v1.end(), rng);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    BOOST_TEST_MESSAGE("Test data created in " + std::to_string(duration) + " ms");

    t1 = std::chrono::high_resolution_clock::now();
    std::map<LocalDateTime, int> m1;
    std::transform(v1.begin(), v1.end(), std::inserter(m1, m1.end()),
        [](const LocalDateTime& ldt) { return std::make_pair(ldt, 0); });
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    BOOST_TEST_MESSAGE("Copied to std::map in " + std::to_string(duration) + " ms");

    std::shuffle(v1.begin(), v1.end(), rng);
    for (int i = 0; i < 10; i++)
    {
        t1 = std::chrono::high_resolution_clock::now();
        for (const auto& t : v1)
        {
            m1.at(t)++;
        }
        t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        BOOST_TEST_MESSAGE("Map access time: " + std::to_string(duration) + " ms");
    }
}

BOOST_AUTO_TEST_CASE(utc_zone_detection)
{
    using namespace Fmi::date_time;
    TimeZonePtr tz1("Europe/Helsinki");
    TimeZonePtr tz2("Etc/UTC");
    TimeZonePtr tz3("UTC");
    TimeZonePtr tz4("GMT");
    TimeZonePtr tz5("Europe/London");

    BOOST_CHECK(!tz1.is_utc());
    BOOST_CHECK(tz2.is_utc());
    BOOST_CHECK(tz3.is_utc());
    BOOST_CHECK(tz4.is_utc());
    BOOST_CHECK(!tz5.is_utc());
}
