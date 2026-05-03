// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiModMeanCalculator
 */
// ======================================================================

#include "NFmiModMeanCalculator.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <cmath>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiModMeanCalculatorTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiModMeanCalculator for unweighted means
 */
// ----------------------------------------------------------------------

void means()
{
  using NFmiStringTools::Convert;

  // Basic operation
  {
    NFmiModMeanCalculator calc;
    if (calc() != kFloatMissing)
      TEST_FAILED("Should return kFloatMissing before anything is added, not " + Convert(calc()));
    calc(10);
    if (calc() != 10)
      TEST_FAILED("Should return 10 after 10 has been added, not " + Convert(calc()));
    calc(20);
    if (calc() != 15)
      TEST_FAILED("Should return 15 after 10,20 have been added, not " + Convert(calc()));
    calc(30);
    if (calc() != 20)
      TEST_FAILED("Should return 20 after 10,20,30 have been added, not " + Convert(calc()));
    calc(kFloatMissing);
    if (calc() != kFloatMissing)
      TEST_FAILED("Should return kFloatMissing after 10,20,30,kFloatMissing have been added, not " +
                  Convert(calc()));
  }

  // Now modulo comes into play

  {
    NFmiModMeanCalculator calc;
    calc(350);
    calc(20);
    if (calc() != 5)
      TEST_FAILED("Should return 5 after 350,20 have been added, not " + Convert(calc()));
    calc(50);
    if (calc() != 20)
      TEST_FAILED("Should return 10 after 350,20,50 have been added, not " + Convert(calc()));
    calc(300);
    if (calc() != 0)
      TEST_FAILED("Should return 0 after 350,20,50,300 have been added, not " + Convert(calc()));
    calc(310);
    if (calc() != 350)
      TEST_FAILED("Should return 350 after 350,20,50,300,310 have been added, not " +
                  Convert(calc()));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiModMeanCalculator for weighted means
 */
// ----------------------------------------------------------------------

void weightedmeans()
{
  using NFmiStringTools::Convert;

  // Basic operation
  {
    NFmiModMeanCalculator calc;

    calc(10, 0.5);
    if (calc() != 10)
      TEST_FAILED("Should return 10 after 0.5*10 has been added, not " + Convert(calc()));
    calc(20, 0.5);
    if (calc() != 15)
      TEST_FAILED("Should return 15 after 0.5*10,0.5*20 have been added, not " + Convert(calc()));
    calc(30, 2);
    if (calc() != 25)
      TEST_FAILED("Should return 7 after 0.5*10,0.5*20,2*30 have been added, not " +
                  Convert(calc()));
    calc(kFloatMissing);
    if (calc() != kFloatMissing)
      TEST_FAILED(
          "Should return kFloatMissing after kFloatMissing has been added to weighted mean, not " +
          Convert(calc()));
  }

  // Now modulo comes into play

  {
    NFmiModMeanCalculator calc;
    calc(350, 0.1);
    calc(20, 0.2);
    if (abs(calc() - 10) > 0.001)
      TEST_FAILED("Should return 10 after 0.1*350,0.2*20 have been added, not " + Convert(calc()));
    calc(50, 0.3);
    if (abs(calc() - 30) > 0.001)
      TEST_FAILED("Should return 30 after 0.1*350,0.2*20,0.3*50 have been added, not " +
                  Convert(calc()));
    calc(300, 0.4);
    if (abs(calc() - 354) > 0.001)
      TEST_FAILED("Should return 0 after 0.1*350,0.2*20,0.3*50,0.4*300 have been added, not " +
                  Convert(calc()));
    calc(310, 1);
    if (abs(calc() - 332) > 0.001)
      TEST_FAILED(
          "Should return 332 after 0.1*350,0.2*20,0.3*50,0.4*300,1*310  have been added, not " +
          Convert(calc()));
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
    TEST(means);
    TEST(weightedmeans);
  }
};

}  // namespace NFmiModMeanCalculatorTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiModMeanCalculator tester" << endl << "============================" << endl;
  NFmiModMeanCalculatorTest::tests t;
  return t.run();
}

// ======================================================================
