// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiNearTree
 */
// ======================================================================

#include "NFmiNearTree.h"
#include "NFmiPoint.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiNearTreeTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NearestPoint() methods
 */
// ----------------------------------------------------------------------

void nearestpoint(void)
{
  using namespace std;
  // Build a test tree
  NFmiNearTree<NFmiPoint> tree;
  for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++)
      tree.Insert(NFmiPoint(i, j));

  NFmiPoint result;

  // An exact point
  if (!tree.NearestPoint(result, NFmiPoint(0, 0)))
    TEST_FAILED("NearestPoint failed for point (0,0)");
  if (result != NFmiPoint(0, 0))
    TEST_FAILED("NearestPoint returned wrong result for (0,0)");

  // A near point inside grid
  if (!tree.NearestPoint(result, NFmiPoint(4.9, 5.1)))
    TEST_FAILED("NearestPoint failed for point(4.9,5.1");
  if (result != NFmiPoint(5, 5))
    TEST_FAILED("NearestPoint returned wrong result for (4.9,5.1");

  // A near point outside grid
  if (!tree.NearestPoint(result, NFmiPoint(-0.5, 5.1)))
    TEST_FAILED("NearestPoint failed for point(-0.5,5.1");
  if (result != NFmiPoint(0, 5))
    TEST_FAILED("NearestPoint returned wrong result for (-0.5,5.1");

  // A distance limit
  if (tree.NearestPoint(result, NFmiPoint(-1, -1), 1))
    TEST_FAILED("NearestPoint failed distance limit 1");
  if (!tree.NearestPoint(result, NFmiPoint(-1, -1), 2))
    TEST_FAILED("NearestPoint failed distance limit 2");
  if (result != NFmiPoint(0, 0))
    TEST_FAILED("NearestPoint returned wrong result for dist. limit 2");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test FarthestPoint() method
 */
// ----------------------------------------------------------------------

void farthestpoint(void)
{
  using namespace std;
  // Build a test tree
  NFmiNearTree<NFmiPoint> tree;
  for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++)
      tree.Insert(NFmiPoint(i, j));

  NFmiPoint result;

  // An exact point
  if (!tree.FarthestPoint(result, NFmiPoint(0, 0)))
    TEST_FAILED("FarthestPoint failed for point (0,0)");
  if (result != NFmiPoint(9, 9))
    TEST_FAILED("FarthestPoint returned wrong result for (0,0)");

  // A near point inside grid
  if (!tree.FarthestPoint(result, NFmiPoint(4.9, 5.1)))
    TEST_FAILED("FarthestPoint failed for point(4.9,5.1");
  if (result != NFmiPoint(0, 0))
    TEST_FAILED("FarthestPoint returned wrong result for (4.9,5.1");

  // A near point outside grid
  if (!tree.FarthestPoint(result, NFmiPoint(-0.5, 5.1)))
    TEST_FAILED("FarthestPoint failed for point(-0.5,5.1");
  if (result != NFmiPoint(9, 0))
    TEST_FAILED("FarthestPoint returned wrong result for (-0.5,5.1");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NearestPoints() method
 */
// ----------------------------------------------------------------------

void nearestpoints(void)
{
  using namespace std;
  // Build a test tree
  NFmiNearTree<NFmiPoint> tree;
  for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++)
      tree.Insert(NFmiPoint(i, j));

  std::vector<NFmiPoint> results;

  // (0,0) (0,1) (1,0)
  if (tree.NearestPoints(results, NFmiPoint(0, 0), 1.25) != 3)
    TEST_FAILED("NearestPoints failed to find 3 points for (0,0)");

  // (0,0) (0,1) (1,0) (1,1)
  if (tree.NearestPoints(results, NFmiPoint(0, 0), 1.5) != 4)
    TEST_FAILED("NearestPoints failed to find 4 points for (0,0)");

  // .......
  // ..XXX..
  // .XXXXX.
  // .XXXXX.
  // .XXXXX.
  // ..XXX..
  // .......

  if (tree.NearestPoints(results, NFmiPoint(5, 5), 2.5) != 21)
    TEST_FAILED("NearestPoints failed to find 21 points for (5,5)");

  // .......
  // ...X...
  // ..XXX..
  // .XXXXX.
  // ..XXX..
  // ...X...
  // .......

  if (tree.NearestPoints(results, NFmiPoint(5, 5), 2.1) != 13)
    TEST_FAILED("NearestPoints failed to find 13 points for (5,5)");

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
    TEST(nearestpoint);
    TEST(farthestpoint);
    TEST(nearestpoints);
  }
};

}  // namespace NFmiNearTreeTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiNearTree tester" << endl << "===================" << endl;
  NFmiNearTreeTest::tests t;
  return t.run();
}

// ======================================================================
