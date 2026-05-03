// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiLocationFinder
 */
// ======================================================================

#include "NFmiLocationFinder.h"
#include <regression/tframe.h>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;

std::string tostr(const NFmiPoint& p)
{
  std::stringstream out;
  out << p.X() << "," << p.Y();
  return out.str();
}

//! Protection against conflicts with global functions
namespace NFmiLocationFinderTest
{
// ----------------------------------------------------------------------

void optional_windows()
{
  NFmiLocationFinder finder;

  if (!finder.AddFile("data/name2longlat.win", true, true))
    TEST_FAILED("Failed to read test data");

  NFmiPoint p;

  p = finder.Find("Helsinki");
  if (p != NFmiPoint(24.95, 60.18))
    TEST_FAILED("Found wrong coordinates for Helsinki: " + tostr(p));

  p = finder.Find("Salla");
  if (p != NFmiPoint(28.6, 66.8))
    TEST_FAILED("Found wrong coordinates for Salla: " + tostr(p));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void optional_unix()
{
  NFmiLocationFinder finder;

  if (!finder.AddFile("data/name2longlat.unix", true, true))
    TEST_FAILED("Failed to read test data");

  NFmiPoint p;

  p = finder.Find("Helsinki");
  if (p != NFmiPoint(24.95, 60.18))
    TEST_FAILED("Found wrong coordinates for Helsinki: " + tostr(p));

  p = finder.Find("Salla");
  if (p != NFmiPoint(28.6, 66.8))
    TEST_FAILED("Found wrong coordinates for Salla: " + tostr(p));

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
    TEST(optional_windows);
    TEST(optional_unix);
  }
};

}  // namespace NFmiLocationFinderTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiLocationFinder tester" << endl << "=========================" << endl;
  NFmiLocationFinderTest::tests t;
  return t.run();
}

// ======================================================================
