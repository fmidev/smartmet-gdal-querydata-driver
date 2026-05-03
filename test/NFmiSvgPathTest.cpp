// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiSvgPath
 */
// ======================================================================

#include "NFmiPoint.h"
#include "NFmiSvgPath.h"
#include <regression/tframe.h>
#include <cmath>
#include <sstream>

//! Protection against conflicts with global functions
namespace NFmiSvgPathTest
{
// ----------------------------------------------------------------------
/*!
 * Test the size() method
 */
// ----------------------------------------------------------------------

void size(void)
{
  using namespace std;

  NFmiSvgPath path;

  if (path.size() != 0)
    TEST_FAILED("Path size after construction must be 0");

  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  if (path.size() != 1)
    TEST_FAILED("Path size after 1 push_back must be 1");

  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  if (path.size() != 2)
    TEST_FAILED("Path size after 2 push_backs must be 2");

  path.clear();
  if (path.size() != 0)
    TEST_FAILED("Path size after clear() must be 0");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test the empty() method
 */
// ----------------------------------------------------------------------

void empty(void)
{
  using namespace std;

  NFmiSvgPath path;

  if (!path.empty())
    TEST_FAILED("Path must be empty after construction");

  path.push_back(NFmiSvgPath::value_type(NFmiSvgPath::kElementMoveto, 0, 0));
  if (path.empty())
    TEST_FAILED("Path must not be empty after push_back()");

  path.clear();
  if (!path.empty())
    TEST_FAILED("Path must be empty after clear()");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test the front() and back() methods
 */
// ----------------------------------------------------------------------

void front_and_back(void)
{
  using namespace std;

  NFmiSvgPath path;

  NFmiSvgPath::value_type elem1(NFmiSvgPath::kElementMoveto, 0, 0);
  NFmiSvgPath::value_type elem2(NFmiSvgPath::kElementLineto, 0, 0);

  path.push_back(elem1);
  path.push_back(elem2);

  if (path.front().itsType != elem1.itsType || path.front().itsX != elem1.itsX ||
      path.front().itsY != elem1.itsY)
    TEST_FAILED("front() failed");

  if (path.back().itsType != elem2.itsType || path.back().itsX != elem2.itsX ||
      path.back().itsY != elem2.itsY)
    TEST_FAILED("back() failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test the iterators
 */
// ----------------------------------------------------------------------

void iter(void)
{
  using namespace std;

  NFmiSvgPath path;
  NFmiSvgPath::value_type elem1(NFmiSvgPath::kElementMoveto, 0, 0);

  if (path.begin() != path.end())
    TEST_FAILED("begin() and end() must be equal for empty paths");

  path.push_back(elem1);

  NFmiSvgPath::const_iterator it1 = path.begin();
  NFmiSvgPath::const_iterator it2 = path.end();

  if (++it1 != it2)
    TEST_FAILED("++begin() and end() must be equal for one element");

  if (--it1 != --it2)
    TEST_FAILED("begin() and --end() must be equal for one element");

  path.begin()->itsX = 1;
  if (path.front().itsX != 1)
    TEST_FAILED("Failed to modify iterated element");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test the Read() and Write() methods
 */
// ----------------------------------------------------------------------

void read_and_write(void)
{
  using namespace std;

  string path1 = "\"M 0 1 L 1 2\"";
  string path2 = "\"M0 1 1 2\"";
  string path3 = "\"m0 1 1 1\"";
  string path4 = "\"M0,1 1,2\"";
  string path5 = "\"M0,1,1,2\"";

  {
    NFmiSvgPath path;
    istringstream in(path1);
    path.Read(in);

    if (path.size() != 2 || path.front().itsType != NFmiSvgPath::kElementMoveto ||
        path.front().itsX != 0 || path.front().itsY != 1 ||
        path.back().itsType != NFmiSvgPath::kElementLineto || path.back().itsX != 1 ||
        path.back().itsY != 2)
      TEST_FAILED("Failed to read M 0 1 L 1 2");

    ostringstream out;
    path.Write(out);
    if (out.str() != path1)
      TEST_FAILED("Failed to write M 0 1 L 1 2");
  }

  {
    NFmiSvgPath path;
    istringstream in(path2);
    path.Read(in);
    ostringstream out;
    path.Write(out);
    if (out.str() != path1)
      TEST_FAILED("Failed to read M0 1 1 2");
  }

  {
    NFmiSvgPath path;
    istringstream in(path3);
    path.Read(in);
    ostringstream out;
    path.Write(out);
    if (out.str() != path1)
      TEST_FAILED("Failed to read m0 1 1 1");
  }

  {
    NFmiSvgPath path;
    istringstream in(path4);
    path.Read(in);
    ostringstream out;
    path.Write(out);
    if (out.str() != path1)
      TEST_FAILED("Failed to read M0,1 1,2");
  }

  {
    NFmiSvgPath path;
    istringstream in(path5);
    path.Read(in);
    ostringstream out;
    path.Write(out);
    if (out.str() != path1)
      TEST_FAILED("Failed to read M0,1,1,2");
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
    TEST(size);
    TEST(empty);
    TEST(front_and_back);
    TEST(iter);
    TEST(read_and_write);
  }
};

}  // namespace NFmiSvgPathTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiSvgPath tester" << endl << "===================" << endl;
  NFmiSvgPathTest::tests t;
  return t.run();
}

// ======================================================================
