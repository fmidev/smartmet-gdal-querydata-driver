// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiSettings
 */
// ======================================================================

#include "NFmiSettings.h"
#include <regression/tframe.h>

#include <string>

//! Protection against conflicts with global functions
namespace NFmiSettingsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiSettings::Require
 */
// ----------------------------------------------------------------------

void require()
{
  using namespace std;

  NFmiSettings::Clear();
  NFmiSettings::Set("key a", "value a");

  // strings
  try
  {
    string tmp = NFmiSettings::Require<string>("key a");
    if (tmp != "value a")
      TEST_FAILED("Failed to retrieve 'value a' for 'key a'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to retrieve 'key a'");
  }

  try
  {
    string tmp = NFmiSettings::Require<string>("key b");
    TEST_FAILED("Should not have retrieved value for 'key b'");
  }
  catch (...)
  {
  }

  // conversion to int

  NFmiSettings::Set("key b", "123");
  try
  {
    int tmp = NFmiSettings::Require<int>("key b");
    if (tmp != 123)
      TEST_FAILED("Failed to retrieve value 123 for 'key b'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to retrieve and parse 'key b'");
  }

  NFmiSettings::Set("key c", "123.456");
  try
  {
    NFmiSettings::Require<int>("key c");
    TEST_FAILED("Should not have gotten integer value for 'key c'");
  }
  catch (...)
  {
  }

  // conversion to double

  try
  {
    double tmp = NFmiSettings::Require<double>("key b");
    if (tmp != 123.0)
      TEST_FAILED("Failed to retrieve value 123.0 for 'key b'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to retrieve and parse 'key b' as double");
  }

  try
  {
    double tmp = NFmiSettings::Require<double>("key c");
    if (tmp != 123.456)
      TEST_FAILED("Failed to retrieve value 123.456 for 'key c'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to retrieve and parse 'key c' as double");
  }

  // conversion to boolean

  NFmiSettings::Set("key d", "false");
  try
  {
    bool tmp = NFmiSettings::Require<bool>("key d");
    if (tmp != false)
      TEST_FAILED("Failed to retrieve value false for 'key d'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to retrieve and parse 'key d' as boolean");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiSettings::RequireRange
 */
// ----------------------------------------------------------------------

void require_range()
{
  NFmiSettings::Clear();

  NFmiSettings::Set("key a", "10");

  try
  {
    int tmp = NFmiSettings::RequireRange<int>("key a", 0, 100);
    if (tmp != 10)
      TEST_FAILED("Failed to obtain value 10 for 'key a'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to parse 'key a' value as 10");
  }

  try
  {
    NFmiSettings::RequireRange<int>("key a", 50, 100);
    TEST_FAILED("Failed to reject value 10 for 'key a' for not belonging to 50-100");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiSettings::Optional
 */
// ----------------------------------------------------------------------

void optional()
{
  using namespace std;

  NFmiSettings::Clear();

  // strings

  {
    NFmiSettings::Set("key a", "value a");

    string tmp = NFmiSettings::Optional<string>("key a", "optional a");
    if (tmp != "value a")
      TEST_FAILED("Failed to retrieve 'value a' for 'key a'");

    tmp = NFmiSettings::Optional<string>("key b", "optional b");
    if (tmp != "optional b")
      TEST_FAILED("Failed to retrieve 'optional b' for 'key a'");
  }

  // ints

  {
    NFmiSettings::Set("key b", "123");

    int tmp = NFmiSettings::Optional<int>("key b", 321);
    if (tmp != 123)
      TEST_FAILED("Failed to retrieve value 123 for 'key b'");

    tmp = NFmiSettings::Optional<int>("key c", 321);
    if (tmp != 321)
      TEST_FAILED("Failed to retrieve value 321 for 'key c'");
  }

  // doubles

  {
    NFmiSettings::Set("key a", "123");
    double tmp = NFmiSettings::Optional<double>("key a", 0);
    if (tmp != 123)
      TEST_FAILED("Failed to retrieve value 123 for 'key a'");

    NFmiSettings::Set("key b", "123.456");
    tmp = NFmiSettings::Optional<double>("key b", 0);
    if (tmp != 123.456)
      TEST_FAILED("Failed to retrieve value 123.456 for 'key b'");

    NFmiSettings::Set("key c", "0.0123");
    tmp = NFmiSettings::Optional<double>("key c", 0);
    if (tmp != 0.0123)
      TEST_FAILED("Failed to retrieve value 0.0123 for 'key c'");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiSettings::OptionalRange
 */
// ----------------------------------------------------------------------

void optional_range()
{
  NFmiSettings::Clear();

  NFmiSettings::Set("key a", "10");

  try
  {
    int tmp = NFmiSettings::OptionalRange<int>("key a", 50, 0, 100);
    if (tmp != 10)
      TEST_FAILED("Failed to obtain value 10 for 'key a'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to parse 'key a' value as 10");
  }

  try
  {
    NFmiSettings::OptionalRange<int>("key a", 50, 50, 100);
    TEST_FAILED("Failed to reject value 10 for 'key a' for not belonging to 50-100");
  }
  catch (...)
  {
  }

  try
  {
    int tmp = NFmiSettings::OptionalRange<int>("key b", 50, 0, 100);
    if (tmp != 50)
      TEST_FAILED("Failed to obtain value 50 for 'key b'");
  }
  catch (...)
  {
    TEST_FAILED("Failed to parse 'key b' value as 50");
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
    TEST(require);
    TEST(require_range);
    TEST(optional);
    TEST(optional_range);
  }
};

}  // namespace NFmiSettingsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiSettings tester" << endl << "===================" << endl;
  NFmiSettingsTest::tests t;
  return t.run();
}

// ======================================================================
