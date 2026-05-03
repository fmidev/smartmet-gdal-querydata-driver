// ======================================================================
/*!
 * \file
 * \brief Regression tests for class DateTimeParser
 */
// ======================================================================

#include "DateTimeParser.h"
#include "TimeZoneFactory.h"
#include <regression/tframe.h>

using Fmi::date_time::Hours;
using Fmi::date_time::Minutes;
using Fmi::date_time::Seconds;

namespace second_clock = Fmi::date_time::SecondClock;

template <typename T>
std::string tostring(const T& obj)
{
  std::ostringstream out;
  out << obj;
  return out.str();
}

// Protection against conflicts with global functions
namespace DateTimeParserTest
{
// ----------------------------------------------------------------------

void parse_timestamp()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("20070102T0500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("20000228T0515")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("foobar");
    TEST_FAILED("Should fail to parse 'foobar', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("12345678901");
    TEST_FAILED("Should fail to parse '12345678901', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("123456789012");
    TEST_FAILED("Should fail to parse '123456789012', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_epoch()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  // Fmi::Date +%s --date="2007-01-02 05:00:00 UTC"  --> 1167714000
  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("1167714000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2000-02-28 05:15:00 UTC" --> 951714900
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("951714900")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("foobar");
    TEST_FAILED("Should fail to parse 'foobar', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("12345678901");
    TEST_FAILED("Should fail to parse '12345678901', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  // This looks like a valid Fmi::Datetime:

  ok = Fmi::DateTime(Fmi::Date(2014, 05, 13), Hours(20) + Minutes(8));
  if ((res = parser.parse("1400011680")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_iso()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("20070102T050000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("20000228T051500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("2007-01-02T05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("2000-02-28T05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("2000-02-28T05:15:10")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("2000-02-28T05:15:10Z")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(4) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("2000-02-28T05:15:10+01:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(3) + Minutes(45) + Seconds(10));
  if ((res = parser.parse("2000-02-28T05:15:10+01:30")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(3) + Minutes(45) + Seconds(10));
  if ((res = parser.parse("2000-02-28T02:15:10-01:30")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("2000-02-28T051510")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("20000228T05:15:10")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10));
  if ((res = parser.parse("2000-0228T0515:10")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28));
  if ((res = parser.parse("20000228T")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(12));
  if ((res = parser.parse("20000228T12")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fractional seconds are ignored
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(12));
  if ((res = parser.parse("2000-02-28T12:00:00.321")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(9));
  if ((res = parser.parse("2000-02-28T12:00:00.321+03:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fractional seconds are ignored
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(12));
  try
  {
    res = parser.parse("2000-02-28T12:00:00.3233");
    TEST_FAILED("Expected to fail when given four fractional seconds");

    res = parser.parse("2000-02-28T12:00:00.33");
    TEST_FAILED("Expected to fail when given two fractional seconds");

    res = parser.parse("2000-02-28T12:00:00.33");
    TEST_FAILED("Expected to fail when given one fractional second");
  }
  catch (...)
  {
  }

  // BRAINSTORM-480
  ok = Fmi::DateTime(Fmi::Date(2015, 6, 9), Hours(13));
  if ((res = parser.parse("2015-06-09T16:00:00+03")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));
  if ((res = parser.parse("2015-06-09T16:00:00+03:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));
  if ((res = parser.parse("2015-06-09T16:00:00+0300")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2015, 6, 9), Hours(13) + Minutes(30));
  if ((res = parser.parse("2015-06-09T08:00:00-05:30")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  if ((res = parser.parse("2015-06-09T08:00:00-0530")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("2015-06-09T16:00:00+00300");
    TEST_FAILED("Should fail to parse '00300' timezone, got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("2015-06-09T16:00:00--0300");
    TEST_FAILED("Should fail to parse double minus in timezone, got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("2015-06-09T16:00:00%0300");
    TEST_FAILED("Should fail to parse invalid sign in timezone, got " + to_simple_string(res));
  }
  catch (...)
  {
  }

#ifdef WE_DO_NOT_SUPPORT_FRACTIONS
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10) + microSeconds(123456));
  if ((res = parser.parse("2000-02-28T05:15:10.123456")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15) + Seconds(10) + microSeconds(987654));
  if ((res = parser.parse("2000-02-28T05:15:10.987654Z")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));
#endif
  // the Gregorian calendar is officially introduced on 1582-10-15
  ok = Fmi::DateTime(Fmi::Date(1582, 10, 15), Hours(0) + Minutes(0) + Seconds(0));
  if ((res = parser.parse("1582-10-15T00:00:00Z")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("foobar");
    TEST_FAILED("Should fail to parse 'foobar', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("12345678901");
    TEST_FAILED("Should fail to parse '12345678901', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("foobar");
    TEST_FAILED("Should fail to parse 'foobar', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("12345678901");
    TEST_FAILED("Should fail to parse '12345678901', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("123456789012");
    TEST_FAILED("Should fail to parse '123456789012', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  // BRAINSTORM-696

  try
  {
    res = parser.parse("20160101T00000");  // extra zero
    TEST_FAILED("Should fail to parse 20160101T00000 due to an extra zero, got " +
                to_simple_string(res));
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_fmi()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(2009, 12, 12), Hours(5) + Minutes(0));
  if ((res = parser.parse("200912120500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(1800, 1, 1), Hours(15) + Minutes(0));
  if ((res = parser.parse("180001011500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("2009121205");
    TEST_FAILED("Should fail to parse '2009121205', due to missing minute definition");
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("200912120500334");
    TEST_FAILED("Should fail to parse '200912120500334', due to extra tokens");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_sql()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("2007-01-02 05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("2000-02-28 05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5));
  if ((res = parser.parse("2000-02-28 05")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(20) + Seconds(34));
  if ((res = parser.parse("2000-02-28 05:20:34")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28));
  if ((res = parser.parse("2000-02-28")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  try
  {
    res = parser.parse("foobar");
    TEST_FAILED("Should fail to parse 'foobar', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  try
  {
    res = parser.parse("12345678901");
    TEST_FAILED("Should fail to parse '12345678901', got " + to_simple_string(res));
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("200701020500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("200002280515")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("20070102T050000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("20000228T051500")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("2007-01-02 05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("2000-02-28 05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2007-01-02 05:00:00 UTC"  --> 1167714000
  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("1167714000")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  // Fmi::Date +%s --date="2000-02-28 05:15:00 UTC" --> 951714900
  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("951714900")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2007, 1, 2), Hours(5) + Minutes(0));
  if ((res = parser.parse("2007-01-02T05:00:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2000, 2, 28), Hours(5) + Minutes(15));
  if ((res = parser.parse("2000-02-28T05:15:00")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2013, 5, 15), Hours(13) + Minutes(17) + Seconds(23));
  if ((res = parser.parse("20130515131723")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  ok = Fmi::DateTime(Fmi::Date(2013, 5, 15), Hours(13) + Minutes(17));
  if ((res = parser.parse("201305151317")) != ok)
    TEST_FAILED("Expected " + to_simple_string(ok) + ", got " + to_simple_string(res));

  parser.parse("-10h");
  parser.parse("+12h");
  parser.parse("+13H");
  parser.parse("-14H");

  parser.parse("-10m");
  parser.parse("+12m");
  parser.parse("+13M");
  parser.parse("-14M");

  TEST_PASSED();
}

void parse_http()
{
  using namespace Fmi;

  Fmi::DateTime res, ok;

  DateTimeParser parser;

  ok = Fmi::DateTime(Fmi::Date(1994, 11, 6), Hours(8) + Minutes(49) + Seconds(37));

  if ((res = parser.parse_http("Sun, 06 Nov 1994 08:49:37 GMT")) != ok)
    TEST_FAILED("Failed to parse Sun, 06 Nov 1994 08:49:37 GMT");

  if ((res = parser.parse_http("Sunday, 06-Nov-94 08:49:37 GMT")) != ok)
    TEST_FAILED("Failed to parse Sunday, 06-Nov-94 08:49:37 GMT");

  if ((res = parser.parse_http("Sun Nov  6 08:49:37 1994")) != ok)
    TEST_FAILED("Failed to parse Sun Nov  6 08:49:37 1994");

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

  DateTimeParser parser;

  // Now rounded to closest minute
  Fmi::DateTime now = second_clock::universal_time();
  Fmi::TimeDuration tnow = now.time_of_day();
  int secs = tnow.seconds();
  if (secs >= 30)
    now += Seconds(60 - secs);
  else
    now -= Seconds(secs);

  Fmi::DateTime res, ok;

  res = parser.parse("0");
  if (res - now != Minutes(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = parser.parse("0m");
  if (res - now != Minutes(0))
    TEST_FAILED("Failed to parse 0m correctly, got " + to_simple_string(res));

  res = parser.parse("0h");
  if (res - now != Minutes(0))
    TEST_FAILED("Failed to parse 0h correctly, got " + to_simple_string(res));

  res = parser.parse("0d");
  if (res - now != Minutes(0))
    TEST_FAILED("Failed to parse 0d correctly, got " + to_simple_string(res));

  res = parser.parse("+1");
  if (res - now != Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = parser.parse("+10m");
  if (res - now != Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = parser.parse("+3h");
  if (res - now != Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = parser.parse("-24h");
  if (now - res != Hours(24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = parser.parse("+2d");
  if (res - now != Hours(48))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = parser.parse("+2y");
  if (res - now != Hours(2 * 365 * 24))
    TEST_FAILED("Failed to parse +2y correctly, got " + to_simple_string(res));

  res = parser.parse("+2w");
  if (res - now != Hours(2 * 24 * 7))
    TEST_FAILED("Failed to parse +2w correctly, got " + to_simple_string(res));

  res = parser.parse("+2H");
  if (res - now != Hours(2))
    TEST_FAILED("Failed to parse +2H correctly, got " + to_simple_string(res));

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

  Fmi::TimeDuration res, ok;

  DateTimeParser parser;

  res = parser.parse_duration("0");
  if (res != Seconds(0))
    TEST_FAILED("Failed to parse 0 correctly, got " + to_simple_string(res));

  res = parser.parse_duration("+1");
  if (res != Minutes(1))
    TEST_FAILED("Failed to parse +1 correctly, got " + to_simple_string(res));

  res = parser.parse_duration("+10m");
  if (res != Minutes(10))
    TEST_FAILED("Failed to parse +10m correctly, got " + to_simple_string(res));

  res = parser.parse_duration("+3h");
  if (res != Hours(3))
    TEST_FAILED("Failed to parse +3h correctly, got " + to_simple_string(res));

  res = parser.parse_duration("-24h");
  if (res != Hours(-24))
    TEST_FAILED("Failed to parse -24h correctly, got " + to_simple_string(res));

  res = parser.parse_duration("+2d");
  if (res != Hours(2 * 24))
    TEST_FAILED("Failed to parse +2d correctly, got " + to_simple_string(res));

  res = parser.parse_duration("+2W");
  if (res != Hours(7 * 2 * 24))
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

  DateTimeParser parser;

  res = parser.parse_iso_duration("P2W");
  if (res != Hours(7 * 2 * 24))
    TEST_FAILED("Failed to parse P2W correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("P1D");
  if (res != Hours(24))
    TEST_FAILED("Failed to parse P1D correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("P2D");
  if (res != Hours(2 * 24))
    TEST_FAILED("Failed to parse P2D correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("PT0H");
  if (res != Seconds(0))
    TEST_FAILED("Failed to parse PT0H correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("PT1M");
  if (res != Minutes(1))
    TEST_FAILED("Failed to parse PT1M correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("PT10M");
  if (res != Minutes(10))
    TEST_FAILED("Failed to parse PT10M correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("PT3H");
  if (res != Hours(3))
    TEST_FAILED("Failed to parse PT3H correctly, got " + to_simple_string(res));

  res = parser.parse_iso_duration("P1DT10H20M30S");
  if (res != Hours(24) + Hours(10) + Minutes(20) + Seconds(30))
    TEST_FAILED("Failed to parse P1DT10H20M30S correctly, got " + to_simple_string(res));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void parse_wintertime()
{
  using namespace Fmi;

  DateTimeParser parser;

  auto zone = TimeZoneFactory::instance().time_zone_from_string("Europe/Helsinki");

  Fmi::LocalDateTime ok = LocalDateTime(
      Date(2012, 10, 27), Hours(3) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  Fmi::LocalDateTime res = parser.parse("201210270330", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = LocalDateTime(
      Date(2012, 10, 27), Hours(2) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = parser.parse("201210270230", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = LocalDateTime(
      Date(2012, 10, 27), Hours(1) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = parser.parse("201210270130", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  TEST_PASSED();
}

void parse_summertime()
{
  using namespace Fmi;
  DateTimeParser parser;

  auto zone = TimeZoneFactory::instance().time_zone_from_string("Europe/Helsinki");

  Fmi::LocalDateTime ok = Fmi::LocalDateTime(
      Date(2013, 3, 31), Hours(1) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  Fmi::LocalDateTime res = parser.parse("201303310130", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

  ok = Fmi::LocalDateTime(
      Date(2013, 3, 31), Hours(2) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = parser.parse("201303310230", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));

#if 0
  // FIXME: make_time behavior has changed (it returns now 04:00 - closest valid time)
  //        disable test for now
  // We expect 04:30 for invalid 03:30!!
  ok = Fmi::LocalDateTime(
      Date(2013, 3, 31), Hours(4) + Minutes(30), zone, Fmi::LocalDateTime::EXCEPTION_ON_ERROR);
  res = parser.parse("201303310330", zone);
  if (res != ok)
    TEST_FAILED("Expected " + tostring(ok) + ", got " + tostring(res));
#endif
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
    TEST(parse_iso);
    TEST(parse_fmi);
    TEST(parse_sql);
    TEST(parse_epoch);
    TEST(parse);
    TEST(parse_http);
    TEST(parse_offset);
    TEST(parse_duration);
    TEST(parse_iso_duration);
  }
};

}  // namespace DateTimeParserTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "DateTimeParser tester" << endl << "=====================" << endl;
  DateTimeParserTest::tests t;
  return t.run();
}

// ======================================================================
