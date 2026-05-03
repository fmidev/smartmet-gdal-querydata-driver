// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiString
 */
// ======================================================================

#include "NFmiString.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiStringTest
{
// ----------------------------------------------------------------------

void add()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void getlen()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void getchars()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void trimr()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void triml()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void fillr()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void uppercase()
{
  NFmiString tmp;

  tmp = "test";
  tmp.UpperCase();
  if (string(tmp) != "TEST")
    TEST_FAILED("Failed to convert 'test'");

  tmp = "TEST";
  tmp.UpperCase();
  if (string(tmp) != "TEST")
    TEST_FAILED("Failed to convert 'TEST'");

  tmp =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ\xC5\xC4\xD6"
      "abcdefghijklmnopqrstuvwxyz\xE5\xE4\xF6"
      "01234567890!#%&/"
      "()=?+*^~,;.:-_ ";
  const static char *shouldbe =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ\xC5\xC4\xD6"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ\xC5\xC4\xD6"
      "01234567890!#%&/"
      "()=?+*^~,;.:-_ ";

  tmp.UpperCase();
  if (string(tmp) != string(shouldbe))
    TEST_FAILED("Failed to convert the alphabet:\n" + tmp + "\n" + shouldbe + "\n");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void lowercase()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void firstchartoupper()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void firstinwordtoupper()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void firstcharisupper()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void search()
{
  NFmiString path = "/smartmet/foo/bar/";
  const unsigned char *needle = reinterpret_cast<const unsigned char *>("/");

  unsigned long pos;
  if ((pos = path.Search(needle)) != 1)
    TEST_FAILED("Should find / in " + path + " at position 1, not " +
                boost::lexical_cast<string>(pos));

  if ((pos = path.Search(needle, 1)) != 1)
    TEST_FAILED("Should find / in " + path + " at position 1 with startpos 1, not " +
                boost::lexical_cast<string>(pos));

  if ((pos = path.Search(needle, 2)) != 10)
    TEST_FAILED("Should find / in " + path + " at position 10 with startpos 2, not " +
                boost::lexical_cast<string>(pos));

  if ((pos = path.Search(needle, 3)) != 10)
    TEST_FAILED("Should find / in " + path + " at position 10 with startpos 3, not " +
                boost::lexical_cast<string>(pos));

  path = "|smartmet|foo|bar";
  if ((pos = path.Search(needle)) != 0)
    TEST_FAILED("Should not find / in " + path);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void searchlast()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void charcount()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void replacechars()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void replace()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void removeextraspaces()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char *error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(add);
    TEST(getlen);
    TEST(getchars);
    TEST(trimr);
    TEST(triml);
    TEST(fillr);
    TEST(uppercase);
    TEST(lowercase);
    TEST(firstchartoupper);
    TEST(firstinwordtoupper);
    TEST(firstcharisupper);
    TEST(search);
    TEST(searchlast);
    TEST(charcount);
    TEST(replacechars);
    TEST(replace);
    TEST(removeextraspaces);
  }
};

}  // namespace NFmiStringTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiString tester" << endl << "=================" << endl;
  NFmiStringTest::tests t;
  return t.run();
}

// ======================================================================
