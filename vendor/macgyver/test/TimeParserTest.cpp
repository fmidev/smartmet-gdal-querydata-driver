// ======================================================================
/*!
 * \file
 * \brief Regression tests for class TimeParser
 */
// ======================================================================

#include "TimeParser.h"
#include "TimeZoneFactory.h"
#include <regression/tframe.h>

#include "TimeParserTester.h"

template <typename T>
std::string tostring(const T& obj)
{
  std::ostringstream out;
  out << obj;
  return out.str();
}

// Protection against conflicts with global functions
namespace TimeParserTest
{
// ----------------------------------------------------------------------

void parse_timestamp()
{
  using namespace Fmi;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      {"20070102T0500", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"20000228T0515", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))}};

  std::vector<std::string> should_fail =
      {
          "foobar",
          "12345678901",
          "123456789012",
          "202402122401"
      };

  Fmi::Test::check_time_parse(should_pass, &TimeParser::parse_iso);
  Fmi::Test::check_time_parse_fail(should_fail, &TimeParser::parse_iso);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_epoch()
{
  using namespace Fmi;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      // Fmi::Date +%s --date="2007-01-02 05:00:00 UTC"  --> 1167714000
      {"1167714000", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      // Fmi::Date +%s --date="2000-02-28 05:15:00 UTC" --> 951714900
      {"951714900", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))},
      // Fmi::Date +%s --date="2019-08-23 07:05:33 UTC" --> 1566543933
      {"1566543933", Fmi::DateTime(Fmi::Date(2019, 8, 23), Fmi::Hours(7) + Fmi::Minutes(5) + Fmi::Seconds(33))},
      // Fmi::Date +%s --date="2019-08-22 07:01:59 UTC" --> 1566457319
      {"1566457319", Fmi::DateTime(Fmi::Date(2019, 8, 22), Fmi::Hours(7) + Fmi::Minutes(1) + Fmi::Seconds(59))},

      {"1400011680", Fmi::DateTime(Fmi::Date(2014, 05, 13), Fmi::Hours(20) + Fmi::Minutes(8))}};

  std::vector<std::string> should_fail = {"foobar", "12345678901"};

  Fmi::Test::check_time_parse(should_pass, &TimeParser::parse_epoch);
  Fmi::Test::check_time_parse_fail(should_fail, &TimeParser::parse_epoch);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_iso()
{
  using namespace Fmi;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      {"20070102T050000", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"20000228T051500", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))},
      {"2007-01-02T05:00:00", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"2000-02-28T05:15:00", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))},
      {"2000-02-28T05:15:10", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-02-28T05:15:10Z", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-02-28T05:15:10+01:00", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(4) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-02-28T05:15:10+01:30", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(3) + Fmi::Minutes(45) + Fmi::Seconds(10))},
      {"2000-02-28T02:15:10-01:30", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(3) + Fmi::Minutes(45) + Fmi::Seconds(10))},
      {"2000-02-28T051510", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"20000228T05:15:10", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-0228T0515:10", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"20000228T", Fmi::DateTime(Fmi::Date(2000, 2, 28))},
      {"20000228T12", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(12))}  // 3 fractional Fmi::Seconds are ignored
      ,
      {"2000-02-28T12:00:00.321", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(12))},
      {"2000-02-28T12:00:00.321+03:00", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(9))}  // BRAINSTORM-480
      ,
      {"2022-05-19T21:00:00.000Z", Fmi::DateTime(Fmi::Date(2022, 5, 19), Fmi::Hours(21))},
      {"2015-06-09T16:00:00+03", Fmi::DateTime(Fmi::Date(2015, 6, 9), Fmi::Hours(13))},
      {"2015-06-09T16:00:00+03:00", Fmi::DateTime(Fmi::Date(2015, 6, 9), Fmi::Hours(13))},
      {"2015-06-09T16:00:00+0300", Fmi::DateTime(Fmi::Date(2015, 6, 9), Fmi::Hours(13))},
      {"2015-06-09T08:00:00-05:30", Fmi::DateTime(Fmi::Date(2015, 6, 9), Fmi::Hours(13) + Fmi::Minutes(30))}
#ifdef WE_DO_NOT_SUPPORT_FRACTIONS
      ,
      {"2000-02-28T05:15:10.123456",
       Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10) + microFmi::Seconds(123456))},
      {"2000-02-28T05:15:10.987654Z",
       Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10) + microFmi::Seconds(987654))}

#endif
      // the Gregorian calendar is officially introduced on 1582-10-15
      ,
      {"1582-10-15T00:00:00Z", Fmi::DateTime(Fmi::Date(1582, 10, 15), Fmi::Hours(0) + Fmi::Minutes(0) + Fmi::Seconds(0))}

      // BRAINSTORM-1071
      ,
      {"1900-1-1T00:00:00", Fmi::DateTime(Fmi::Date(1900, 1, 1))}};

  std::vector<std::string> should_fail = {
      "2000-02-28T12:00:00.3233",   // Expected to fail when given four fractional Fmi::Seconds
      "2000-02-28T12:00:00.33",     // Expected to fail when given two fractional Fmi::Seconds
      "2000-02-28T12:00:00.3"       // Expected to fail when given one fractional second
      "2015-06-09T16:00:00+00300",  // Should fail to parse '00300' timezone
      "2015-06-09T16:00:00--0300",  // Should fail to parse double minus in timezone
      "2015-06-09T16:00:00%0300",   // Should fail to parse invalid sign in timezone
      "foobar",
      "12345678901",                // Too short
      "123456789012",               // month, day and hour are out of range
      "20160101T00000"  // Should fail to parse 20160101T00000 due to an extra zero (BRAINSTORM-696)
  };

  Fmi::Test::check_time_parse(should_pass, &TimeParser::parse_iso);
  Fmi::Test::check_time_parse_fail(should_fail, &TimeParser::parse_iso);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_fmi()
{
  using namespace Fmi;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      {"200912120500", Fmi::DateTime(Fmi::Date(2009, 12, 12), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"180001011500", Fmi::DateTime(Fmi::Date(1800, 1, 1), Fmi::Hours(15) + Fmi::Minutes(0))}};

  std::vector<std::string> should_fail = {
      "2009121205",      // Should fail to parse '2009121205', due to missing minute definition
      "200912120500334"  // Should fail to parse '200912120500334', due to extra tokens
  };

  Fmi::Test::check_time_parse(should_pass, &TimeParser::parse_fmi);
  Fmi::Test::check_time_parse_fail(should_fail, &TimeParser::parse_fmi);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void try_parse_iso()
{
  using namespace Fmi;

  bool utc;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      {"20070102T050000", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"20000228T051500", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))},
      {"2000-02-28T05:15:10Z", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-02-28T05:15:10+01:00", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(4) + Fmi::Minutes(15) + Fmi::Seconds(10))},
      {"2000-02-28T05:15:10-01:30",
       Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(6) + Fmi::Minutes(45) + Fmi::Seconds(10))}};

  Fmi::Test::check_time_parse(should_pass,
                              std::bind(&TimeParser::try_parse_iso, std::placeholders::_1, &utc));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_sql()
{
  using namespace Fmi;

  std::vector<Fmi::Test::TimeParseTest<Fmi::DateTime> > should_pass = {
      {"2007-01-02 05:00:00", Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0))},
      {"2000-02-28 05:15:00", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15))},
      {"2000-02-28 05", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5))},
      {"2000-02-28 05:20:34", Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(20) + Fmi::Seconds(34))},
      {"2000-02-28", Fmi::DateTime(Fmi::Date(2000, 2, 28))},
      {"1900-01-01 00:00:0.0", Fmi::DateTime(Fmi::Date(1900, 1, 1))}};

  Fmi::Test::check_time_parse(should_pass, &TimeParser::parse_sql);

  const std::vector<std::string> invalid = {"foobar",
                                            "12345678901",
                                            "1970-0-1 00:00:00",
                                            "1970-13-1 00:00:00",
                                            "1970-001-1 00:00:00",
                                            "1970-1-0 00:00:00",
                                            "1970-1-32 00:00:00",
                                            "1970-1-1 24:00:00",
                                            "1970-1-1 00:60:00",
                                            "1970-1-1 23:00:60",
                                            "1970-1-1 23:059:00",
                                            "1970-01-01 00:000:00"};

  Fmi::Test::check_time_parse_fail(invalid, &TimeParser::parse_sql);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0));
  if ((res = TimeParser::parse("200701020500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15));
  if ((res = TimeParser::parse("200002280515")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0));
  if ((res = TimeParser::parse("20070102T050000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15));
  if ((res = TimeParser::parse("20000228T051500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0));
  if ((res = TimeParser::parse("2007-01-02 05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15));
  if ((res = TimeParser::parse("2000-02-28 05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2007-01-02 05:00:00 UTC"  --> 1167714000
  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0));
  if ((res = TimeParser::parse("1167714000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2000-02-28 05:15:00 UTC" --> 951714900
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15));
  if ((res = TimeParser::parse("951714900")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Fmi::Hours(5) + Fmi::Minutes(0));
  if ((res = TimeParser::parse("2007-01-02T05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Fmi::Hours(5) + Fmi::Minutes(15));
  if ((res = TimeParser::parse("2000-02-28T05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2013, 5, 15), Fmi::Hours(13) + Fmi::Minutes(17) + Fmi::Seconds(23));
  if ((res = TimeParser::parse("20130515131723")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2013, 5, 15), Fmi::Hours(13) + Fmi::Minutes(17));
  if ((res = TimeParser::parse("201305151317")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2019-08-23 07:05:33 UTC" --> 1566543933
  ok = Fmi::DateTime(Fmi::Date(2019, 8, 23), Fmi::Hours(7) + Fmi::Minutes(5) + Fmi::Seconds(33));
  if ((res = TimeParser::parse("1566543933")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2019-08-22 07:01:59 UTC" --> 1566457319
  ok = Fmi::DateTime(Fmi::Date(2019, 8, 22), Fmi::Hours(7) + Fmi::Minutes(1) + Fmi::Seconds(59));
  if ((res = TimeParser::parse("1566457319")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  TimeParser::parse("-10h");
  TimeParser::parse("+12h");
  TimeParser::parse("+13H");
  TimeParser::parse("-14H");

  TimeParser::parse("-10m");
  TimeParser::parse("+12m");
  TimeParser::parse("+13M");
  TimeParser::parse("-14M");

  try {
      auto result = TimeParser::parse("202301dropthedb");
      TEST_FAILED("TimeParser::parse(\"202301dropthedb\") is expected to fail (got "
          + to_iso_string(result));
  } catch (const tframe::failed&) { throw;
  } catch (...) {}

  try {
      auto result = TimeParser::parse("2023060bb60000");
      TEST_FAILED("TimeParser::parse(\"2023060bb60000\") is expected to fail (got "
          + to_iso_string(result));
  } catch (const tframe::failed&) { throw;
  } catch (...) {}

  TEST_PASSED();
}

void parse_http()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  ok = Fmi::DateTime(Fmi::Date(1994, 11, 6), Fmi::Hours(8) + Fmi::Minutes(49) + Fmi::Seconds(37));

  if ((res = TimeParser::parse_http("Sun, 06 Nov 1994 08:49:37 GMT")) != ok)
    TEST_FAILED("Failed to parse Sun, 06 Nov 1994 08:49:37 GMT");

  if ((res = TimeParser::parse_http("Sunday, 06-Nov-94 08:49:37 GMT")) != ok)
    TEST_FAILED("Failed to parse Sunday, 06-Nov-94 08:49:37 GMT");

  if ((res = TimeParser::parse_http("Sun Nov  6 08:49:37 1994")) != ok)
    TEST_FAILED("Failed to parse Sun Nov  6 08:49:37 1994");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse offsets
 */
// ----------------------------------------------------------------------

void try_parse_offset()
{
  using namespace Fmi;

  // Now rounded to closest minute
  Fmi::DateTime now = Fmi::SecondClock::universal_time();
  Fmi::TimeDuration tnow = now.time_of_day();
  int secs = tnow.seconds();
  if (secs >= 30)
    now += Fmi::Seconds(60 - secs);
  else
    now -= Fmi::Seconds(secs);

  Fmi::DateTime res;

  res = TimeParser::try_parse_offset("0");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("0m");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0m correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("0h");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0h correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("0d");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0d correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+1");
  if (res - now != Fmi::Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+10m");
  if (res - now != Fmi::Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+3h");
  if (res - now != Fmi::Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("-24h");
  if (now - res != Fmi::Hours(24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+2d");
  if (res - now != Fmi::Hours(48))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+2y");
  if (res - now != Fmi::Hours(2 * 365 * 24))
    TEST_FAILED("Failed to parse +2y correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+2w");
  if (res - now != Fmi::Hours(2 * 24 * 7))
    TEST_FAILED("Failed to parse +2w correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("+2H");
  if (res - now != Fmi::Hours(2))
    TEST_FAILED("Failed to parse +2H correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("1");
  if (!res.is_not_a_date_time())
    TEST_FAILED("Should have failed to parse 1 as an offset, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset(" 123 ");
  if (!res.is_not_a_date_time())
    TEST_FAILED("Should have failed to parse ' 123 ' as an offset, got " + to_simple_string(res));

  res = TimeParser::try_parse_offset("-a");
  if (!res.is_not_a_date_time())
    TEST_FAILED("Should have failed to parse '-a' as an offset, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse offsets
 */
// ----------------------------------------------------------------------

void parse_offset()
{
  using namespace Fmi;

  // Now rounded to closest minute
  Fmi::DateTime now = Fmi::SecondClock::universal_time();
  Fmi::TimeDuration tnow = now.time_of_day();
  int secs = tnow.seconds();
  if (secs >= 30)
    now += Fmi::Seconds(60 - secs);
  else
    now -= Fmi::Seconds(secs);

  Fmi::DateTime res;

  res = TimeParser::parse("0");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = TimeParser::parse("0m");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0m correctly, got " + to_simple_string(res));

  res = TimeParser::parse("0h");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0h correctly, got " + to_simple_string(res));

  res = TimeParser::parse("0d");
  if (res - now != Fmi::Minutes(0))
    TEST_FAILED("Failed to parse 0d correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+1");
  if (res - now != Fmi::Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+10m");
  if (res - now != Fmi::Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+3h");
  if (res - now != Fmi::Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = TimeParser::parse("-24h");
  if (now - res != Fmi::Hours(24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+2d");
  if (res - now != Fmi::Hours(48))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+2y");
  if (res - now != Fmi::Hours(2 * 365 * 24))
    TEST_FAILED("Failed to parse +2y correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+2w");
  if (res - now != Fmi::Hours(2 * 24 * 7))
    TEST_FAILED("Failed to parse +2w correctly, got " + to_simple_string(res));

  res = TimeParser::parse("+2H");
  if (res - now != Fmi::Hours(2))
    TEST_FAILED("Failed to parse +2H correctly, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse durations
 */
// ----------------------------------------------------------------------

void try_parse_duration()
{
  using namespace Fmi;

  Fmi::TimeDuration res;

  res = TimeParser::try_parse_duration("0");
  if (res != Fmi::Seconds(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("+1");
  if (res != Fmi::Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("+10m");
  if (res != Fmi::Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("+3h");
  if (res != Fmi::Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("-24h");
  if (res != Fmi::Hours(-24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("+2d");
  if (res != Fmi::Hours(2 * 24))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("+2W");
  if (res != Fmi::Hours(7 * 2 * 24))
    TEST_FAILED("Failed to parse +2W correctly, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration("123");
  if (!res.is_not_a_date_time())
    TEST_FAILED("Should have failed to parse 123 as duration, got " + to_simple_string(res));

  res = TimeParser::try_parse_duration(" -123 ");
  if (!res.is_not_a_date_time())
    TEST_FAILED("Should have failed to parse ' -123' as duration, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse durations
 */
// ----------------------------------------------------------------------

void parse_duration()
{
  using namespace Fmi;

  Fmi::TimeDuration res;

  res = TimeParser::parse_duration("0");
  if (res != Fmi::Seconds(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("+1");
  if (res != Fmi::Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("+10m");
  if (res != Fmi::Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("+3h");
  if (res != Fmi::Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("-24h");
  if (res != Fmi::Hours(-24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("+2d");
  if (res != Fmi::Hours(2 * 24))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = TimeParser::parse_duration("+2W");
  if (res != Fmi::Hours(7 * 2 * 24))
    TEST_FAILED("Failed to parse +2W correctly, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse ISO8601 durations
 */
// ----------------------------------------------------------------------

void parse_iso_duration()
{
  using namespace Fmi;

  Fmi::TimeDuration res;

  res = TimeParser::parse_iso_duration("P2W");
  if (res != Fmi::Hours(7 * 2 * 24))
    TEST_FAILED("Failed to parse P2W correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("P1D");
  if (res != Fmi::Hours(24))
    TEST_FAILED("Failed to parse P1D correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("P2D");
  if (res != Fmi::Hours(2 * 24))
    TEST_FAILED("Failed to parse P2D correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("PT0H");
  if (res != Fmi::Seconds(0))
    TEST_FAILED("Failed to parse PT0H correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("PT1M");
  if (res != Fmi::Minutes(1))
    TEST_FAILED("Failed to parse PT1M correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("PT10M");
  if (res != Fmi::Minutes(10))
    TEST_FAILED("Failed to parse PT10M correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("PT3H");
  if (res != Fmi::Hours(3))
    TEST_FAILED("Failed to parse PT3H correctly, got " + to_simple_string(res));

  res = TimeParser::parse_iso_duration("P1DT10H20M30S");
  if (res != Fmi::Hours(24) + Fmi::Hours(10) + Fmi::Minutes(20) + Fmi::Seconds(30))
    TEST_FAILED("Failed to parse P1DT10H20M30S correctly, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test time format recognizition
 */
// ----------------------------------------------------------------------

void looks()
{
  using namespace Fmi;

  std::string res;

  if ((res = TimeParser::looks("200701020500")) != "iso")
    TEST_FAILED("200701020500 should look like an ISO time stamp");

  if ((res = TimeParser::looks("20000228T051500")) != "iso")
    TEST_FAILED("20000228T051500 should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2007-01-02T05:00:00")) != "iso")
    TEST_FAILED("2007-01-02T05:00:00 should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28T05:15:00")) != "iso")
    TEST_FAILED("2000-02-28T05:15:00 should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28T05:15:10")) != "iso")
    TEST_FAILED("2000-02-28T05:15:10 should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28T05:15:10Z")) != "iso")
    TEST_FAILED("2000-02-28T05:15:10Z should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28T05:15:10+01:00")) != "iso")
    TEST_FAILED("2000-02-28T05:15:10Z should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28T05:15:10-02")) != "iso")
    TEST_FAILED("2000-02-28T05:15:10Z should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2022-05-19T21:00:00.000Z")) != "iso")
    TEST_FAILED("2022-05-19T21:00:00.000Z should look like an ISO Fmi::Date time");

  if ((res = TimeParser::looks("2007-01-02 05:00:00")) != "sql")
    TEST_FAILED("2007-01-02 05:00:00 should look like a SQL Fmi::Date time");

  if ((res = TimeParser::looks("2000-02-28 05:15:00")) != "sql")
    TEST_FAILED("2000-02-28 05:15:00 should look like a SQL Fmi::Date time");

  if ((res = TimeParser::looks("1167714000")) != "epoch")
    TEST_FAILED("1167714000 should look like an epoch time");

  if ((res = TimeParser::looks("1357063111")) != "epoch")
    TEST_FAILED("1357063111 should look like an epoch time");

  if ((res = TimeParser::looks("+10")) != "offset")
    TEST_FAILED("+10 should look like an offset");
  if ((res = TimeParser::looks("-100")) != "offset")
    TEST_FAILED("-100 should look like an offset");
  if ((res = TimeParser::looks("+10m")) != "offset")
    TEST_FAILED("+10m should look like an offset");
  if ((res = TimeParser::looks("-100H")) != "offset")
    TEST_FAILED("-100H should look like an offset");

  if ((res = TimeParser::looks("1900-01-01 00:00:0.0")) != "sql")
    TEST_FAILED("1900-01-01 00:00:0.0 should look like SQL Fmi::Date time");

  try
  {
    res = TimeParser::looks("foobar");
    TEST_FAILED("foobar should not look like a Fmi::Date time");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UTC time format recognizition
 */
// ----------------------------------------------------------------------

void looks_utc()
{
  using namespace Fmi;

  std::vector<Test::TimeParseTest<bool> > data = {{"200701020500", false},
                                                  {"20000228T051500", false},
                                                  {"2007-01-02T05:00:00", false},
                                                  {"2000-02-28T05:15:00", false},
                                                  {"2000-02-28T05:15:10", false},
                                                  {"2000-02-28T05:15:10Z", true},
                                                  {"2000-02-28T05:15:10+01:00", true},
                                                  {"2000-02-28T05:15:10-02", true},
                                                  {"2007-01-02 05:00:00", false},
                                                  {"2000-02-28 05:15:00", false},
                                                  {"1167714000", true}};

  for (const auto& item : data)
  {
    if (item.expected ^ TimeParser::looks_utc(item.src))
    {
      TEST_FAILED(
          (item.src + " should look like " + (item.expected ? "UTC" : "local") + " time").c_str());
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_wintertime()
{
  using namespace Fmi;

  auto zone = TimeZoneFactory::instance().time_zone_from_string("Europe/Helsinki");

  Fmi::LocalDateTime ok = Fmi::LocalDateTime(
      Fmi::Date(2012, 10, 27), Fmi::Hours(3) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  Fmi::LocalDateTime res = TimeParser::parse("201210270330", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = Fmi::LocalDateTime(
      Fmi::Date(2012, 10, 27), Fmi::Hours(2) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = TimeParser::parse("201210270230", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = Fmi::LocalDateTime(
      Fmi::Date(2012, 10, 27), Fmi::Hours(1) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = TimeParser::parse("201210270130", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  TEST_PASSED();
}

void parse_summertime()
{
  using namespace Fmi;

  auto zone = TimeZoneFactory::instance().time_zone_from_string("Europe/Helsinki");

  Fmi::LocalDateTime ok = Fmi::LocalDateTime(
      Fmi::Date(2013, 3, 31), Fmi::Hours(1) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  Fmi::LocalDateTime res = TimeParser::parse("201303310130", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = Fmi::LocalDateTime(
      Fmi::Date(2013, 3, 31), Fmi::Hours(2) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = TimeParser::parse("201303310230", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

#if 0
  // FIXME: make_time behavior has changed (it returns now 04:00 - closest valid time)
  //        disable test for now
  // We expect 04:30 for invalid 03:30!!
  ok = Fmi::LocalDateTime(
      Fmi::Date(2013, 3, 31), Fmi::Hours(4) + Fmi::Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = TimeParser::parse("201303310330", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));
#endif
  TEST_PASSED();
}

void parse_errors()
{
  using namespace Fmi;

  try
  {
    std::string input = "202301dropthedb";
    auto res = TimeParser::parse(input);
    TEST_FAILED("Should not return result " + tostring(res) + " for input " + input);
  }
  catch (...)
  {
  }

  try
  {
    std::string input = "2023060bb60000";
    auto res = TimeParser::parse(input);
    TEST_FAILED("Should not return result " + tostring(res) + " for input " + input);
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(parse_wintertime);
    TEST(parse_summertime);
    TEST(parse_timestamp);
    TEST(try_parse_iso);
    TEST(parse_iso);
    TEST(parse_fmi);
    TEST(parse_sql);
    TEST(parse_epoch);
    TEST(parse);
    TEST(parse_http);
    TEST(try_parse_offset);
    TEST(parse_offset);
    TEST(looks);
    TEST(looks_utc);
    TEST(try_parse_duration);
    TEST(parse_duration);
    TEST(parse_iso_duration);
    TEST(parse_errors);
  }
};

}  // namespace TimeParserTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "TimeParser tester" << endl << "=================" << endl;
  TimeParserTest::tests t;
  return t.run();
}

// ======================================================================
