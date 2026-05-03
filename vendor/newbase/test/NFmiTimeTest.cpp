// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiTime
 */
// ======================================================================

#include "NFmiStringTools.h"
#include "NFmiTime.h"
#include <regression/tframe.h>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiTimeTest
{
class MyTime : public NFmiTime
{
 public:
  MyTime() : NFmiTime() {}
  MyTime(short year, short month, short day, short hour = 0, short min = 0)
      : NFmiTime(year, month, day, hour, min)
  {
  }

  long MyGetCompareValue() const { return GetCompareValue(); }
  void MyDecodeCompareValue(long value) { DecodeCompareValue(value); }
};

// ----------------------------------------------------------------------
/*!
 * \brief Test DaysInYear
 */
// ----------------------------------------------------------------------

void daysinyear()
{
  using NFmiStringTools::Convert;

  NFmiTime t;

  if (t.DaysInYear(2005) != 365)
    TEST_FAILED("There are 365 days in year 2005");
  if (t.DaysInYear(2004) != 366)
    TEST_FAILED("There are 366 days in year 2004");
  if (t.DaysInYear(2003) != 365)
    TEST_FAILED("There are 365 days in year 2003");
  if (t.DaysInYear(2002) != 365)
    TEST_FAILED("There are 365 days in year 2002");
  if (t.DaysInYear(2001) != 365)
    TEST_FAILED("There are 365 days in year 2001");
  if (t.DaysInYear(2000) != 366)
    TEST_FAILED("There are 366 days in year 2000");
  if (t.DaysInYear(1999) != 365)
    TEST_FAILED("There are 365 days in year 1999");

  if (t.DaysInYear(1900) != 365)
    TEST_FAILED("There are 365 days in year 1900");
  if (t.DaysInYear(2100) != 365)
    TEST_FAILED("There are 365 days in year 2100");
  if (t.DaysInYear(2200) != 365)
    TEST_FAILED("There are 365 days in year 2200");
  if (t.DaysInYear(2400) != 366)
    TEST_FAILED("There are 366 days in year 2400");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test GetCompareValue
 */
// ----------------------------------------------------------------------

void getcomparevalue()
{
  using NFmiStringTools::Convert;

  long value;

  {
    MyTime t(2002, 1, 1);
    if ((value = t.MyGetCompareValue()) != 0)
      TEST_FAILED("Value for 1.1.2002 00:00 should be 0, not " + Convert(value));
  }

  {
    MyTime t(2002, 1, 1, 1, 0);
    if ((value = t.MyGetCompareValue()) != 60)
      TEST_FAILED("Value for 1.1.2002 01:00 should be 60, not " + Convert(value));
  }

  {
    MyTime t(2002, 1, 1, 2, 15);
    if ((value = t.MyGetCompareValue()) != 135)
      TEST_FAILED("Value for 1.1.2002 02:15 should be 135, not " + Convert(value));
  }

  {
    MyTime t(2002, 1, 2, 0, 0);
    if ((value = t.MyGetCompareValue()) != 1440)
      TEST_FAILED("Value for 2.1.2002 00:00 should be 1440, not " + Convert(value));
  }

  {
    MyTime t(2002, 2, 1, 0, 0);
    if ((value = t.MyGetCompareValue()) != 44640)
      TEST_FAILED("Value for 1.2.2002 00:00 should be 44640, not " + Convert(value));
  }

  {
    MyTime t(2001, 12, 31, 23, 0);
    if ((value = t.MyGetCompareValue()) != -60)
      TEST_FAILED("Value for 31.12.2001 23:00 should be -60, not " + Convert(value));
  }

  {
    MyTime t(2001, 3, 1, 0, 0);
    if ((value = t.MyGetCompareValue()) != -440640)
      TEST_FAILED("Value for 3.1.2001 00:00 should be -440640, not " + Convert(value));
  }

  {
    MyTime t(2001, 2, 28, 0, 0);
    if ((value = t.MyGetCompareValue()) != -442080)
      TEST_FAILED("Value for 28.2.2001 00:00 should be -442080, not " + Convert(value));
  }

  {
    MyTime t(2000, 3, 1, 0, 0);
    if ((value = t.MyGetCompareValue()) != -966240)
      TEST_FAILED("Value for 1.3.2000 00:00 should be -966240, not " + Convert(value));
  }

  {
    MyTime t(2000, 2, 29, 0, 0);
    if ((value = t.MyGetCompareValue()) != -967680)
      TEST_FAILED("Value for 29.2.2000 00:00 should be -967680, not " + Convert(value));
  }

  {
    MyTime t(2000, 2, 28, 0, 0);
    if ((value = t.MyGetCompareValue()) != -969120)
      TEST_FAILED("Value for 28.2.2000 00:00 should be -969120, not " + Convert(value));
  }

  {
    MyTime t(1963, 11, 1, 7, 15);
    if ((value = t.MyGetCompareValue()) != -20074605)
      TEST_FAILED("Value for 1.11.1963 7:15 should be -20074605, not " + Convert(value));
  }

  {
    MyTime t(1970, 5, 9, 2, 0);
    if ((value = t.MyGetCompareValue()) != -16646280)
      TEST_FAILED("Value for 9.5.1970 2:00 should be -16646280, not " + Convert(value));
  }

  {
    MyTime t(2048, 12, 24, 0, 59);
    if ((value = t.MyGetCompareValue()) != 24709019)
      TEST_FAILED("Value for 24.12.2048 0:59 should be 24709019, not " + Convert(value));
  }

  {
    MyTime t(2400, 2, 29, 0, 0);
    if ((value = t.MyGetCompareValue()) != 209412000)
      TEST_FAILED("Value for 29.2.2400 0:0 should be 209412000, not " + Convert(value));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test DecodeCompareValue
 */
// ----------------------------------------------------------------------

void decodecomparevalue()
{
  using NFmiStringTools::Convert;

  MyTime t(2000, 1, 1);

  t.MyDecodeCompareValue(0);
  if (t != MyTime(2002, 1, 1))
    TEST_FAILED("Time for value 0 should be 1.1.2002 00:00");

  t.MyDecodeCompareValue(60);
  if (t != MyTime(2002, 1, 1, 1, 0))
    TEST_FAILED("Time for value 60 should be 1.1.2002 01:00");

  t.MyDecodeCompareValue(135);
  if (t != MyTime(2002, 1, 1, 2, 15))
    TEST_FAILED("Time for value 135 should be 1.1.2002 02:15");

  t.MyDecodeCompareValue(1440);
  if (t != MyTime(2002, 1, 2, 0, 0))
    TEST_FAILED("Time for value 1440 should be 2.1.2002 00:00");

  t.MyDecodeCompareValue(44640);
  if (t != MyTime(2002, 2, 1, 0, 0))
    TEST_FAILED("Time for value 44640 should be 1.2.2002 00:00");

  t.MyDecodeCompareValue(-60);
  if (t != MyTime(2001, 12, 31, 23, 0))
    TEST_FAILED("Time for value -60 should be 31.12.2001 23:00");

  t.MyDecodeCompareValue(-440640);
  if (t != MyTime(2001, 3, 1, 0, 0))
    TEST_FAILED("Time for value -440640 should be 3.1.2001 00:00");

  t.MyDecodeCompareValue(-442080);
  if (t != MyTime(2001, 2, 28, 0, 0))
    TEST_FAILED("Time for value -442080 should be 28.2.2001 00:00");

  t.MyDecodeCompareValue(-966240);
  if (t != MyTime(2000, 3, 1, 0, 0))
    TEST_FAILED("Time for value -966240 should be 1.3.2000 00:00");

  t.MyDecodeCompareValue(-967680);
  if (t != MyTime(2000, 2, 29, 0, 0))
    TEST_FAILED("Time for value -967680 should be 29.2.2000 00:00");

  t.MyDecodeCompareValue(-969120);
  if (t != MyTime(2000, 2, 28, 0, 0))
    TEST_FAILED("Time for value -969120 should be 28.2.2000 00:00");

  t.MyDecodeCompareValue(-20074605);
  if (t != MyTime(1963, 11, 1, 7, 15))
    TEST_FAILED("Time for value -20074605 should be 1.11.1963 7:15");

  t.MyDecodeCompareValue(-16646280);
  if (t != MyTime(1970, 5, 9, 2, 0))
    TEST_FAILED("Time for value -16646280 should be 9.5.1970 2:00");

  t.MyDecodeCompareValue(24709019);
  if (t != MyTime(2048, 12, 24, 0, 59))
    TEST_FAILED("Time for value 24709019 should be 24.12.2048 0:59");

  t.MyDecodeCompareValue(209412000);
  if (t != MyTime(2400, 2, 29, 0, 0))
    TEST_FAILED("Time for value 209412000 should be 29.2.2400 0:0");

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
    TEST(daysinyear)
    TEST(getcomparevalue);
    TEST(decodecomparevalue);
  }
};

}  // namespace NFmiTimeTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiTime tester" << endl << "===============" << endl;
  NFmiTimeTest::tests t;
  return t.run();
}

// ======================================================================
