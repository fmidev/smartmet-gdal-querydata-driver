// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiMetMath
 */
// ======================================================================

#include "NFmiMetMath.h"
#include <regression/tframe.h>

using namespace std;

std::string tostr(float num)
{
  std::ostringstream out;
  out << std::fixed << std::setprecision(3) << num;
  return out.str();
}

//! Protection against conflicts with global functions
namespace NFmiMetMathTest
{
// ----------------------------------------------------------------------

void windchill()
{
  std::string ret;

  ret = tostr(FmiWindChill(0, 0));
  if (ret != "0.000")
    TEST_FAILED("W=0 T=0 should return 0, not " + ret);

  ret = tostr(FmiWindChill(0, 10));
  if (ret != "10.000")
    TEST_FAILED("W=0 T=10 should return 10, not " + ret);

  ret = tostr(FmiWindChill(5, 0));
  if (ret != "-4.935")
    TEST_FAILED("W=5 T=0 should return -4.935, not " + ret);

  ret = tostr(FmiWindChill(5, -5));
  if (ret != "-11.191")
    TEST_FAILED("W=5 T=-5 should return -11.1909332, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void feelslike()
{
  std::string ret;

  // without radiation

  ret = tostr(FmiFeelsLikeTemperature(0, 50, 0, kFloatMissing));
  if (ret != "0.000")
    TEST_FAILED("W=0 RH=50 T=0 should return 0, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(0, 50, 10, kFloatMissing));
  if (ret != "10.000")
    TEST_FAILED("W=0 RH=50 T=10 should return 10, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 50, 0, kFloatMissing));
  if (ret != "-4.980")
    TEST_FAILED("W=5 RH=50 T=0 should return -4.980, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 50, -5, kFloatMissing));
  if (ret != "-10.653")
    TEST_FAILED("W=5 RH=50 T=-5 should return -10.653, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 50, 25, kFloatMissing));
  if (ret != "23.385")
    TEST_FAILED("W=5 RH=50 T=25 should return 23.385, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 90, 25, kFloatMissing));
  if (ret != "26.588")
    TEST_FAILED("W=5 RH=90 T=25 should return 26.588, not " + ret);

  // With radiation

  ret = tostr(FmiFeelsLikeTemperature(0, 50, 0, 0));
  if (ret != "-0.250")
    TEST_FAILED("W=0 RH=50 T=0 should return -0.250, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(0, 50, 10, 50));
  if (ret != "9.995")
    TEST_FAILED("W=0 RH=50 T=10 should return 9.995, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 50, 0, 800));
  if (ret != "-2.617")
    TEST_FAILED("W=5 RH=50 T=0 should return -2.617, not " + ret);

  ret = tostr(FmiFeelsLikeTemperature(5, 50, 25, 425));
  if (ret != "24.523")
    TEST_FAILED("W=5 RH=50 T=25 should return 24.523, not " + ret);

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
    TEST(windchill);
    TEST(feelslike);
  }
};

}  // namespace NFmiMetMathTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiMetMath tester" << endl << "==================" << endl;
  NFmiMetMathTest::tests t;
  return t.run();
}

// ======================================================================
