#include "date_time/ParserDefinitions.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
;
#include <boost/test/included/unit_test.hpp>
#include <iostream>

#define DEBUG_PARSER_FEFINITIONS 0

#if DEBUG_PARSER_FEFINITIONS
#define DEBUG(X) X
#else
#define DEBUG(X)
#endif

using namespace boost::unit_test;
using namespace std::string_literals;

namespace qi = boost::spirit::qi;

using iterator = std::string::const_iterator;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "Fmi::date_time::ParserDefinitions test";
  // unit_test_log.set_threshold_level(log_test_units);
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  std::setlocale(LC_ALL, "C");
  return nullptr;
}

#if DEBUG_PARSER_FEFINITIONS
namespace
{
    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::date_members_t& dm)
    {
        os << "(DATE " << dm.year << "-" << dm.month << "-" << dm.mday << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::seconds_members_t& sm)
    {
        os << "(SECONDS " << sm.seconds << "." << sm.frac_sec << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::duration_members_t& dm)
    {
        os << "(DURATION " << dm.hours << ":" << dm.minutes;
        if (dm.seconds) os  << ":" << *dm.seconds;
        os << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::time_zone_offset_members_t& tz)
    {
        os << "(TZ_OFFSET " << tz.sign << tz.hours << ":" << tz.minutes << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::date_time_members_t& dt)
    {
        os << "(DATE_TIME " << dt.date;
        if (dt.time) os << " " << *dt.time;
        if (dt.tz_offset) os << " " << *dt.tz_offset;
        os << ")";
        return os;
    }
}
#endif

BOOST_AUTO_TEST_CASE (test_month_short_name_parser)
{
    BOOST_TEST_MESSAGE("Month shory name parser");
    bool ok;
    unsigned result;
    Fmi::date_time::parser::MonthShortNameParser<iterator> parser;

    std::string input = "Jan";
    std::string::const_iterator begin = input.begin();
    std::string::const_iterator end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, result);
    BOOST_CHECK(ok);
    if (ok) {
        BOOST_CHECK_EQUAL(result, 1);
    }

    input = "feb";
    begin = input.begin();
    end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, result);
    BOOST_CHECK(ok);
    if (ok) {
        BOOST_CHECK_EQUAL(result, 2);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_date_iso_string)
{
    BOOST_TEST_MESSAGE("Parse ISO string");
    bool ok;
    Fmi::date_time::parser::date_members_t date_result;
    Fmi::date_time::parser::DateParser<iterator, char> parser(0, true);

    std::string input = "20170314";
    std::string::const_iterator begin = input.begin();
    std::string::const_iterator end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, date_result);
    BOOST_CHECK(ok);
    if (ok) {
        BOOST_CHECK_EQUAL(date_result.year, 2017);
        BOOST_CHECK_EQUAL(date_result.month, 3);
        BOOST_CHECK_EQUAL(date_result.mday, 14);
    }

    input = "2017-03-14";
    begin = input.begin();
    end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, date_result);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_CASE(test_parse_date_iso_extended_string)
{
    BOOST_TEST_MESSAGE("Parse ISO string");
    bool ok;
    Fmi::date_time::parser::date_members_t date_result;
    Fmi::date_time::parser::DateParser<iterator, char> parser('-', true);

    std::string input = "2017-03-14";
    std::string::const_iterator begin = input.begin();  
    std::string::const_iterator end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, date_result);
    BOOST_CHECK(ok);
    if (ok) {
        BOOST_CHECK_EQUAL(date_result.year, 2017);
        BOOST_CHECK_EQUAL(date_result.month, 3);
        BOOST_CHECK_EQUAL(date_result.mday, 14);
    }

    input = "20170314";
    begin = input.begin();
    end = input.end();
    ok = boost::spirit::qi::parse(begin, end, parser, date_result);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_CASE (test_simple_date_parser)
{
    BOOST_TEST_MESSAGE("Simple date parser");

    struct {
        std::string input;
        unsigned year;
        unsigned month;
        unsigned mday;
    }
    const test_data_pass[] = {
        {"2017-03-14"s, 2017, 3, 14},
        {"2017-3-4"s, 2017, 3, 4},
        {"2024-Feb-27"s, 2024, 2, 27}
    };

    const std::string test_input_fail[] = {
        "",
        "2017-03 14",
        "2017-13-14",
        "2017-03-32"
    };

    using iterator = std::string::const_iterator;

    Fmi::date_time::parser::DateParser<iterator, char> parser1('-', true);
    Fmi::date_time::parser::DateParser<iterator, char> parser2(0, true);
    Fmi::date_time::parser::DateParser<iterator, char> parser3('-', false);

    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::date_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(
            begin, end,
            ( parser1 | parser2 | parser3) >> qi::eoi,
            result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.year, td.year);
            BOOST_CHECK_EQUAL(result.month, td.month);
            BOOST_CHECK_EQUAL(result.mday, td.mday);
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input);
        }
    }   

    for (const auto& input : test_input_fail) {
        Fmi::date_time::parser::date_members_t result;
        iterator begin = input.begin();
        iterator end = input.end();
        bool ok = boost::spirit::qi::parse(
            begin, end,
            ( parser1 | parser2 | parser3) >> qi::eoi,
            result);
        if (ok) {
            BOOST_FAIL("Parsing should have failed for input: " + input);
        }
    }
}

BOOST_AUTO_TEST_CASE (test_parse_seconds)
{
    BOOST_TEST_MESSAGE("Parse seconds");

    struct {
        std::string input;
        unsigned seconds;
        std::string frac_sec;
    }
    const test_data_pass[] = {
        {"12.345"s, 12, "345"},
        {"12.345678"s, 12, "345678"},
        {"12"s, 12, ""}
    };

    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::SecondsParser<iterator> parser(true);
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        Fmi::date_time::parser::seconds_members_t result;
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.seconds, td.seconds);
            BOOST_CHECK_EQUAL(result.frac_sec, td.frac_sec);
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input);
        }
    }
}

BOOST_AUTO_TEST_CASE (test_duration_parser_1)
{
    BOOST_TEST_MESSAGE("Duration parser (iso format, < 24 hours)");

    struct {
        std::string input;
        int sign;
        unsigned hours;
        unsigned minutes;
        unsigned seconds;
        std::string frac_sec;
    }
    const test_data_pass[] = {
        {"123456.789"s, 1, 12, 34, 56, "789"},
        {"123456.7"s, 1, 12, 34, 56, "7"},
        {"123456"s, 1, 12, 34, 56, ""},
    };

 
    using iterator = std::string::const_iterator;

    // No separator, up to 23 hours including
    Fmi::date_time::parser::DurationParser<iterator, char> parser(0, false, 24);
    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::duration_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.hours, td.hours);
            BOOST_CHECK_EQUAL(result.minutes, td.minutes);
            if (result.seconds) {
                BOOST_CHECK_EQUAL(result.seconds->seconds, td.seconds);
                BOOST_CHECK_EQUAL(result.seconds->frac_sec, td.frac_sec);
            } else {
                BOOST_CHECK_EQUAL(td.seconds, 0);
                BOOST_CHECK_EQUAL(td.frac_sec, "");
            } 
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}

BOOST_AUTO_TEST_CASE (test_duration_parser_2)
{
    BOOST_TEST_MESSAGE("Duration parser (iso extended format, < 24 hours)");

    struct {
        std::string input;
        int sign;
        unsigned hours;
        unsigned minutes;
        unsigned seconds;
        std::string frac_sec;
    }
    const test_data_pass[] = {
        {"12:34:56.789"s, 1, 12, 34, 56, "789"},
        {"12:34:56.7"s, 1, 12, 34, 56, "7"},
        {"12:34:56"s, 1, 12, 34, 56, ""},
        {"12:34"s, 1, 12, 34, 0, ""},
        {"12:3:4.56"s, 1, 12, 3, 4, "56"}
    };
 
    using iterator = std::string::const_iterator;

    // No separator, up to 23 hours including
    Fmi::date_time::parser::DurationParser<iterator, char> parser(':', false, 24);
    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::duration_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.sign, td.sign);
            BOOST_CHECK_EQUAL(result.hours, td.hours);
            BOOST_CHECK_EQUAL(result.minutes, td.minutes);
            if (result.seconds) {
                BOOST_CHECK_EQUAL(result.seconds->seconds, td.seconds);
                BOOST_CHECK_EQUAL(result.seconds->frac_sec, td.frac_sec);
            } else {
                BOOST_CHECK_EQUAL(td.seconds, 0);
                BOOST_CHECK_EQUAL(td.frac_sec, "");
            }
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}

BOOST_AUTO_TEST_CASE (test_duration_parser_3)
{
    BOOST_TEST_MESSAGE("Duration parser (iso format, no 24 hours limit)");

    struct {
        std::string input;
        int sign;
        unsigned hours;
        unsigned minutes;
        unsigned seconds;
        std::string frac_sec;
    }
    const test_data_pass[] = {
        {"123456.789"s, 1, 12, 34, 56, "789"},
        {"-123456.789"s, -1, 12, 34, 56, "789"},
        {"+123456.789"s, 1, 12, 34, 56, "789"},
        {"123456.7"s, 1, 12, 34, 56, "7"},
        {"123456"s, 1, 12, 34, 56, ""},
    };


    using iterator = std::string::const_iterator;

    // No separator, up to 23 hours including
    Fmi::date_time::parser::DurationParser<iterator, char> parser(0, true, std::numeric_limits<unsigned>::max());
    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::duration_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.hours, td.hours);
            BOOST_CHECK_EQUAL(result.minutes, td.minutes);
            if (result.seconds) {
                BOOST_CHECK_EQUAL(result.seconds->seconds, td.seconds);
                BOOST_CHECK_EQUAL(result.seconds->frac_sec, td.frac_sec);
            } else {
                BOOST_CHECK_EQUAL(td.seconds, 0);
                BOOST_CHECK_EQUAL(td.frac_sec, "");
            }
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}

BOOST_AUTO_TEST_CASE (test_duration_parser_4)
{
    BOOST_TEST_MESSAGE("Duration parser (iso extended format, no 24 hours limit)");

    struct {
        std::string input;
        int sign;
        unsigned hours;
        unsigned minutes;
        unsigned seconds;
        std::string frac_sec;
    }
    const test_data_pass[] = {
        {"12:34:56.789"s, 1, 12, 34, 56, "789"},
        {"-12:34:56.789"s, -1, 12, 34, 56, "789"},
        {"+12:34:56.789"s, 1, 12, 34, 56, "789"},
        {"12:34:56.7"s, 1, 12, 34, 56, "7"},
        {"12:34:56"s, 1, 12, 34, 56, ""},
        {"12:34"s, 1, 12, 34, 0, ""},
        {"12:3:4.56"s, 1, 12, 3, 4, "56"}
    };

    using iterator = std::string::const_iterator;

    // No separator, up to 23 hours including
    Fmi::date_time::parser::DurationParser<iterator, char> parser(':', true, std::numeric_limits<unsigned>::max());
    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::duration_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.sign, td.sign);
            BOOST_CHECK_EQUAL(result.hours, td.hours);
            BOOST_CHECK_EQUAL(result.minutes, td.minutes);
            if (result.seconds) {
                BOOST_CHECK_EQUAL(result.seconds->seconds, td.seconds);
                BOOST_CHECK_EQUAL(result.seconds->frac_sec, td.frac_sec);
            } else {
                BOOST_CHECK_EQUAL(td.seconds, 0);
                BOOST_CHECK_EQUAL(td.frac_sec, "");
            } 
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}

BOOST_AUTO_TEST_CASE (time_zone_offset_1)
{
    BOOST_TEST_MESSAGE("Time zone offset parser");
    struct {
        std::string input;
        char sign;
        unsigned hours;
        unsigned minutes;
    } const test_data_pass[] = {
        {"+02:00"s, '+', 2, 0},
        {"-02:00"s, '-', 2, 0},
        {"+02:30"s, '+', 2, 30},
        {"-02:30"s, '-', 2, 30},
        {"Z"s, 'Z', 0, 0}
    };  

    using iterator = std::string::const_iterator;
    Fmi::date_time::parser::TimeZoneOffsetParser<iterator> parser;
    for (const auto& td : test_data_pass) {
        Fmi::date_time::parser::time_zone_offset_members_t result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, parser, result);
        if (ok) {
            BOOST_CHECK_EQUAL(result.sign, td.sign);
            BOOST_CHECK_EQUAL(result.hours, td.hours);
            BOOST_CHECK_EQUAL(result.minutes, td.minutes);
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}

BOOST_AUTO_TEST_CASE (time_zone_offset_2)
{
    BOOST_TEST_MESSAGE("Time zone offset parser 2");
    struct {
        std::string input;
        char sign;
        unsigned hours;
        unsigned minutes;
    } const test_data_pass[] = {
        {"+02:00"s, '+', 2, 0},
        {"-02:00"s, '-', 2, 0},
        {"+02:30"s, '+', 2, 30},
        {"-02:30"s, '-', 2, 30},
        {"Z"s, 'Z', 0, 0}
    };

    using iterator = std::string::const_iterator;
    Fmi::date_time::parser::TimeZoneOffsetParser<iterator> parser;
    for (const auto& td : test_data_pass) {
        std::optional<Fmi::date_time::parser::time_zone_offset_members_t> result;
        iterator begin = td.input.begin();
        iterator end = td.input.end();
        bool ok = boost::spirit::qi::parse(begin, end, -parser >> qi::eoi, result);
        if (ok) {
            if (result) {
                BOOST_CHECK_EQUAL(result->sign, td.sign);
                BOOST_CHECK_EQUAL(result->hours, td.hours);
                BOOST_CHECK_EQUAL(result->minutes, td.minutes);
            } else {
                BOOST_FAIL("Unexpected empty result for input: " + td.input);
            }
        } else {
            BOOST_FAIL("Parsing failed for input: " + td.input + ", position="
                + std::string(begin, end));
        }
    }
}
