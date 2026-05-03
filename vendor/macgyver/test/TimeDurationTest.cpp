// ======================================================================
/*!
 * \file
 * \brief Regression tests for date_time/TimeDuration.h
 */
// ======================================================================

#include "date_time/TimeDuration.h"
#include "DebugTools.h"
#include <functional>
#include <iostream>
#include <boost/test/included/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time/TimeDuration tester";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_TimeDuration_1)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration: extraction and conversion to string");

    using namespace Fmi::date_time;

    int num_err = 0;
    int num_tests = 0;
    int mks = 0;
    for (int hours = 0; hours < 48; hours++)
    {
      for (int minutes = 0; minutes < 60; minutes++)
      {
        for (int seconds = 0; seconds < 60; seconds++)
        {
            using namespace boost::posix_time;
            num_tests++;
            if (num_tests > 7200) mks += 1;
            TimeDuration td(hours, minutes, seconds, mks);
            int frac = mks * time_duration::ticks_per_second() / td.ticks_per_second();
            time_duration pt(hours, minutes, seconds, frac);
            const std::string str1 = td.to_simple_string();
            const std::string str2 = boost::posix_time::to_simple_string(pt);

            const std::string str3 = td.to_iso_string();
            const std::string str4 = boost::posix_time::to_iso_string(pt);
            if (str1 != str2)
            {
                if (num_err < 10) {
                    std::cout << "Error: " << str1 << " != " << str2
                              << std::endl;
                }
                num_err++;
            }
            if (str3 != str4)
            {
                if (num_err < 10) {
                    std::cout << "Error: " << str3 << " != " << str4
                              << std::endl;
                }
                num_err++;
            }

            BOOST_REQUIRE_EQUAL(int(td.hours()), hours);
            BOOST_REQUIRE_EQUAL(int(td.minutes()), minutes);
            BOOST_REQUIRE_EQUAL(int(td.seconds()), seconds);
        }
      }
    }

    BOOST_REQUIRE_EQUAL(num_err, 0);
}

BOOST_AUTO_TEST_CASE(test_TimeDuration_2)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration: test negative value");

    using namespace Fmi::date_time;
    using namespace boost::posix_time;

    TimeDuration td1(-1, -2, -3, -4);
    time_duration pt1(-1, -2, -3, -4);

    BOOST_REQUIRE_EQUAL(td1.hours(), pt1.hours());
    BOOST_REQUIRE_EQUAL(td1.minutes(), pt1.minutes());
    BOOST_REQUIRE_EQUAL(td1.seconds(), pt1.seconds());

    const std::string str1 = td1.to_simple_string();
    const std::string str2 = boost::posix_time::to_simple_string(pt1);
    BOOST_REQUIRE_EQUAL(str1, str2);
}

BOOST_AUTO_TEST_CASE(test_operations)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration: test operations");

    using namespace Fmi::date_time;

    TimeDuration td1(1, 2, 3, 4);
    TimeDuration td2(0, 3, 0, 0);
    TimeDuration td3(1, 5, 3, 4);
    TimeDuration td4(0, 59, 3, 4);
    TimeDuration td5(2, 4, 6, 8);

    BOOST_CHECK(td1 == td1);
    BOOST_CHECK(td1 != td2);
    BOOST_CHECK(td1 > td2);
    BOOST_CHECK(td1 >= td1);
    BOOST_CHECK(td1 >= td2);
    BOOST_CHECK(td4 < td1);
    BOOST_CHECK(td4 <= td1);
    BOOST_CHECK(td4 <= td4);
    BOOST_CHECK_EQUAL(td1 + td2, td3);
    BOOST_CHECK_EQUAL(td1 - td2, td4);
    BOOST_CHECK_EQUAL(td1 * 2, td5);
    BOOST_CHECK_EQUAL(td5 / 2, td1);
}

BOOST_AUTO_TEST_CASE(from_string)
{
    // FIXME: test also limit support

    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration::from_string");

    using Fmi::date_time::duration_from_string;

    const std::pair<const char*, const char*> test_data[] =
        {
            {"12:11:10.123456789", "12:11:10.123456"}
            , {"12:11:10.123456", nullptr}
            , {"12:11:10.12345", "12:11:10.123450"}
            , {"12:11:10.1234", "12:11:10.123400"}
            , {"12:11:10.123", "12:11:10.123000"}
            , {"12:11:10.12", "12:11:10.120000"}
            , {"12:11:10.1", "12:11:10.100000"}
            , {"12:11:10.", "12:11:10"}
            , {"12:11:10", nullptr}
            , {"-12:11:10", nullptr}
            , {"+12:11:10", "12:11:10"}
            , {"12:11 ", "12:11:00"}
            , {"10:17", "10:17:00"}
        };

    const auto test_parse = [](const std::string& src) -> Fmi::date_time::TimeDuration
    {
        return SHOW_EXCEPTIONS(duration_from_string(src));
    };

    for (const auto& td : test_data)
    {
        const char* src = td.first;
        const char* dst = td.second ? td.second : td.first;
        Fmi::date_time::TimeDuration td1;
        BOOST_CHECK_NO_THROW(td1 = test_parse(src));
        BOOST_CHECK_EQUAL(td1.to_simple_string(), dst);
    }
}

BOOST_AUTO_TEST_CASE(from_iso_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration::from_iso_string");

    using Fmi::date_time::duration_from_string;

    const std::pair<const char*, const char*> test_data[] =
        {
            {"121110.123456789", "12:11:10.123456"}
            , {"121110.123456", "12:11:10.123456"}
            , {"121110.12345", "12:11:10.123450"}
            , {"121110.1234", "12:11:10.123400"}
            , {"121110.123", "12:11:10.123000"}
            , {"121110.12", "12:11:10.120000"}
            , {"-121110.12", "-12:11:10.120000"}
            , {"+121110.12", "12:11:10.120000"}
            , {"121110.1", "12:11:10.100000"}
            , {"121110.", "12:11:10"}
            , {"121110", "12:11:10"}
            , {"1211 ", "12:11:00"}
            , {"1017", "10:17:00"}
        };

    const auto test_parse = [](const std::string& src) -> Fmi::date_time::TimeDuration
    {
        return SHOW_EXCEPTIONS(Fmi::date_time::TimeDuration::from_iso_string(src));
    };

    for (const auto& td : test_data)
    {
        const char* src = td.first;
        const char* dst = td.second ? td.second : td.first;
        Fmi::date_time::TimeDuration td1;
        BOOST_CHECK_NO_THROW(td1 = test_parse(src));
        BOOST_CHECK_EQUAL(td1.to_simple_string(), dst);
    }
}

BOOST_AUTO_TEST_CASE(from_iso_extended_string)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration::from_iso_extended_string");

    using Fmi::date_time::duration_from_string;

    const std::pair<const char*, const char*> test_data[] =
        {
            {"12:11:10.123456789", "12:11:10.123456"}
            , {"12:11:10.123456", "12:11:10.123456"}
            , {"12:11:10.12345", "12:11:10.123450"}
            , {"12:11:10.1234", "12:11:10.123400"}
            , {"12:11:10.123", "12:11:10.123000"}
            , {"12:11:10.12", "12:11:10.120000"}
            , {"12:11:10.1", "12:11:10.100000"}
            , {"12:11:10.", "12:11:10"}
            , {"12:11:10", "12:11:10"}
            , {"12:11 ", "12:11:00"}
            , {"10:17", "10:17:00"}
        };

    const auto test_parse = [](const std::string& src) -> Fmi::date_time::TimeDuration
    {
        return SHOW_EXCEPTIONS(Fmi::date_time::TimeDuration::from_iso_extended_string(src));
    };

    for (const auto& td : test_data)
    {
        const char* src = td.first;
        const char* dst = td.second ? td.second : td.first;
        Fmi::date_time::TimeDuration td1;
        BOOST_CHECK_NO_THROW(td1 = test_parse(src));
        BOOST_CHECK_EQUAL(td1.to_simple_string(), dst);
    }
}

BOOST_AUTO_TEST_CASE(factory_methods)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration: test factory methods");

    using Fmi::date_time::Hours;
    using Fmi::date_time::Minutes;
    using Fmi::date_time::Seconds;
    using Fmi::date_time::Milliseconds;
    using Fmi::date_time::Microseconds;

    BOOST_CHECK_EQUAL(Hours(1).to_simple_string(), "01:00:00");
    BOOST_CHECK_EQUAL(Minutes(1).to_simple_string(), "00:01:00");
    BOOST_CHECK_EQUAL(Seconds(1).to_simple_string(), "00:00:01");
    BOOST_CHECK_EQUAL(Milliseconds(1).to_simple_string(), "00:00:00.001000");
    BOOST_CHECK_EQUAL(Microseconds(1).to_simple_string(), "00:00:00.000001");

    BOOST_CHECK_EQUAL(Minutes(1445).to_simple_string(), "24:05:00");
    BOOST_CHECK_EQUAL(Seconds(86401).to_simple_string(), "24:00:01");

    BOOST_CHECK_EQUAL(Minutes(-1445).to_simple_string(), "-24:05:00");
    BOOST_CHECK_EQUAL(Seconds(-86401).to_simple_string(), "-24:00:01");
}

BOOST_AUTO_TEST_CASE(serialization_test)
{
    BOOST_TEST_MESSAGE("Fmi::date_time::TimeDuration: test serialization");

    using namespace Fmi::date_time;

    std::vector<TimeDuration> test_data =
        {
                TimeDuration(1, 2, 3, 4),
                TimeDuration(TimeDuration::NOT_A_DATE_TIME),
                TimeDuration(TimeDuration::POS_INFINITY),
                TimeDuration(TimeDuration::NEG_INFINITY)
        };

        for (const auto& td : test_data)
        {
            std::stringstream ss;
            {
                boost::archive::text_oarchive oa(ss);
                oa << td;
            }

            TimeDuration td1;
            {
                boost::archive::text_iarchive ia(ss);
                ia >> td1;
            }

            BOOST_CHECK_MESSAGE(td == td1, "Serialization failed for " + td.to_simple_string()
                + ". Got " + td1.to_simple_string());
        }
}
