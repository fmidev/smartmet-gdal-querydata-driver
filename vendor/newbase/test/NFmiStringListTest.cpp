// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiStringList
 */
// ======================================================================

#include "NFmiStringList.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiStringListTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringList::NumberOfItems()
 */
// ----------------------------------------------------------------------

void numberofitems()
{
  NFmiStringList lst;

  if (lst.NumberOfItems() != 0)
    TEST_FAILED("List size must be 0 after construction");

  lst.Add(new NFmiString("a"));
  if (lst.NumberOfItems() != 1)
    TEST_FAILED("List size must be 1 after 1 insert");

  lst.Add(new NFmiString("b"));
  if (lst.NumberOfItems() != 2)
    TEST_FAILED("List size must be 2 after 2 inserts");

  lst.Clear(true);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringList::Index
 */
// ----------------------------------------------------------------------

void index()
{
  NFmiStringList lst;

  if (lst.Index() != 0)
    TEST_FAILED("Index() should return 0 after construction");

  if (lst.Index(0))
    TEST_FAILED("Index(0) should fail after construction");
  if (lst.Index(-1))
    TEST_FAILED("Index(-1) should fail after construction");
  if (lst.Index(1))
    TEST_FAILED("Index(1) should fail after construction");

  lst.Add(new NFmiString("a"));
  if (!lst.Index(0))
    TEST_FAILED("Index(0) should succeed after 1 insert");
  if (lst.Index(-1))
    TEST_FAILED("Index(-1) should fail after 1 insert");
  if (lst.Index(1))
    TEST_FAILED("Index(1) should fail after 1 insert");

  lst.Add(new NFmiString("b"));
  if (!lst.Index(0))
    TEST_FAILED("Index(0) should succeed after 2 inserts");
  if (lst.Index(-1))
    TEST_FAILED("Index(-1) should fail after 2 inserts");
  if (!lst.Index(1))
    TEST_FAILED("Index(1) should succeed after 2 inserts");

  lst.Clear(true);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringList::Current
 */
// ----------------------------------------------------------------------

void current()
{
  NFmiString* a = new NFmiString("a");
  NFmiString* b = new NFmiString("b");
  NFmiString* c = new NFmiString("c");

  NFmiStringList lst;
  lst.Add(a);
  lst.Add(b);
  lst.Add(c);

  if (lst.Current() != a)
    TEST_FAILED("Current() should return first element after construction");

  if (!lst.Next())
    TEST_FAILED("Next() failed after first element");
  if (lst.Current() != b)
    TEST_FAILED("Current() failed after 1 Next()");

  if (!lst.Next())
    TEST_FAILED("Next() failed after second element");
  if (lst.Current() != c)
    TEST_FAILED("Current() failed after 2 Next() calls");

  if (lst.Next())
    TEST_FAILED("Next() failed after third element");

  lst.Clear(true);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringList::Find
 */
// ----------------------------------------------------------------------

void find()
{
  NFmiString* a = new NFmiString("a");
  NFmiString* b = new NFmiString("b");
  NFmiString* c = new NFmiString("c");

  NFmiStringList lst;
  lst.Add(a);
  lst.Add(b);
  lst.Add(c);

  if (!lst.Find(*a))
    TEST_FAILED("Failed to find string a from list");
  if (lst.Current() != a)
    TEST_FAILED("Failed to set correct index for string a");

  if (!lst.Find(*b))
    TEST_FAILED("Failed to find string b from list");
  if (lst.Current() != b)
    TEST_FAILED("Failed to set correct index for string b");

  if (!lst.Find(*c))
    TEST_FAILED("Failed to find string c from list");
  if (lst.Current() != c)
    TEST_FAILED("Failed to set correct index for string c");

  if (lst.Find(NFmiString("x")))
    TEST_FAILED("Should not have found string x from the list");

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
    TEST(numberofitems);
    TEST(index);
    TEST(current);
    TEST(find);
  }
};

}  // namespace NFmiStringListTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiStringList tester" << endl << "=====================" << endl;
  NFmiStringListTest::tests t;
  return t.run();
}

// ======================================================================
