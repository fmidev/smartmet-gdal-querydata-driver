// ======================================================================
/*!
 * \file
 * \brief Regression tests for NFmiEnumConverter
 */
// ======================================================================

#include "NFmiEnumConverter.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiEnumConverterTest
{
NFmiEnumConverter converter;

// ----------------------------------------------------------------------

void tostring()
{
  string result;

  result = converter.ToString(kFmiTemperature);
  if (result != "Temperature")
    TEST_FAILED("kFmiTemperature name not found");

  result = converter.ToString(kFmiPrecipitation1h);
  if (result != "Precipitation1h")
    TEST_FAILED("kFmiPrecipitation1h name not found");

  result = converter.ToString(66666);
  if (result != "")
    TEST_FAILED("66666 should not be defined");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void toenum()
{
  int result;

  result = converter.ToEnum("Temperature");
  if (result != kFmiTemperature)
    TEST_FAILED("Temperature id should be " + boost::lexical_cast<string>(kFmiTemperature) +
                " not " + boost::lexical_cast<string>(result));

  result = converter.ToEnum("Precipitation1h");
  if (result != kFmiPrecipitation1h)
    TEST_FAILED("Precipitation1h id should be " + boost::lexical_cast<string>(kFmiPrecipitation1h) +
                " not " + boost::lexical_cast<string>(result));

  result = converter.ToEnum("temperature");
  if (result != kFmiTemperature)
    TEST_FAILED("temperature id should be " + boost::lexical_cast<string>(kFmiTemperature) +
                " not " + boost::lexical_cast<string>(result));

  result = converter.ToEnum("windspeedMS");
  if (result != kFmiWindSpeedMS)
    TEST_FAILED("windspeedMS id should be " + boost::lexical_cast<string>(kFmiWindSpeedMS) +
                " not " + boost::lexical_cast<string>(result));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void size()
{
  size_t sz = converter.Names().size();
  if (sz < 860)
    TEST_FAILED("newbase should define at least 860 names, not " + boost::lexical_cast<string>(sz));

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
    TEST(tostring);
    TEST(toenum);
    TEST(size);
  }
};

}  // namespace NFmiEnumConverterTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiEnumConverter tester" << endl << "========================" << endl;
  NFmiEnumConverterTest::tests t;
  return t.run();
}

// ======================================================================
