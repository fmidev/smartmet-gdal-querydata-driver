// ======================================================================
/*!
 * \file
 * \brief Regression tests for date_time::DateTime
 * */
// ======================================================================

#include "date_time/DateTime.h"
#include "DebugTools.h"
#include "Exception.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <locale>
#include <random>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time::DateTime tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

BOOST_AUTO_TEST_CASE(construct_and_extract_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: construction and extraction (1)");
    Fmi::date_time::TimeDuration td1(12, 34, 45, 123456);
    Fmi::date_time::Date d1(2014, 1, 24);
    Fmi::date_time::DateTime dt1(d1, td1);
    BOOST_CHECK_EQUAL(dt1.date(), d1);
    BOOST_CHECK_EQUAL(dt1.time_of_day(), td1);
}

BOOST_AUTO_TEST_CASE(construct_and_extract_test_2)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: construction and extraction (2)");
    Fmi::date_time::TimeDuration td1(12, 34, 45, 123456);
    Fmi::date_time::Date d1(1723, 1, 24);
    Fmi::date_time::DateTime dt1(d1, td1);
    Fmi::date_time::Date d2 = dt1.date();
    Fmi::date_time::TimeDuration td2 = dt1.time_of_day();
    BOOST_CHECK_EQUAL(d1, d2);
    BOOST_CHECK_EQUAL(td1, td2);
}

BOOST_AUTO_TEST_CASE(to_simple_string_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: to_simple_string");
    Fmi::date_time::TimeDuration td1(12, 34, 45);
    Fmi::date_time::Date d1(2024, 1, 24);
    Fmi::date_time::DateTime dt1(d1, td1);
    std::string str = Fmi::date_time::to_simple_string(dt1);
    BOOST_CHECK_EQUAL(str, "2024-Jan-24 12:34:45");

    dt1 += Fmi::date_time::Microseconds(123456);
    str = Fmi::date_time::to_simple_string(dt1);
    BOOST_CHECK_EQUAL(str, "2024-Jan-24 12:34:45.123456");
}

BOOST_AUTO_TEST_CASE(to_iso_string_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: to_iso_string");
    Fmi::date_time::TimeDuration td1(12, 34, 45);
    Fmi::date_time::Date d1(2024, 1, 24);
    Fmi::date_time::DateTime dt1(d1, td1);
    std::string str = Fmi::date_time::to_iso_string(dt1);
    BOOST_CHECK_EQUAL(str, "20240124T123445");

    dt1 += Fmi::date_time::Microseconds(123456);
    str = Fmi::date_time::to_iso_string(dt1);
    BOOST_CHECK_EQUAL(str, "20240124T123445.123456");
}

BOOST_AUTO_TEST_CASE(to_iso_extended_string_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: to_iso_extended_string");
    Fmi::date_time::TimeDuration td1(12, 34, 45);
    Fmi::date_time::Date d1(2024, 1, 24);
    Fmi::date_time::DateTime dt1(d1, td1);
    std::string str = Fmi::date_time::to_iso_extended_string(dt1);
    BOOST_CHECK_EQUAL(str, "2024-01-24T12:34:45");

    dt1 += Fmi::date_time::Microseconds(123456);
    str = Fmi::date_time::to_iso_extended_string(dt1);
    BOOST_CHECK_EQUAL(str, "2024-01-24T12:34:45.123456");
}

BOOST_AUTO_TEST_CASE(to_tm)
{
    using namespace Fmi::date_time;
    BOOST_TEST_MESSAGE("Fmi::date_time: to_tm");
    const DateTime dt1(Date(2005, 1, 1), TimeDuration(1, 2, 3));
    const auto test = dt1.as_tm();
    BOOST_CHECK_EQUAL(int(test.tm_year), 105);
    BOOST_CHECK_EQUAL(int(test.tm_mon), 0);
    BOOST_CHECK_EQUAL(int(test.tm_mday), 1);
    BOOST_CHECK_EQUAL(int(test.tm_wday), 6); // Saturday
    BOOST_CHECK_EQUAL(int(test.tm_yday), 0);
    BOOST_CHECK_EQUAL(int(test.tm_hour), 1);
    BOOST_CHECK_EQUAL(int(test.tm_min), 2);
    BOOST_CHECK_EQUAL(int(test.tm_sec), 3);
    BOOST_CHECK_EQUAL(int(test.tm_isdst), -1);
}

BOOST_AUTO_TEST_CASE(to_time_t)
{
    using namespace Fmi::date_time;
    BOOST_TEST_MESSAGE("Fmi::date_time: as_time_t() and from_time_t()");
    const DateTime dt1(Date(2005, 1, 1), TimeDuration(1, 2, 3));
    const auto test = dt1.as_time_t();
    BOOST_CHECK_EQUAL(test, 1104541323);

    const DateTime dt2 = Fmi::date_time::from_time_t(test);
}

namespace
{
    Fmi::date_time::DateTime make_datetime(int year, int month, int day,
        int hours, int minutes, int seconds, int mks = 0)
    {
        Fmi::date_time::Date d(year, month, day);
        Fmi::date_time::TimeDuration td(hours, minutes, seconds, mks);
        return Fmi::date_time::DateTime(d, td);
    }
}

BOOST_AUTO_TEST_CASE(time_from_string_test_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime::from_string");

    const std::string fmt = "%Y-%m-%d %H:%M:%S";

    const std::pair<std::string, Fmi::date_time::DateTime> tests[] = {
        {"2024-Jan-24 12:34:45", make_datetime(2024, 1, 24, 12, 34, 45)},
        {"2024-Jan-24 12:34:45.123456", make_datetime(2024, 1, 24, 12, 34, 45, 123456)},
        {"2024-Jan-24 12:34:45.123456789", make_datetime(2024, 1, 24, 12, 34, 45, 123456)},
        {"2024-Jan-24 12:34:45.12345678901234567890", make_datetime(2024, 1, 24, 12, 34, 45, 123456)},
        {"2024-01-31 10:17:00", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"2024-01-31 10:17", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"2024-Jan-31 10:17", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"2024-01-31", make_datetime(2024, 1, 31, 0, 0, 0)},
        {"2024-Jan-31 10:17+0300", make_datetime(2024, 1, 31, 7, 17, 0)},
        {"2024-01-31 10:17+0300", make_datetime(2024, 1, 31, 7, 17, 0)},
        {"2024-Jan-31 10:17Z", make_datetime(2024, 1, 31, 10, 17, 0)}
    };

    for (const auto& test : tests)
    {
        try {
            Fmi::date_time::DateTime dt =
                SHOW_EXCEPTIONS(Fmi::date_time::DateTime::from_string(test.first));
            BOOST_CHECK_EQUAL(dt, test.second);
        }
        catch (const std::exception& e)
        {
            BOOST_ERROR("Failed to parse '" + test.first + "': " + e.what());
        }
    }
}

BOOST_AUTO_TEST_CASE(time_from_iso_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime::from_iso_string");

    const std::string fmt = "%Y-%m-%d %H:%M:%S";

    const std::pair<std::string, Fmi::date_time::DateTime> tests[] = {
        {"20240131T101700", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"20240131T1017", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"20240131", make_datetime(2024, 1, 31, 0, 0, 0)},
        {"20240131T101700+0300", make_datetime(2024, 1, 31, 7, 17, 0)},
        {"20240131T101700Z", make_datetime(2024, 1, 31, 10, 17, 0)}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int num_passed = 0;

    for (const auto& test : tests)
    {
        try {
            Fmi::date_time::DateTime dt =
                SHOW_EXCEPTIONS(Fmi::date_time::DateTime::from_iso_string(test.first));
            BOOST_CHECK_EQUAL(dt, test.second);
            num_passed++;
        }
        catch (const std::exception& e)
        {
            BOOST_ERROR("Failed to parse '" + test.first + "': " + e.what());
        }
    }

    if (num_passed != num_tests)
    {
        BOOST_TEST_MESSAGE("Failed " << num_tests - num_passed << " out of " << num_tests << " tests");
    }
}

BOOST_AUTO_TEST_CASE(time_from_iso_extended_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime::from_iso_extended_string");

    const std::string fmt = "%Y-%m-%d %H:%M:%S";

    const std::pair<std::string, Fmi::date_time::DateTime> tests[] = {
        {"2024-01-31T10:17:00", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"2024-01-31T10:17", make_datetime(2024, 1, 31, 10, 17, 0)},
        {"2024-01-31", make_datetime(2024, 1, 31, 0, 0, 0)},
        {"2024-01-31T10:17+0300", make_datetime(2024, 1, 31, 7, 17, 0)},
        {"2024-01-31T10:17Z", make_datetime(2024, 1, 31, 10, 17, 0)}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int num_passed = 0;

    for (const auto& test : tests)
    {
        try {
            Fmi::date_time::DateTime dt =
                SHOW_EXCEPTIONS(Fmi::date_time::DateTime::from_iso_extended_string(test.first));
            BOOST_CHECK_EQUAL(dt, test.second);
            num_passed++;
        }
        catch (const std::exception& e)
        {
            BOOST_ERROR("Failed to parse '" + test.first + "': " + e.what());
        }
    }

    if (num_passed != num_tests)
    {
        BOOST_TEST_MESSAGE("Failed " << num_tests - num_passed << " out of " << num_tests << " tests");
    }
}

BOOST_AUTO_TEST_CASE(test_serialization_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: serialization (text archive)");

    Fmi::date_time::DateTime dt1(Fmi::date_time::Date(1971, 1, 24), Fmi::date_time::TimeDuration(12, 34, 45, 123456));
    std::ostringstream os;
    {
        boost::archive::text_oarchive oa(os);
        oa & dt1;
    }

    //std::cout << "\ntext archive: " << os.str() << std::endl;

    Fmi::date_time::DateTime dt2;
    std::istringstream is(os.str());
    {
        boost::archive::text_iarchive ia(is);
        ia & dt2;
    }

    BOOST_CHECK_EQUAL(dt1, dt2);
}


BOOST_AUTO_TEST_CASE(test_serialization_2)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: serialization (xml archive)");

    Fmi::date_time::DateTime dt1(Fmi::date_time::Date(2014, 1, 24), Fmi::date_time::TimeDuration(12, 34, 45, 123456));
    std::ostringstream os;
    {
        boost::archive::xml_oarchive oa(os);
        oa & BOOST_SERIALIZATION_NVP(dt1);
    }

    //std::cout << "\nxml archive: " << os.str() << std::endl;

    Fmi::date_time::DateTime dt2;
    std::istringstream is(os.str());
    {
        boost::archive::xml_iarchive ia(is);
        ia & BOOST_SERIALIZATION_NVP(dt2);
    }

    BOOST_CHECK_EQUAL(dt1, dt2);
}

BOOST_AUTO_TEST_CASE(test_inf_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: infinity");

    using namespace Fmi::date_time;
    DateTime neg_inf = DateTime::NEG_INFINITY;
    DateTime pos_inf = DateTime::POS_INFINITY;
    DateTime dt1(Date(2024, 2, 16), TimeDuration(0, 0, 0));

    BOOST_CHECK(pos_inf > neg_inf);
    BOOST_CHECK(neg_inf < pos_inf);
    BOOST_CHECK(neg_inf < dt1);
    BOOST_CHECK(pos_inf > dt1);

    BOOST_CHECK(neg_inf != pos_inf);
    BOOST_CHECK(neg_inf != dt1);
    BOOST_CHECK(pos_inf != dt1);
    BOOST_CHECK(neg_inf == neg_inf);
    BOOST_CHECK(pos_inf == pos_inf);
}

BOOST_AUTO_TEST_CASE(test_inf_2)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: infinity");

    using namespace Fmi::date_time;
    DateTime ndt = DateTime::NOT_A_DATE_TIME;
    DateTime dt_neg_inf = DateTime::NEG_INFINITY;
    DateTime dt_norm(Date(2024, 2, 16), TimeDuration(0, 0, 0));
    DateTime dt_pos_inf = DateTime::POS_INFINITY;
    TimeDuration ntd = TimeDuration::NOT_A_DURATION;
    TimeDuration td_pos_inf = TimeDuration::POS_INFINITY;
    TimeDuration td_norm = TimeDuration(1, 2, 3);
    TimeDuration td_neg_inf = TimeDuration::NEG_INFINITY;

    BOOST_CHECK_EQUAL(dt_neg_inf + td_norm, dt_neg_inf);
    BOOST_CHECK_EQUAL(dt_neg_inf - td_pos_inf, dt_neg_inf);
    BOOST_CHECK_EQUAL(dt_pos_inf + td_norm, dt_pos_inf);
    BOOST_CHECK_EQUAL(dt_pos_inf - td_pos_inf, ndt);

    BOOST_CHECK_EQUAL(dt_neg_inf - td_norm, dt_neg_inf);
    BOOST_CHECK_EQUAL(dt_neg_inf + td_neg_inf, dt_neg_inf);

    BOOST_CHECK_EQUAL(dt_pos_inf - td_norm, dt_pos_inf);
    BOOST_CHECK_EQUAL(dt_pos_inf + td_neg_inf, ndt);

    // FIXME: add misisng tests

}

BOOST_AUTO_TEST_CASE(test_date_time_map_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: DateTime map index");

    using namespace Fmi::date_time;
    using namespace std::string_literals;

    const DateTime dt1(Date(2024, 1, 24), TimeDuration(12, 34, 45));
    const DateTime dt2(Date(2024, 1, 24), TimeDuration(12, 35, 45));
    const DateTime dt3(Date(2024, 1, 24), TimeDuration(12, 36, 45));
    const DateTime dt_ni(DateTime::NEG_INFINITY);
    const DateTime dt_pi(DateTime::POS_INFINITY);
    const DateTime dt_nad(DateTime::NOT_A_DATE_TIME);

    std::map<DateTime, std::string> m;

    m[dt1] = "first";
    m[dt2] = "second";
    m[dt_pi] = "pos infinity";
    m[dt_nad] = "not a date time";
    m[dt_ni] = "neg infinity";

    std::string str;
    BOOST_REQUIRE_NO_THROW(str = m[dt_nad]);
    BOOST_CHECK_EQUAL(str, "not a date time"s);

    BOOST_REQUIRE_NO_THROW(str = m[dt_ni]);
    BOOST_CHECK_EQUAL(str, "neg infinity"s);

    const auto it1 = m.find(dt1);
    BOOST_REQUIRE_EQUAL(it1->first.to_simple_string(), dt1.to_simple_string());
}


BOOST_AUTO_TEST_CASE(timing_1)
{
    constexpr const std::size_t size = 1000000;

    using namespace Fmi::date_time;

    BOOST_TEST_MESSAGE("Fmi::date_time::DateTime: timing test for effectiveness");
    BOOST_CHECK(true);  // We are interested how long it takes

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist(1, 1000);

    Date date(2024, 10, 27);
    TimeDuration time(2, 59, 59);
    DateTime dt1(date, time);                     // 02:59:59

    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<DateTime> v1;
    for (std::size_t i = 0; i < size; ++i)
    {
        dt1 += Seconds(1);
        v1.push_back(dt1);
    }
    std::shuffle(v1.begin(), v1.end(), rng);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    BOOST_TEST_MESSAGE("Test data created in " + std::to_string(duration) + " ms");

    t1 = std::chrono::high_resolution_clock::now();
    std::map<DateTime, int> m1;
    std::transform(v1.begin(), v1.end(), std::inserter(m1, m1.end()),
        [](const DateTime& ldt) { return std::make_pair(ldt, 0); });
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
