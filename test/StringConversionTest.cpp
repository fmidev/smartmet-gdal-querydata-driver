// ======================================================================
/*!
 * \file
 * \brief Regression tests for StringConversion.h
 */
// ======================================================================

#include "DateTime.h"
#include "Exception.h"
#include "StringConversion.h"
#include <numeric>
#include <optional>
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;
using namespace Fmi::date_time;
using namespace Fmi::literals;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "String conversions tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

namespace
{
  template <typename T>
  std::string as_string(const std::optional<T>& opt)
  {
    return opt ? Fmi::to_string(*opt) : "none";
  }
}

BOOST_AUTO_TEST_CASE(stoi)
{
  BOOST_TEST_MESSAGE(" + Fmi::stoi()");
  BOOST_CHECK_EQUAL(0, Fmi::stoi("0"));
  BOOST_CHECK_EQUAL(0, Fmi::stoi("+0"));
  BOOST_CHECK_EQUAL(0, Fmi::stoi("-0"));
  BOOST_CHECK_EQUAL(123, Fmi::stoi("123"));
  BOOST_CHECK_EQUAL(-123, Fmi::stoi("-123"));

  BOOST_CHECK_THROW(Fmi::stoi("321.1234"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi("123456789012"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi("ABC"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi("12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi(" 12"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoi(" 12 "), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stol)
{
  BOOST_TEST_MESSAGE(" + Fmi::stol()");
  BOOST_CHECK_EQUAL(0L, Fmi::stol("0"));
  BOOST_CHECK_EQUAL(0L, Fmi::stol("+0"));
  BOOST_CHECK_EQUAL(0L, Fmi::stol("-0"));
  BOOST_CHECK_EQUAL(123L, Fmi::stol("123"));
  BOOST_CHECK_EQUAL(-123L, Fmi::stol("-123"));
  BOOST_CHECK_EQUAL(123456789012, Fmi::stol("123456789012"));

  BOOST_CHECK_THROW(Fmi::stol("321.1234"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol("ABC"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol("12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol(" 12"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stol(" 12 "), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stoul)
{
  BOOST_TEST_MESSAGE(" + Fmi::stoul()");
  BOOST_CHECK_EQUAL(0UL, Fmi::stoul("0"));
  BOOST_CHECK_EQUAL(123UL, Fmi::stoul("123"));
  BOOST_CHECK_EQUAL(123456789012, Fmi::stoul("123456789012"));

  BOOST_CHECK_THROW(Fmi::stoul("+0"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("-0"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("-123"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("321.1234"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("ABC"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul("12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul(" 12"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stoul(" 12 "), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stof)
{
  BOOST_TEST_MESSAGE(" + Fmi::stof()");
  BOOST_CHECK_EQUAL(0.f, Fmi::stof("0"));
  BOOST_CHECK_EQUAL(0.f, Fmi::stof("+0"));
  BOOST_CHECK_EQUAL(0.f, Fmi::stof("-0"));
  BOOST_CHECK_EQUAL(123.f, Fmi::stof("123"));
  BOOST_CHECK_EQUAL(-123.f, Fmi::stof("-123"));
  BOOST_CHECK_CLOSE(321.1234f, Fmi::stof("321.1234"), 0.001);
  BOOST_CHECK_CLOSE(123456789012.f, Fmi::stof("123456789012"), 1);
  BOOST_CHECK_THROW(Fmi::stof("ABC"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof(" 12"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof(" 12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("NaN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("NAN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("+INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof("-INF"), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stod)
{
  BOOST_TEST_MESSAGE(" + Fmi::stod()");
  BOOST_CHECK_EQUAL(0, Fmi::stod("0"));
  BOOST_CHECK_EQUAL(0, Fmi::stod("+0"));
  BOOST_CHECK_EQUAL(0, Fmi::stod("-0"));
  BOOST_CHECK_EQUAL(123, Fmi::stod("123"));
  BOOST_CHECK_EQUAL(-123, Fmi::stod("-123"));
  BOOST_CHECK_CLOSE(321.1234f, Fmi::stod("321.1234"), 0.001);
  BOOST_CHECK_CLOSE(123456789012, Fmi::stod("123456789012"), 1);
  BOOST_CHECK_THROW(Fmi::stod("ABC"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("12A"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod(" 12"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod(" 12 "), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("NaN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("NAN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("+INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod("-INF"), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stoi_opt)
{
  constexpr const auto maxval = std::numeric_limits<int>::max();
  BOOST_TEST_MESSAGE(" + Fmi::stoi_opt()");
  BOOST_CHECK_EQUAL(0, Fmi::stoi_opt("0").value_or(maxval));
  BOOST_CHECK_EQUAL(0, Fmi::stoi_opt("+0").value_or(maxval));
  BOOST_CHECK_EQUAL(0, Fmi::stoi_opt("-0").value_or(maxval));
  BOOST_CHECK_EQUAL(123, Fmi::stoi_opt("123").value_or(maxval));
  BOOST_CHECK_EQUAL(-123, Fmi::stoi_opt("-123").value_or(maxval));

  BOOST_CHECK(!Fmi::stoi_opt("321.1234"));
  BOOST_CHECK(!Fmi::stoi_opt("ABC"));
  BOOST_CHECK(!Fmi::stoi_opt("12A"));
  BOOST_CHECK(!Fmi::stoi_opt("12A"));
  BOOST_CHECK(!Fmi::stoi_opt("12 "));
  BOOST_CHECK(!Fmi::stoi_opt(" 12"));
  BOOST_CHECK(!Fmi::stoi_opt(" 12 "));
}

BOOST_AUTO_TEST_CASE(stol_opt)
{
  constexpr const auto maxval = std::numeric_limits<long>::max();
  BOOST_TEST_MESSAGE(" + Fmi::stol_opt()");
  BOOST_CHECK_EQUAL(0L, Fmi::stol_opt("0").value_or(maxval));
  BOOST_CHECK_EQUAL(0L, Fmi::stol_opt("+0").value_or(maxval));
  BOOST_CHECK_EQUAL(0L, Fmi::stol_opt("-0").value_or(maxval));
  BOOST_CHECK_EQUAL(123L, Fmi::stol_opt("123").value_or(maxval));
  BOOST_CHECK_EQUAL(-123L, Fmi::stol_opt("-123").value_or(maxval));
  BOOST_CHECK_EQUAL(123456789012, Fmi::stol_opt("123456789012").value_or(maxval));

  BOOST_CHECK(!Fmi::stol_opt("321.1234"));
  BOOST_CHECK(!Fmi::stol_opt("ABC"));
  BOOST_CHECK(!Fmi::stol_opt("12A"));
  BOOST_CHECK(!Fmi::stol_opt("12A"));
  BOOST_CHECK(!Fmi::stol_opt("12 "));
  BOOST_CHECK(!Fmi::stol_opt(" 12"));
  BOOST_CHECK(!Fmi::stol_opt(" 12 "));
}

BOOST_AUTO_TEST_CASE(stoul_opt)
{
  constexpr const auto maxval = std::numeric_limits<long>::max();
  BOOST_TEST_MESSAGE(" + Fmi::stoul_opt()");
  BOOST_CHECK_EQUAL(0UL, Fmi::stoul_opt("0").value_or(maxval));
  BOOST_CHECK_EQUAL(123UL, Fmi::stoul_opt("123").value_or(maxval));
  BOOST_CHECK_EQUAL(123456789012UL, Fmi::stoul_opt("123456789012").value_or(maxval));

  BOOST_CHECK(!Fmi::stoul_opt("+0"));
  BOOST_CHECK(!Fmi::stoul_opt("-0"));
  BOOST_CHECK(!Fmi::stoul_opt("-123"));
  BOOST_CHECK(!Fmi::stoul_opt("321.1234"));
  BOOST_CHECK(!Fmi::stoul_opt("ABC"));
  BOOST_CHECK(!Fmi::stoul_opt("12A"));
  BOOST_CHECK(!Fmi::stoul_opt("12A"));
  BOOST_CHECK(!Fmi::stoul_opt("12 "));
  BOOST_CHECK(!Fmi::stoul_opt(" 12"));
  BOOST_CHECK(!Fmi::stoul_opt(" 12 "));
}

BOOST_AUTO_TEST_CASE(stof_opt)
{
  constexpr const auto maxval = std::numeric_limits<float>::max();
  BOOST_TEST_MESSAGE(" + Fmi::stof_opt()");
  BOOST_CHECK_EQUAL(0.f, Fmi::stof_opt("0").value_or(maxval));
  BOOST_CHECK_EQUAL(0.f, Fmi::stof_opt("+0").value_or(maxval));
  BOOST_CHECK_EQUAL(0.f, Fmi::stof_opt("-0").value_or(maxval));
  BOOST_CHECK_EQUAL(123.f, Fmi::stof_opt("123").value_or(maxval));
  BOOST_CHECK_EQUAL(-123.f, Fmi::stof_opt("-123").value_or(maxval));
  BOOST_CHECK(!Fmi::stof_opt("ABC"));
  BOOST_CHECK(!Fmi::stof_opt("12A"));
  BOOST_CHECK(!Fmi::stof_opt("12A"));
  BOOST_CHECK(!Fmi::stof_opt("12 "));
  BOOST_CHECK(!Fmi::stof_opt(" 12"));
  BOOST_CHECK(!Fmi::stof_opt(" 12 "));
  BOOST_CHECK_THROW(Fmi::stof_opt("NaN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof_opt("NAN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof_opt("INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof_opt("+INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stof_opt("-INF"), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(stod_opt)
{
  constexpr const auto maxval = std::numeric_limits<float>::max();
  BOOST_TEST_MESSAGE(" + Fmi::stod_opt()");
  BOOST_CHECK_EQUAL(0.0, Fmi::stod_opt("0").value_or(maxval));
  BOOST_CHECK_EQUAL(0.0, Fmi::stod_opt("+0").value_or(maxval));
  BOOST_CHECK_EQUAL(0.0, Fmi::stod_opt("-0").value_or(maxval));
  BOOST_CHECK_EQUAL(123.0, Fmi::stod_opt("123").value_or(maxval));
  BOOST_CHECK_EQUAL(-123.0, Fmi::stod_opt("-123").value_or(maxval));
  BOOST_CHECK(!Fmi::stod_opt("ABC"));
  BOOST_CHECK(!Fmi::stod_opt("12A"));
  BOOST_CHECK(!Fmi::stod_opt("12A"));
  BOOST_CHECK(!Fmi::stod_opt("12 "));
  BOOST_CHECK(!Fmi::stod_opt(" 12"));
  BOOST_CHECK(!Fmi::stod_opt(" 12 "));
  BOOST_CHECK_THROW(Fmi::stod_opt("NaN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod_opt("NAN"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod_opt("INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod_opt("+INF"), Fmi::Exception);
  BOOST_CHECK_THROW(Fmi::stod_opt("-INF"), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(to_string)
{
  BOOST_TEST_MESSAGE(" + Fmi::to_string()");
  BOOST_CHECK_EQUAL("0", Fmi::to_string(false));
  BOOST_CHECK_EQUAL("1", Fmi::to_string(true));
  BOOST_CHECK_EQUAL("0", Fmi::to_string(0));
  BOOST_CHECK_EQUAL("1", Fmi::to_string(1));
  BOOST_CHECK_EQUAL("-1", Fmi::to_string(-1));
  BOOST_CHECK_EQUAL("1234567890", Fmi::to_string(1234567890));
  BOOST_CHECK_EQUAL("1234567890123456789", Fmi::to_string(1234567890123456789));
  BOOST_CHECK_EQUAL("-1234567890", Fmi::to_string(-1234567890));
  BOOST_CHECK_EQUAL("-1234567890123456789", Fmi::to_string(-1234567890123456789));

  BOOST_CHECK_EQUAL("0", Fmi::to_string(0L));
  BOOST_CHECK_EQUAL("1", Fmi::to_string(1L));
  BOOST_CHECK_EQUAL("-1", Fmi::to_string(-1L));
  BOOST_CHECK_EQUAL("1234567890", Fmi::to_string(1234567890L));
  BOOST_CHECK_EQUAL("1234567890123456789", Fmi::to_string(1234567890123456789L));
  BOOST_CHECK_EQUAL("-1234567890", Fmi::to_string(-1234567890L));
  BOOST_CHECK_EQUAL("-1234567890123456789", Fmi::to_string(-1234567890123456789L));

  BOOST_CHECK_EQUAL("0", Fmi::to_string(0UL));
  BOOST_CHECK_EQUAL("1", Fmi::to_string(1UL));
  BOOST_CHECK_EQUAL("1234567890", Fmi::to_string(1234567890UL));
  BOOST_CHECK_EQUAL("1234567890123456789", Fmi::to_string(1234567890123456789UL));

  BOOST_CHECK_EQUAL("0", Fmi::to_string(0.0));
  BOOST_CHECK_EQUAL("0.1", Fmi::to_string(0.1));
  BOOST_CHECK_EQUAL("1", Fmi::to_string(1.0));
  BOOST_CHECK_EQUAL("1.2345", Fmi::to_string(1.2345));
  BOOST_CHECK_EQUAL("1.23457", Fmi::to_string(1.234567890123456789f));
  BOOST_CHECK_EQUAL("1.23457", Fmi::to_string(1.234567890123456789));

  BOOST_CHECK_EQUAL("1.235", Fmi::to_string("%.3f", 1.234567890123456789f));
  BOOST_CHECK_EQUAL("1.235", Fmi::to_string("%.3f", 1.234567890123456789));
}

BOOST_AUTO_TEST_CASE(conv_to_iso_string)
{
  BOOST_TEST_MESSAGE(" + Fmi::to_iso_string()");

  Fmi::DateTime time1(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3));
  BOOST_CHECK_EQUAL("20020101T010203", Fmi::to_iso_string(time1));

  Fmi::DateTime time2(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3) + Fmi::Milliseconds(4));
  BOOST_CHECK_EQUAL("20020101T010203,004000", Fmi::to_iso_string(time2));

  Fmi::DateTime time3(Fmi::Date(1970, Jan, 1), Fmi::TimeDuration(0, 0, 0));
  BOOST_CHECK_EQUAL("19700101T000000", Fmi::to_iso_string(std::time_t(0)));

  // Fmi::Date u --date='@2147483647' --> Tue Jan 19 03:14:07 EET 2038
  Fmi::DateTime time4(Fmi::Date(2038, Jan, 19), Fmi::TimeDuration(3, 14, 7));
  BOOST_CHECK_EQUAL("20380119T031407", Fmi::to_iso_string(std::time_t(2147483647)));
}

BOOST_AUTO_TEST_CASE(conv_to_iso_extended_string)
{
  BOOST_TEST_MESSAGE(" + Fmi::to_iso_extended_string()");

  Fmi::DateTime time1(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3));
  BOOST_CHECK_EQUAL("2002-01-01T01:02:03", Fmi::to_iso_extended_string(time1));

  Fmi::DateTime time2(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3) + Milliseconds(4));
  BOOST_CHECK_EQUAL("2002-01-01T01:02:03,004000", Fmi::to_iso_extended_string(time2));
}

BOOST_AUTO_TEST_CASE(conv_to_simple_string)
{
  BOOST_TEST_MESSAGE(" + Fmi::to_simple_string()");

  Fmi::DateTime time1(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3));
  BOOST_CHECK_EQUAL("2002-Jan-01 01:02:03", Fmi::to_simple_string(time1));

  Fmi::DateTime time2(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3) + Milliseconds(4));
  BOOST_CHECK_EQUAL("2002-Jan-01 01:02:03,004000", Fmi::to_simple_string(time2));

  Fmi::TimeDuration dura1(123, 45, 43);
  BOOST_CHECK_EQUAL("123:45:43", Fmi::to_simple_string(dura1));

  Fmi::TimeDuration dura2(123456, 45, 43);
  BOOST_CHECK_EQUAL("123456:45:43", Fmi::to_simple_string(dura2));

  Fmi::TimeDuration dura3(0, 45, 43);
  BOOST_CHECK_EQUAL("00:45:43", Fmi::to_simple_string(dura3));
}

BOOST_AUTO_TEST_CASE(to_http_string)
{
  BOOST_TEST_MESSAGE(" + Fmi::to_http_string()");

  Fmi::DateTime time1(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3));
  BOOST_CHECK_EQUAL("Tue, 01 Jan 2002 01:02:03 GMT", Fmi::to_http_string(time1));

  Fmi::DateTime time2(Fmi::Date(2002, Jan, 1), Fmi::TimeDuration(1, 2, 3) + Milliseconds(4));
  BOOST_CHECK_EQUAL("Tue, 01 Jan 2002 01:02:03 GMT", Fmi::to_http_string(time2));
}

BOOST_AUTO_TEST_CASE(ascii_tolower)
{
  BOOST_TEST_MESSAGE(" + Fmi::ascii_tolower()");

  std::string input = "ABDCEFGHIJKLMNOPQRSTUVWXYZ";

  Fmi::ascii_tolower(input);

  const std::string correct_result = "abdcefghijklmnopqrstuvwxyz";

  BOOST_CHECK_EQUAL(input, correct_result);
}

BOOST_AUTO_TEST_CASE(ascii_toupper)
{
  BOOST_TEST_MESSAGE(" + Fmi::ascii_toupper()");

  std::string input = "abdcefghijklmnopqrstuvwxyz";

  Fmi::ascii_toupper(input);

  const std::string correct_result = "ABDCEFGHIJKLMNOPQRSTUVWXYZ";

  BOOST_CHECK_EQUAL(input, correct_result);
}

BOOST_AUTO_TEST_CASE(trim)
{
  BOOST_TEST_MESSAGE(" + Fmi::trim()");

  std::string input = "";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "");

  input = " ";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "");

  input = "  ";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "");

  input = "a ";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "a");

  input = " a ";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "a");

  input = " a";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "a");

  input = "\t\n a\t\n ";
  Fmi::trim(input);
  BOOST_CHECK_EQUAL(input, "a");
}

BOOST_AUTO_TEST_CASE(xmlescape)
{
  BOOST_TEST_MESSAGE(" + Fmi::xmlescape()");

  std::string input = "< 100";
  std::string output = Fmi::xmlescape(input);
  BOOST_CHECK_EQUAL(output, "&lt; 100");
}

BOOST_AUTO_TEST_CASE(safexmlescape)
{
  BOOST_TEST_MESSAGE(" + Fmi::safexmlescape()");

  std::string input = "< 100 &lt; 200 & &amp;";
  std::string output = Fmi::safexmlescape(input);
  BOOST_CHECK_EQUAL(output, "&lt; 100 &lt; 200 &amp; &amp;");

  input = "12&&34";
  output = Fmi::safexmlescape(input);
  BOOST_CHECK_EQUAL("12&amp;&amp;34", output);

  input = "&#176;C";
  output = Fmi::safexmlescape(input);
  BOOST_CHECK_EQUAL(input, output);
}

BOOST_AUTO_TEST_CASE(stdsz)
{
  BOOST_TEST_MESSAGE(" + Fmi::stosz()");

  BOOST_CHECK_EQUAL(123UL, Fmi::stosz("123B"));
  BOOST_CHECK_EQUAL(123UL << 10, Fmi::stosz("123K"));
  BOOST_CHECK_EQUAL(123UL << 20, Fmi::stosz("123M"));
  BOOST_CHECK_EQUAL(123UL << 30, Fmi::stosz("123G"));
  BOOST_CHECK_EQUAL(123UL << 40, Fmi::stosz("123T"));
  BOOST_CHECK_EQUAL(123UL << 50, Fmi::stosz("123P"));
}

// ======================================================================
