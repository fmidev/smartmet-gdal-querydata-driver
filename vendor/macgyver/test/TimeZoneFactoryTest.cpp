// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::TimeZoneFactory
 */
// ======================================================================

#include "TimeZoneFactory.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <iostream>
#include <string>

using namespace std;
using namespace Fmi::date_time;

namespace TimeZoneFactoryTest
{
// ----------------------------------------------------------------------
/*
 * Test region list
 */
// ----------------------------------------------------------------------

void region_list()
{
  using boost::lexical_cast;

  vector<string> lst = Fmi::TimeZoneFactory::instance().region_list();
  if (lst.size() < 460)
    TEST_FAILED("There should at least 460 regions, not " + lexical_cast<string>(lst.size()));

  if (lst.front() != "Africa/Abidjan")
    TEST_FAILED("First region should be Africa/Abidjan");

  if (lst.back() != "Zulu")
    TEST_FAILED("Last region should be Zulu, not " + lst.back());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test time_zone_from_region
 */
// ----------------------------------------------------------------------

void time_zone_from_region()
{
  using boost::lexical_cast;
  using namespace Fmi::date_time;

  TimeZonePtr tz = Fmi::TimeZoneFactory::instance().time_zone_from_region("Europe/Helsinki");
  LocalDateTime t01(DateTime(Date(2024, Feb, 7), TimeDuration(0, 0, 0)), tz);
  LocalDateTime t02(DateTime(Date(2024, Jun, 7), TimeDuration(0, 0, 0)), tz);

  if (tz->name() != "Europe/Helsinki")
    TEST_FAILED("Europe/Helsinki region should be Europe/Helsinki, not " + tz->name());
  if (t02.abbrev() != "EEST")
    TEST_FAILED("Europe/Helsinki dst_zone_abbrev should be EEST, not " + t02.abbrev());
  if (t01.abbrev() != "EET")
    TEST_FAILED("Europe/Helsinki std_zone_abbrev should be EET, not " + t01.abbrev());

// FIXME: not supported currently
#if 0
  string val;

  if ("2000-Mar-26 03:00:00" != (val = lexical_cast<string>(tz->dst_local_start_time(2000))))
    TEST_FAILED("dst_local_start_time should be 2000-Mar-26 03:00:00, not " + val);
  if ("2000-Oct-29 04:00:00" != (val = lexical_cast<string>(tz->dst_local_end_time(2000))))
    TEST_FAILED("dst_local_end_time should be 2000-Oct-29 04:00:00, not " + val);
  if ("02:00:00" != (val = lexical_cast<string>(tz->base_utc_offset())))
    TEST_FAILED("base_utc_offset should be 02:00:00, not " + val);
  if ("01:00:00" != (val = lexical_cast<string>(tz->dst_offset())))
    TEST_FAILED("dst_offset should be 01:00:00, not " + val);
#endif

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test time_zone_from_string
 */
// ----------------------------------------------------------------------

void time_zone_from_string()
{
  using boost::lexical_cast;
  using namespace Fmi::date_time;

  Fmi::DateTime time(Date(2024, Jan, 1), TimeDuration(0, 0, 0));

  // UTC

  {
    string posix = "UTC";
    string expected = "UTC";
    Fmi::TimeZonePtr tz = Fmi::TimeZoneFactory::instance().time_zone_from_string("UTC");

    Fmi::LocalDateTime ldt(time, tz);
    if (ldt.abbrev() != expected)
      TEST_FAILED("UTC string should be " + expected + ", not " + tz->name());
  }

  // Helsinki
  {
    string region = "Europe/Helsinki";
    string posix = "EET";

    Fmi::TimeZonePtr tz1 = Fmi::TimeZoneFactory::instance().time_zone_from_string(region);
    if (tz1->name() != region)
      TEST_FAILED(region + " string should be " + region + ", not " + tz1->name());

    Fmi::TimeZonePtr tz2 = Fmi::TimeZoneFactory::instance().time_zone_from_string(posix);
    LocalDateTime t01(time, tz2);
    if (t01.abbrev() != posix)
      TEST_FAILED(posix + " string should be " + posix + ", not " + t01.abbrev());
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test time_zone_from_coordinate
 */
// ----------------------------------------------------------------------

void time_zone_from_coordinate()
{
  using boost::lexical_cast;
  using namespace Fmi::date_time;

  string ok1 = "CET";
  Fmi::TimeZonePtr tz1 = Fmi::TimeZoneFactory::instance().time_zone_from_coordinate(17, 60);
  LocalDateTime ldt1(DateTime(Date(2024, Feb, 7), TimeDuration(0, 0, 0)), tz1);
  if (ldt1.abbrev() != ok1)
    TEST_FAILED("17,60 string should be " + ok1 + ", not " + ldt1.abbrev());

  string ok2 = "EET";
  Fmi::TimeZonePtr tz2 = Fmi::TimeZoneFactory::instance().time_zone_from_coordinate(25, 60);
  LocalDateTime ldt2(DateTime(Date(2024, Feb, 7), TimeDuration(0, 0, 0)), tz2);
  if (ldt2.abbrev() != ok2)
    TEST_FAILED("25,60 string should be " + ok2 + ", not " + ldt2.abbrev());

  string ok3 = "EET";
  Fmi::TimeZonePtr tz3 = Fmi::TimeZoneFactory::instance().time_zone_from_coordinate(21.3705, 59.7811);
  LocalDateTime ldt3(DateTime(Date(2024, Feb, 7), TimeDuration(0, 0, 0)), tz3);
  if (ldt3.abbrev() != ok3)
    TEST_FAILED("21.3705,59.7811 string should be " + ok3 + ", not " + ldt3.abbrev());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test zone_name_from_coordinate
 */
// ----------------------------------------------------------------------

void zone_name_from_coordinate()
{
  using boost::lexical_cast;
  using Fmi::TimeZonePtr;

  string ok1 = "Europe/Stockholm";
  string tz1 = Fmi::TimeZoneFactory::instance().zone_name_from_coordinate(17, 60);
  if (tz1 != ok1)
    TEST_FAILED("17,60 string should be " + ok1 + ", not " + tz1);

  string ok2 = "Europe/Helsinki";
  string tz2 = Fmi::TimeZoneFactory::instance().zone_name_from_coordinate(25, 60);
  if (tz2 != ok2)
    TEST_FAILED("25,60 string should be " + ok2 + ", not " + tz2);

  string ok3 = "Europe/Helsinki";
  string tz3 = Fmi::TimeZoneFactory::instance().zone_name_from_coordinate(21.3705, 59.7811);
  if (tz3 != ok3)
    TEST_FAILED("21.3705,59.7811 string should be " + ok3 + ", not " + tz3);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test a coordinate in the Atlantic ocean
 */
// ----------------------------------------------------------------------

void ocean()
{
  using boost::lexical_cast;
  using Fmi::TimeZonePtr;

  string tz = Fmi::TimeZoneFactory::instance().zone_name_from_coordinate(-47, 30);
  if (tz != "Etc/GMT-3")
    TEST_FAILED("-46,30 string should be Etc/GMT-3, not " + tz);

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
    TEST(region_list);
    TEST(time_zone_from_region);
    TEST(time_zone_from_coordinate);
    TEST(time_zone_from_string);
    TEST(zone_name_from_coordinate);
    // Disabled: waiting for fix to brainstorm-912
    // TEST(ocean);
  }
};

}  // namespace TimeZoneFactoryTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "TimeZoneFactory" << endl << "===============" << endl;
  TimeZoneFactoryTest::tests t;
  return t.run();
}

// ======================================================================
