// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiIndexMaskSource
 */
// ======================================================================

#include "NFmiIndexMaskSource.h"
#include "NFmiIndexMaskTools.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiIndexMaskSourceTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test the Insert() and Find() methods
 */
// ----------------------------------------------------------------------

void insert_and_find(void)
{
  using namespace std;

  NFmiIndexMask mask1, mask2, mask3, mask4;
  mask1.insert(1);
  mask2.insert(2);
  mask3.insert(3);
  mask4.insert(4);

  NFmiMetTime time1(2003, 1, 1);
  NFmiMetTime time2(2003, 1, 3);
  NFmiMetTime time3(2003, 1, 7);
  NFmiMetTime time4(2002, 1, 1);

  NFmiIndexMaskSource source;

  if (source.Find(time1).size() != 0)
    TEST_FAILED("Should find empty mask from empty source");

  source.Insert(time1, mask1);
  if (source.Find(time1) != mask1)
    TEST_FAILED("Should find mask1 for time1 after 1 insert");
  if (source.Find(time2) != mask1)
    TEST_FAILED("Should find mask1 for time2 after 1 insert");
  if (source.Find(time3) != mask1)
    TEST_FAILED("Should find mask1 for time3 after 1 insert");
  if (source.Find(time4) != mask1)
    TEST_FAILED("Should find mask1 for time4 after 1 insert");

  source.Insert(time2, mask2);
  if (source.Find(time1) != mask1)
    TEST_FAILED("Should find mask1 for time1 after 2 inserts");
  if (source.Find(time2) != mask2)
    TEST_FAILED("Should find mask2 for time2 after 2 inserts");
  if (source.Find(NFmiMetTime(2003, 1, 2)) != mask1)
    TEST_FAILED("Should find mask1 for (time1+time2)/2 after 2 inserts");
  if (source.Find(time3) != mask2)
    TEST_FAILED("Should find mask2 for time3 after 2 inserts");
  if (source.Find(time4) != mask1)
    TEST_FAILED("Should find mask1 for time4 after 2 inserts");

  source.Insert(time3, mask3);
  if (source.Find(time1) != mask1)
    TEST_FAILED("Should find mask1 for time1 after 3 inserts");
  if (source.Find(time2) != mask2)
    TEST_FAILED("Should find mask2 for time2 after 3 inserts");
  if (source.Find(time3) != mask3)
    TEST_FAILED("Should find mask3 for time3 after 3 inserts");
  if (source.Find(time4) != mask1)
    TEST_FAILED("Should find mask1 for time4 after 3 inserts");

  source.Insert(time4, mask4);
  if (source.Find(time1) != mask1)
    TEST_FAILED("Should find mask1 for time1 after 4 inserts");
  if (source.Find(time2) != mask2)
    TEST_FAILED("Should find mask2 for time2 after 4 inserts");
  if (source.Find(time3) != mask3)
    TEST_FAILED("Should find mask3 for time3 after 4 inserts");
  if (source.Find(time4) != mask4)
    TEST_FAILED("Should find mask4 for time4 after 4 inserts");

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
  void test(void) { TEST(insert_and_find); }
};

}  // namespace NFmiIndexMaskSourceTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiIndexMaskSource tester" << endl << "==========================" << endl;
  NFmiIndexMaskSourceTest::tests t;
  return t.run();
}

// ======================================================================
