// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiIndexMaskTools
 */
// ======================================================================

#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiIndexMask.h"
#include "NFmiIndexMaskTools.h"
#include "NFmiStreamQueryData.h"
#include "NFmiSvgPath.h"
#include <regression/tframe.h>
#include <fstream>

//! Protection against conflicts with global functions
namespace NFmiIndexMaskToolsTest
{
NFmiStreamQueryData theQD;
const NFmiGrid* theGrid;

NFmiSvgPath theSuomi;
NFmiSvgPath theCoast;

void read_querydata(const std::string& theFilename)
{
  theQD.ReadData(theFilename);
  theGrid = theQD.QueryInfoIter()->Grid();
}

void read_svg()
{
  std::ifstream insuomi("data/suomi.svg", std::ios::in);
  insuomi >> theSuomi;
  insuomi.close();

  std::ifstream incoast("data/vesiraja.svg", std::ios::in);
  incoast >> theCoast;
  incoast.close();
}

std::string maskdifference(const std::string& theLhs, const std::string& theRhs)
{
  std::string ret;
  std::string::const_iterator it1 = theLhs.begin();
  std::string::const_iterator it2 = theRhs.begin();
  for (; it1 != theLhs.end() && it2 != theRhs.end(); ++it1, ++it2)
  {
    if (*it1 == *it2)
      ret += *it1;
    else if (*it1 == '.')
      ret += '+';
    else
      ret += '-';
  }
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskInside()
 */
// ----------------------------------------------------------------------

void maskinside(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskInside(*theGrid, theSuomi);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_inside.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskInside failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskOutside()
 */
// ----------------------------------------------------------------------

void maskoutside(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskOutside(*theGrid, theSuomi);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_outside.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskOutside failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskExpand()
 */
// ----------------------------------------------------------------------

void maskexpand(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskExpand(*theGrid, theSuomi, 50);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_expand.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
    cout << endl << "Difference = " << endl << maskdifference(expected, result) << endl;
    TEST_FAILED("MaskExpand failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskShrink()
 */
// ----------------------------------------------------------------------

void maskshrink(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskShrink(*theGrid, theSuomi, 25);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_shrink.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskShrink failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskDistance(path)
 */
// ----------------------------------------------------------------------

void maskdistancepath(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  {
    NFmiIndexMask suomi = MaskDistance(*theGrid, theSuomi, 25);
    string expected;
    NFmiFileSystem::ReadFile2String("data/suomi_distance.mask", expected);
    string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
    if (result != expected)
    {
#if 0
		  cout << endl
			   << "Difference = " << endl
			   << maskdifference(expected,result) << endl;
#endif
      TEST_FAILED("MaskDistance failed for suomi.svg");
    }
  }

  {
    NFmiIndexMask coast = MaskDistance(*theGrid, theCoast, 25);
    string expected;
    NFmiFileSystem::ReadFile2String("data/coast_distance.mask", expected);
    string result = MaskString(coast, theGrid->XNumber(), theGrid->YNumber());
    if (result != expected)
    {
#if 0
		  cout << endl
			   << "Difference = " << endl
			   << maskdifference(expected,result) << endl;
#endif
      TEST_FAILED("MaskDistance failed for coast.svg");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskDistance(point)
 */
// ----------------------------------------------------------------------

void maskdistancepoint(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiPoint hki(25, 60);
  NFmiIndexMask helsinki = MaskDistance(*theGrid, hki, 100);
  string expected;
  NFmiFileSystem::ReadFile2String("data/helsinki_distance.mask", expected);
  string result = MaskString(helsinki, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskDistance failed for helsinki");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskExpand() for a vector of distances
 */
// ----------------------------------------------------------------------

void maskexpandmany(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  std::vector<double> distances;
  distances.push_back(0);
  distances.push_back(-25);
  distances.push_back(50);

  string expected1, expected2, expected3;
  NFmiFileSystem::ReadFile2String("data/suomi_inside.mask", expected1);
  NFmiFileSystem::ReadFile2String("data/suomi_shrink.mask", expected2);
  NFmiFileSystem::ReadFile2String("data/suomi_expand.mask", expected3);

  std::vector<NFmiIndexMask> masks;
  masks = MaskExpand(*theGrid, theSuomi, distances);

  if (masks.size() != 3)
    TEST_FAILED("MaskExpand failed to return vector of 3 masks");

  string result1 = MaskString(masks[0], theGrid->XNumber(), theGrid->YNumber());
  string result2 = MaskString(masks[1], theGrid->XNumber(), theGrid->YNumber());
  string result3 = MaskString(masks[2], theGrid->XNumber(), theGrid->YNumber());

  if (result1 != expected1)
  {
    cout << endl
         << "Difference for distance 0 = " << endl
         << maskdifference(expected1, result1) << endl;
    TEST_FAILED("MaskExpand failed for distance 0 in vector");
  }
  if (result2 != expected2)
  {
    cout << endl
         << "Difference for distance -25 = " << endl
         << maskdifference(expected2, result2) << endl;
    TEST_FAILED("MaskExpand failed for distance -25 in vector");
  }
  if (result3 != expected3)
  {
    cout << endl
         << "Difference for distance 50 = " << endl
         << maskdifference(expected3, result3) << endl;
    TEST_FAILED("MaskExpand failed for distance 50 in vector");
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
    TEST(maskinside);
    TEST(maskoutside);
    TEST(maskexpand);
    TEST(maskshrink);
    TEST(maskdistancepath);
    TEST(maskdistancepoint);
    TEST(maskexpandmany);
  }
};

}  // namespace NFmiIndexMaskToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiIndexMaskTools tester" << endl << "=========================" << endl;

  NFmiIndexMaskToolsTest::read_querydata("data/hiladata.sqd");
  NFmiIndexMaskToolsTest::read_svg();

  NFmiIndexMaskToolsTest::tests t;
  return t.run();
}

// ======================================================================
