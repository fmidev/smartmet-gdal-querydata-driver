// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiMetTime
 */
// ======================================================================

#include "NFmiMetTime.h"
#include <regression/tframe.h>

using namespace std;
using Fmi::date_time::time_from_string;

//! Protection against conflicts with global functions
namespace NFmiMetTimeTest
{
// ----------------------------------------------------------------------

void construct_from_ptime()
{
  using Fmi::DateTime;

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 03:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025030000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025030000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 04:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025040000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025040000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 05:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025050000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025050000");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void construct_from_local_date_time()
{
  using Fmi::LocalDateTime;
  using Fmi::TimeZonePtr;
  using Fmi::DateTime;

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 03:00:00");
    Fmi::TimeZonePtr zone("Europe/Stockholm");
    Fmi::LocalDateTime ldt(pt, zone);

    NFmiMetTime t(ldt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025030000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025030000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 05:00:00");
    Fmi::TimeZonePtr zone("Europe/Helsinki");
    Fmi::LocalDateTime ldt(pt, zone);

    NFmiMetTime t(ldt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025050000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025050000");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void posixtime()
{
  using Fmi::DateTime;

  NFmiMetTime t(2013, 10, 25, 1, 2, 3, 0);
  Fmi::DateTime res = t.PosixTime();
  if (res != time_from_string("2013-10-25 01:02:00"))
    TEST_FAILED("Incorrect result " + to_simple_string(res) + " for " +
                t.ToStr(kYYYYMMDDHHMMSS).CharPtr());

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void implicit_conversion()
{
  using Fmi::DateTime;

  NFmiMetTime t(2013, 10, 25, 1, 2, 3, 0);
  Fmi::DateTime res = t;
  if (res != time_from_string("2013-10-25 01:02:00"))
    TEST_FAILED("Incorrect result " + to_simple_string(res) + " for " +
                t.ToStr(kYYYYMMDDHHMMSS).CharPtr());

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
    TEST(posixtime);
    TEST(implicit_conversion);
    TEST(construct_from_ptime);
    TEST(construct_from_local_date_time);
  }
};

}  // namespace NFmiMetTimeTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiMetTime tester" << endl << "================" << endl;
  NFmiMetTimeTest::tests t;
  return t.run();
}

// ======================================================================
