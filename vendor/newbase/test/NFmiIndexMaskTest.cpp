// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiIndexMask
 */
// ======================================================================

#include "NFmiIndexMask.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiIndexMaskTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test the clear() method
 */
// ----------------------------------------------------------------------

void clear(void)
{
  using namespace std;

  NFmiIndexMask mask;
  mask.insert(1);
  mask.clear();
  if (!mask.empty())
    TEST_FAILED("clear() failed according to empty()");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the empty() method
 */
// ----------------------------------------------------------------------

void empty(void)
{
  using namespace std;

  NFmiIndexMask mask;
  if (!mask.empty())
    TEST_FAILED("Mask must be empty after construction");

  mask.insert(0);
  if (mask.empty())
    TEST_FAILED("Mask must not be empty after insertion");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the insert() method
 */
// ----------------------------------------------------------------------

void insert(void)
{
  using namespace std;

  NFmiIndexMask mask;
  if (mask.size() != 0)
    TEST_FAILED("Mask size must be 0 after construction");

  mask.insert(1);
  if (mask.size() != 1)
    TEST_FAILED("Mask size must be 1 after 1 insert");

  mask.insert(2);
  if (mask.size() != 2)
    TEST_FAILED("Mask size must be 2 after 2 unique inserts");

  mask.insert(1);
  if (mask.size() != 2)
    TEST_FAILED("Mask size must not change after duplicate insert");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the find() method
 */
// ----------------------------------------------------------------------

void find(void)
{
  using namespace std;

  NFmiIndexMask mask;
  if (mask.find(0) != mask.end())
    TEST_FAILED("Find should fail for empty masks");

  mask.insert(1);
  if (mask.find(1) == mask.end())
    TEST_FAILED("Find should find an element inserted earlier");
  if (mask.find(0) != mask.end())
    TEST_FAILED("Find should not find a wrong element");

  mask.insert(2);
  if (mask.find(1) == mask.end())
    TEST_FAILED("Find should work for old elements too");
  if (mask.find(2) == mask.end())
    TEST_FAILED("Find should work for many elements");
  if (mask.find(0) != mask.end())
    TEST_FAILED("Find should not find 0 when 1 and 2 have been inserted");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the equality comparison
 */
// ----------------------------------------------------------------------

void equality(void)
{
  using namespace std;

  NFmiIndexMask mask1;
  NFmiIndexMask mask2;
  NFmiIndexMask mask3;

  mask1.insert(1);
  mask1.insert(2);
  mask1.insert(3);

  mask2.insert(1);
  mask2.insert(2);

  mask3.insert(1);
  mask3.insert(2);
  mask3.insert(3);

  if (mask1 == mask2)
    TEST_FAILED("operator== failed for different masks");

  if (!(mask1 == mask3))
    TEST_FAILED("operator== failed for identical masks");

  if (!(mask1 != mask2))
    TEST_FAILED("operator!= failed for different masks");

  if (mask1 != mask3)
    TEST_FAILED("operator!= failed for identical masks");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the iterators
 */
// ----------------------------------------------------------------------

void iterating(void)
{
  using namespace std;

  NFmiIndexMask::const_iterator it;

  NFmiIndexMask mask;

  if (mask.begin() != mask.end())
    TEST_FAILED("begin() must equal end() for empty mask");

  mask.insert(0);
  it = mask.begin();
  if (++it != mask.end())
    TEST_FAILED("++begin() must equal end() for mask of size 1");

  it = mask.end();
  if (mask.begin() != --it)
    TEST_FAILED("begin() must equal --end() for mask of size 1");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the logical operators
 */
// ----------------------------------------------------------------------

void logical(void)
{
  using namespace std;

  NFmiIndexMask mask1;
  NFmiIndexMask mask2;

  mask1.insert(1);
  mask1.insert(2);
  mask1.insert(3);
  mask1.insert(4);
  mask1.insert(5);

  mask2.insert(1);
  mask2.insert(3);
  mask2.insert(5);
  mask2.insert(7);

  {
    NFmiIndexMask mask = mask1 & mask2;
    if (mask.size() != 3)
      TEST_FAILED("mask operator& failed to return size 3");
    if (mask.find(1) == mask.end())
      TEST_FAILED("mask operator& failed to keep index 1");
    if (mask.find(3) == mask.end())
      TEST_FAILED("mask operator& failed to keep index 3");
    if (mask.find(5) == mask.end())
      TEST_FAILED("mask operator& failed to keep index 5");
  }

  {
    NFmiIndexMask mask = mask1 | mask2;
    if (mask.size() != 6)
      TEST_FAILED("mask operator| failed to return size 6");
    if (mask.find(1) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 1");
    if (mask.find(2) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 2");
    if (mask.find(3) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 3");
    if (mask.find(4) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 4");
    if (mask.find(5) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 5");
    if (mask.find(7) == mask.end())
      TEST_FAILED("mask operator| failed to keep index 7");
  }

  {
    NFmiIndexMask mask = mask1 ^ mask2;
    if (mask.size() != 3)
      TEST_FAILED("mask operator^ failed to return size 3");
    if (mask.find(2) == mask.end())
      TEST_FAILED("mask operator^ failed to keep index 2");
    if (mask.find(4) == mask.end())
      TEST_FAILED("mask operator^ failed to keep index 4");
    if (mask.find(7) == mask.end())
      TEST_FAILED("mask operator^ failed to keep index 7");
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
    TEST(empty);
    TEST(insert);
    TEST(clear);
    TEST(find);
    TEST(equality);
    TEST(iterating);
    TEST(logical);
  }
};

}  // namespace NFmiIndexMaskTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiIndexMask tester" << endl << "====================" << endl;
  NFmiIndexMaskTest::tests t;
  return t.run();
}

// ======================================================================
