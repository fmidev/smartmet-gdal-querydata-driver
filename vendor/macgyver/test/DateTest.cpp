// ======================================================================
/*!
 * \file
 * \brief Regression tests for date_time::Date
 * */
// ======================================================================

#include "date_time/Date.h"
#include "Exception.h"
#include "DebugTools.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <locale>

using namespace boost::unit_test;
namespace g = boost::gregorian;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time::Date tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(date_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: construction and extraction");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        if (d1.year() != d2.year() || d1.month() != d2.month() || d1.day() != d2.day())
            num_err++;

        BOOST_CHECK_EQUAL(d1.year(), d2.year());
        BOOST_CHECK_EQUAL(d1.month(), d2.month());
        BOOST_CHECK_EQUAL(d1.day(), d2.day());
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(day_of_year_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: day_of_year");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        if (d1.day_of_year() != d2.day_of_year())
            num_err++;

        BOOST_CHECK_EQUAL(d1.day_of_year(), d2.day_of_year());
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(day_of_week_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: day_of_week");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto wd2 = d2.day_of_week();
        const int i1 = d1.day_of_week().as_number();
        const int i2 = wd2.c_encoding();
        if (i1 != i2)
            num_err++;

        BOOST_CHECK_EQUAL(i1, i2);
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(end_of_month_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: end_of_month");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto e1 = d1.end_of_month();
        const auto e2 = d2.end_of_month();
        if (e1.year() != e2.year() || e1.month() != e2.month() || e1.day() != e2.day())
            num_err++;

        BOOST_CHECK_EQUAL(e1.year(), e2.year());
        BOOST_CHECK_EQUAL(e1.month(), e2.month());
        BOOST_CHECK_EQUAL(e1.day(), e2.day());
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(to_simple_string_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: to_simple_string");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto s1 = g::to_simple_string(d1);
        const auto s2 = d2.to_simple_string();
        if (s1 != s2)
            num_err++;
        BOOST_CHECK_EQUAL(s1, s2);
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(to_iso_string_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: to_iso_string");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto s1 = g::to_iso_string(d1);
        const auto s2 = d2.to_iso_string();
        if (s1 != s2)
            num_err++;
        BOOST_CHECK_EQUAL(s1, s2);
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(modjulian_day)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: modjulian_day");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto m1 = d1.modjulian_day();
        const auto m2 = d2.modjulian_day();
        if (m1 != m2)
            num_err++;
        BOOST_CHECK_EQUAL(m1, m2);
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(julian_day)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: julian_day");
    int num_err = 0;
    g::date d1(2014, 1, 1);
    for (int i = 0; i < 1000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto j1 = d1.julian_day();
        const auto j2 = d2.julian_day();
        if (j1 != j2)
            num_err++;
        BOOST_CHECK_EQUAL(j1, j2);
        BOOST_REQUIRE(num_err < 10);
        d1 += g::days(1);
    }
}

BOOST_AUTO_TEST_CASE(week_number)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: week_number");
    int num_err = 0;
    g::date d1(1970, 1, 1);
    for (int i = 0; i < 100000; i++)
    {
        Fmi::date_time::Date d2(d1.year(), d1.month(), d1.day());
        const auto j1 = d1.week_number();
        const auto j2 = d2.week_number();
        if (j1 != j2) {
            num_err++;
            std::cout << "    "
                      << date::format("%Y-%m-%d", d2.get_impl()) << " " << j1 << " != " << j2
                      << std::endl;;
        }
        d1 += g::days(1);
    }
    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(date_from_string_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: date_from_string");

    std::vector<std::pair<std::string, std::string> > test_data = {
        { "2000-2-29", "2000-Feb-29" }
        , { "2000-02-29", "2000-Feb-29" }
        , { "2000-Feb-29", "2000-Feb-29" }
        , { "2024-01-31", "2024-Jan-31" }
        , { "2014-02-01", "2014-Feb-01" }
    }; // end of test_data

    const auto test_parse =  [](const std::string& s1) -> std::string {
        return SHOW_EXCEPTIONS(Fmi::date_time::date_from_string(s1).to_simple_string());
    };

    for (const auto& item : test_data)
    {
        std::string s1;
        BOOST_CHECK_NO_THROW(s1 = test_parse(item.first));
        if (s1 != "") {
            BOOST_CHECK_EQUAL(s1, item.second);
        }
    }
}

BOOST_AUTO_TEST_CASE(date_from_iso_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: date_from_iso_string");

    std::vector<std::pair<std::string, std::string> > test_data = {
        { "20000229", "2000-Feb-29" }
        , { "20000229", "2000-Feb-29" }
        , { "20240131", "2024-Jan-31" }
        , { "20140201", "2014-Feb-01" }
    }; // end of test_data

    const auto test_parse =  [](const std::string& s1) -> std::string {
        return SHOW_EXCEPTIONS(Fmi::date_time::date_from_string(s1).to_simple_string());
    };

    for (const auto& item : test_data)
    {
        std::string s1;
        BOOST_CHECK_NO_THROW(s1 = test_parse(item.first));
        if (s1 != "") {
            BOOST_CHECK_EQUAL(s1, item.second);
        }
    }
}

BOOST_AUTO_TEST_CASE(date_from_iso_extended_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: date_from_iso_extended_string");

    std::vector<std::pair<std::string, std::string> > test_data = {
        { "2000-2-29", "2000-Feb-29" }
        , { "2000-02-29", "2000-Feb-29" }
        , { "2024-01-31", "2024-Jan-31" }
        , { "2014-02-01", "2014-Feb-01" }
    }; // end of test_data

    const auto test_parse =  [](const std::string& s1) -> std::string {
        return SHOW_EXCEPTIONS(Fmi::date_time::date_from_string(s1).to_simple_string());
    };

    for (const auto& item : test_data)
    {
        std::string s1;
        BOOST_CHECK_NO_THROW(s1 = test_parse(item.first));
        if (s1 != "") {
            BOOST_CHECK_EQUAL(s1, item.second);
        }
    }
}

BOOST_AUTO_TEST_CASE(format_for_locale)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: format_for_locale");
    Fmi::date_time::Date d1(2024, 2, 19);
    const char* fmt = "%Y %B %d";

    // It is possible, that some of the locales are not available in the system
    // Do not treat missing locales as errors, but only report what is done
    std::shared_ptr<std::locale> loc;
    const std::pair<std::string, std::string> test_data[] = {
        { "C", "2024 February 19"}
        , { "en_US.UTF8", "2024 February 19" }
        , { "fi_FI.UTF8", "2024 helmikuu 19" }
        , { "sv_FI.UTF8", "2024 februari 19" }
        , { "lv_LV.UTF8", "2024 februāris 19" }
    }; // end of test_data
    constexpr int num_tests = sizeof(test_data) / sizeof(test_data[0]);
    int num_tested = 0;

    for (const auto& item : test_data)
    {
        try
        {
            loc.reset(new std::locale(item.first));
        }
        catch (...)
        {
            BOOST_TEST_MESSAGE("No locale " + item.first + " : ignoring");
            continue;
        }

        num_tested++;
        const std::string s1 = Fmi::date_time::format_time(*loc, fmt, d1);
        BOOST_CHECK_EQUAL(s1, item.second);
    }

    BOOST_TEST_MESSAGE(std::to_string(num_tested) + " locales of "
        + std::to_string(num_tests) + " tested");
}

BOOST_AUTO_TEST_CASE(serialize_date)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::Date: serialize_date");
    using Fmi::date_time::Date;
    std::vector<Date> test_dates =
        {
            Date(2024, 2, 19),
            Date(Date::NOT_A_DATE_TIME),
            Date(Date::POS_INFINITY),
            Date(Date::NEG_INFINITY)
        };

    for (const auto& d : test_dates)
    {
        std::string archive;
        {
            std::ostringstream os;
            boost::archive::text_oarchive oa(os);
            oa << d;
            archive += os.str();
        }

        //std::cout << "Serialized(" + d.as_string() + "): " << archive << std::endl;

        Date result;
        {
            std::istringstream os(archive);
            boost::archive::text_iarchive ia(os);
            ia >> result;
        }

        BOOST_CHECK_MESSAGE(result == d, "Serialization failed for "
            + d.to_simple_string() + ". Got " + result.to_simple_string());
    }
}
