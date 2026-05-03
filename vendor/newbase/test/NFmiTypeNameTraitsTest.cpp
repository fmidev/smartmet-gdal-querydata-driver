// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiTypeNameTraits
 */
// ======================================================================

#include "NFmiTypeNameTraits.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiTypeNameTraitsTest
{
// ----------------------------------------------------------------------
/*!
 * Tests the Name function
 */
// ----------------------------------------------------------------------

void name(void)
{
  using namespace std;

  if (NFmiTypeNameTraits::Name<int>() != "int")
    TEST_FAILED("Failed to name int type");

  if (NFmiTypeNameTraits::Name<unsigned char>() != "unsigned char")
    TEST_FAILED("Failed to name unsigned char type");

  if (NFmiTypeNameTraits::Name<unsigned short>() != "unsigned short")
    TEST_FAILED("Failed to name unsigned short type");

  if (NFmiTypeNameTraits::Name<unsigned int>() != "unsigned int")
    TEST_FAILED("Failed to name unsigned int type");

  if (NFmiTypeNameTraits::Name<unsigned long>() != "unsigned long")
    TEST_FAILED("Failed to name unsigned long type");

  if (NFmiTypeNameTraits::Name<char>() != "char")
    TEST_FAILED("Failed to name char type");

  if (NFmiTypeNameTraits::Name<short>() != "short")
    TEST_FAILED("Failed to name short type");

  if (NFmiTypeNameTraits::Name<int>() != "int")
    TEST_FAILED("Failed to name int type");

  if (NFmiTypeNameTraits::Name<long>() != "long")
    TEST_FAILED("Failed to name long type");

  if (NFmiTypeNameTraits::Name<bool>() != "bool")
    TEST_FAILED("Failed to name bool type");

  if (NFmiTypeNameTraits::Name<float>() != "float")
    TEST_FAILED("Failed to name float type");

  if (NFmiTypeNameTraits::Name<double>() != "double")
    TEST_FAILED("Failed to name double type");

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
  void test(void) { TEST(name); }
};

}  // namespace NFmiTypeNameTraitsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiTypeNameTraits tester" << endl << "=========================" << endl;
  NFmiTypeNameTraitsTest::tests t;
  return t.run();
}

// ======================================================================
