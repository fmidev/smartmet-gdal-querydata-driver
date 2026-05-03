// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiStringTools
 */
// ======================================================================

#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiStringToolsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test FirstCharToUpper()
 */
// ----------------------------------------------------------------------

void firstchartoupper()
{
  using namespace NFmiStringTools;

  string tmp;

  if (FirstCharToUpper(tmp = "test") != "Test")
    TEST_FAILED("Failed to capitalize 'test'");

  if (FirstCharToUpper(tmp = "") != "")
    TEST_FAILED("Should return empty string for empty string");

  if (FirstCharToUpper(tmp = "ääliö") != "Ääliö")
    TEST_FAILED("Failed to capitalize 'ääliö'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test LowerCase()
 */
// ----------------------------------------------------------------------

void lowercase()
{
  using namespace NFmiStringTools;

  string tmp;

  if (LowerCase(tmp = "test") != "test")
    TEST_FAILED("Failed to convert 'test'");
  if (LowerCase(tmp = "TEST") != "test")
    TEST_FAILED("Failed to convert 'TEST'");
  if (LowerCase(tmp = "ÄÄLIÖ") != "ääliö")
    TEST_FAILED("Failed to convert 'ÄÄLIÖ'");

  if (LowerCase(tmp = "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖabcdefghijklmnopqrstuvwxyz01234567890!#%&/"
                      "()=?+*^~,;.:-_ ") !=
      "abcdefghijklmnopqrstuvwxyzäåöabcdefghijklmnopqrstuvwxyz01234567890!#%&/()=?+*^~,;.:-_ ")
    TEST_FAILED("Failed to convert the alphabet:\n" + tmp +
                "\nabcdefghijklmnopqrstuvwxyzäåöabcdefghijklmnopqrstuvwxyz01234567890!#%&/"
                "()=?+*^~,;.:-_ \n");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UpperCase()
 */
// ----------------------------------------------------------------------

void uppercase()
{
  using namespace NFmiStringTools;

  string tmp;

  if (UpperCase(tmp = "test") != "TEST")
    TEST_FAILED("Failed to convert 'test'");
  if (UpperCase(tmp = "TEST") != "TEST")
    TEST_FAILED("Failed to convert 'TEST'");
  if (UpperCase(tmp = "ääliö") != "ÄÄLIÖ")
    TEST_FAILED("Failed to convert 'ääliö'");

  if (UpperCase(tmp = "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖabcdefghijklmnopqrstuvwxyzäåö01234567890!#%&/"
                      "()=?+*^~,;.:-_ ") !=
      "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖ01234567890!#%&/()=?+*^~,;.:-_ ")
    TEST_FAILED("Failed to convert the alphabet:\n" + tmp +
                "\nABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖ01234567890!#%&/"
                "()=?+*^~,;.:-_ \n");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test ReplaceChars()
 */
// ----------------------------------------------------------------------

void replacechars()
{
  using namespace NFmiStringTools;

  string tmp;

  if (ReplaceChars(tmp = "AABBCCDD", 'A', 'X') != "XXBBCCDD")
    TEST_FAILED("Failed to convert A->X in 'AABBCCDD'");

  if (ReplaceChars(tmp = "AABBCCDD", 'B', 'X') != "AAXXCCDD")
    TEST_FAILED("Failed to convert B->X in 'AABBCCDD'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TrimL()
 */
// ----------------------------------------------------------------------

void triml()
{
  using namespace NFmiStringTools;

  string tmp;
  if (TrimL(tmp = "test") != "test")
    TEST_FAILED("Failed to trim 'test'");
  if (TrimL(tmp = " test") != "test")
    TEST_FAILED("Failed to trim ' test'");
  if (TrimL(tmp = "  test") != "test")
    TEST_FAILED("Failed to trim '  test'");
  if (TrimL(tmp = "test", 't') != "est")
    TEST_FAILED("Failed to trim t out of 'test'");
  if (TrimL(tmp = "  ") != "")
    TEST_FAILED("Failed to trim '  ' to ''");
  if (TrimL(tmp = "\n\t ") != "")
    TEST_FAILED("Failed to trim '\\n\\t ' to ''");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test TrimR()
 */
// ----------------------------------------------------------------------

void trimr()
{
  using namespace NFmiStringTools;

  string tmp;
  if (TrimR(tmp = "test") != "test")
    TEST_FAILED("Failed to trim 'test'");
  if (TrimR(tmp = "test ") != "test")
    TEST_FAILED("Failed to trim 'test '");
  if (TrimR(tmp = "test  ") != "test")
    TEST_FAILED("Failed to trim 'test  '");
  if (TrimR(tmp = "test", 't') != "tes")
    TEST_FAILED("Failed to trim t out of 'test'");
  if (TrimR(tmp = "  ") != "")
    TEST_FAILED("Failed to trim '  ' to ''");
  if (TrimR(tmp = "\n\t ") != "")
    TEST_FAILED("Failed to trim '\\n\\t ' to ''")

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Trim()
 */
// ----------------------------------------------------------------------

void trim()
{
  using namespace NFmiStringTools;

  string tmp;

  if (Trim(tmp = "test") != "test")
    TEST_FAILED("Failed to trim 'test'");
  if (Trim(tmp = " test") != "test")
    TEST_FAILED("Failed to trim ' test'");
  if (Trim(tmp = "  test") != "test")
    TEST_FAILED("Failed to trim '  test'");
  if (Trim(tmp = "test", 't') != "es")
    TEST_FAILED("Failed to trim t out of 'test'");
  if (Trim(tmp = "  ") != "")
    TEST_FAILED("Failed to trim '  ' to ''");
  if (Trim(tmp = "\n\t ") != "")
    TEST_FAILED("Failed to trim '\\n\\t ' to ''");

  if (Trim(tmp = "test") != "test")
    TEST_FAILED("Failed to trim 'test'");
  if (Trim(tmp = "test ") != "test")
    TEST_FAILED("Failed to trim 'test '");
  if (Trim(tmp = "test  ") != "test")
    TEST_FAILED("Failed to trim 'test  '");
  if (Trim(tmp = "  ") != "")
    TEST_FAILED("Failed to trim '  ' to ''");
  if (Trim(tmp = "\n\t ") != "")
    TEST_FAILED("Failed to trim '\\n\\t ' to ''");

  if (Trim(tmp = " äaö ") != "äaö")
    TEST_FAILED("Failed to trim ' äaö ' to 'äaö");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Split<>()
 */
// ----------------------------------------------------------------------

void split()
{
  using namespace NFmiStringTools;

  {
    vector<string> result = Split("1,2,3");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2,3 into vector of strings");
  }

  {
    list<int> result = Split<list<int> >("1,2,3");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2,3 into list of integers");
  }

  {
    list<string> result = Split<list<string> >("1,2,3");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2,3 into list of strings");
  }

  {
    vector<double> result = Split<vector<double> >("1,2,3");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2,3 into vector of doubles");
  }

  {
    set<int> result = Split<set<int> >("1,2,3,2,1");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2,3,2,1 into set of integers");
  }

  {
    vector<string> result = Split(",2,3");
    if (result.size() != 3)
      TEST_FAILED("Failed to split ,2,3 into 3 parts");
  }

  {
    vector<string> result = Split("1,2,");
    if (result.size() != 3)
      TEST_FAILED("Failed to split 1,2, into 3 parts");
  }

  {
    vector<string> result = Split(",,");
    if (result.size() != 3)
      TEST_FAILED("Failed to split ,, into 3 parts");
  }

  {
    vector<string> result = Split(",");
    if (result.size() != 2)
      TEST_FAILED("Failed to split , into 2 parts");
  }

  {
    vector<string> result = Split("");
    if (result.size() != 0)
      TEST_FAILED("Failed to split '' into 0 parts");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Join
 */
// ----------------------------------------------------------------------

void join()
{
  using namespace NFmiStringTools;

  {
    vector<string> input = Split("");

    if (Join(input) != "")
      TEST_FAILED("Failed to join ''");

    if (Join(input, ",") != "")
      TEST_FAILED("Failed to join ''");
  }

  {
    vector<string> input = Split("1");

    if (Join(input) != "1")
      TEST_FAILED("Failed to join 1");

    if (Join(input, ",") != "1")
      TEST_FAILED("Failed to join 1");
  }

  {
    vector<string> input = Split("1,2,3");

    if (Join(input) != "123")
      TEST_FAILED("Failed to join 123");

    if (Join(input, ",") != "1,2,3")
      TEST_FAILED("Failed to join 1,2,3");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringTools::Convert
 */
// ----------------------------------------------------------------------

void convert()
{
  if (NFmiStringTools::Convert<string>("abba") != "abba")
    TEST_FAILED("Failed to parse string 'abba'");

  if (NFmiStringTools::Convert<string>("abba pappa") != "abba pappa")
    TEST_FAILED("Failed to parse string 'abba pappa'");

  if (NFmiStringTools::Convert(1) != "1")
    TEST_FAILED("Failed to decompile 1 into '1'");

  if (NFmiStringTools::Convert(1.234) != "1.234")
    TEST_FAILED("Failed to decompile 1.234 into '1.234'");

  if (NFmiStringTools::Convert<int>("1") != 1)
    TEST_FAILED("Failed to convert '1' into 1");

  if (NFmiStringTools::Convert<double>("1.234") != 1.234)
    TEST_FAILED("Failed to convert '1.234' into 1.234");

  if (NFmiStringTools::Convert<int>(std::string("1")) != 1)
    TEST_FAILED("Failed to convert '1' into 1");

  if (NFmiStringTools::Convert<double>(std::string("1.234")) != 1.234)
    TEST_FAILED("Failed to convert '1.234' into 1.234");

  if (NFmiStringTools::Convert<bool>(std::string("true")) != true)
    TEST_FAILED("Failed to convert 'true' to true");

  if (NFmiStringTools::Convert<bool>(std::string("false")) != false)
    TEST_FAILED("Failed to convert 'false' to false");

  if (NFmiStringTools::Convert<bool>(std::string("1")) != true)
    TEST_FAILED("Failed to convert '1' to true");

  if (NFmiStringTools::Convert<bool>(std::string("0")) != false)
    TEST_FAILED("Failed to convert '0' to false");

  try
  {
    NFmiStringTools::Convert<int>("1.234");
    TEST_FAILED("Should have failed to convert '1.234' to integer");
  }
  catch (Fmi::Exception& e)
  {
  }

  try
  {
    NFmiStringTools::Convert<int>("1X");
    TEST_FAILED("Should have failed to convert '1X' to integer");
  }
  catch (Fmi::Exception& e)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief
 */
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringTools::UrlEncode
 */
// ----------------------------------------------------------------------

void urlencode()
{
  using NFmiStringTools::UrlEncode;

  string str("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  if (UrlEncode(str) != str)
    TEST_FAILED("Failed to encode upper case alphabet correctly: " + UrlEncode(str));

  str = "abcdefghijklmnopqrstuvwxyz";
  if (UrlEncode(str) != str)
    TEST_FAILED("Failed to encode lower case alphabet correctly: " + UrlEncode(str));

  str = "0123456789";
  if (UrlEncode(str) != str)
    TEST_FAILED("Failed to encode lower case alphabet correctly: " + UrlEncode(str));

  if (UrlEncode("_") != "_")
    TEST_FAILED("Failed to encode '_' correctly: " + UrlEncode("_"));

  str = "\n\t !\"#%&/()=+?";
  if (UrlEncode(str) != "%0A%09%20%21%22%23%25%26%2F%28%29%3D%2B%3F")
    TEST_FAILED("Failed to encode special characters correctly: " + UrlEncode(str));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringTools::UrlDecode
 */
// ----------------------------------------------------------------------

void urldecode()
{
  using NFmiStringTools::UrlDecode;

  string str("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  if (UrlDecode(str) != str)
    TEST_FAILED("Failed to decode upper case alphabet correctly: " + UrlDecode(str));

  str = "abcdefghijklmnopqrstuvwxyz";
  if (UrlDecode(str) != str)
    TEST_FAILED("Failed to decode lower case alphabet correctly: " + UrlDecode(str));

  str = "0123456789";
  if (UrlDecode(str) != str)
    TEST_FAILED("Failed to decode lower case alphabet correctly: " + UrlDecode(str));

  if (UrlDecode("_") != "_")
    TEST_FAILED("Failed to decode '_' correctly: " + UrlDecode("_"));

  str = "%0A%09%20%21%22%23%25%26%2F%28%29%3D%2B%3F";
  if (UrlDecode(str) != "\n\t !\"#%&/()=+?")
    TEST_FAILED("Failed to encode special characters correctly: " + UrlDecode(str));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiStringTools::ParseQueryString
 */
// ----------------------------------------------------------------------

void parsequerystring()
{
  using NFmiStringTools::ParseQueryString;

  map<string, string> result;
  map<string, string>::const_iterator it;

  {
    result = ParseQueryString("");
    if (!result.empty())
      TEST_FAILED("Parsing an empty string should return empty map");
  }

  {
    result = ParseQueryString("a=1");
    if (result.size() != 1)
      TEST_FAILED("Failed to parse 'a=1'");
    it = result.find("a");
    if (it == result.end() || it->second != "1")
      TEST_FAILED("Failed to extract 'a' value '1'");
  }

  {
    result = ParseQueryString("QUERYSTRING=b=2");
    if (result.size() != 1)
      TEST_FAILED("Failed to parse 'QUERYSTRING=b=2'");
    it = result.find("b");
    if (it == result.end() || it->second != "2")
      TEST_FAILED("Failed to extract 'b' value '2'");
  }

  {
    result = ParseQueryString("a=1&b=2&c=3");
    if (result.size() != 3)
      TEST_FAILED("Failed to parse 'a=1&b=2&c=3'");
    it = result.find("a");
    if (it == result.end() || it->second != "1")
      TEST_FAILED("Failed to extract 'a' value '1'");
    it = result.find("b");
    if (it == result.end() || it->second != "2")
      TEST_FAILED("Failed to extract 'b' value '2'");
    it = result.find("c");
    if (it == result.end() || it->second != "3")
      TEST_FAILED("Failed to extract 'c' value '3'");
  }

  {
    result = ParseQueryString("space=%20&newline=%0A&tabulator=%09");
    if (result.size() != 3)
      TEST_FAILED("Failed to parse 'space=%20&newline=%0A&tabulator=%09'");
    it = result.find("space");
    if (it == result.end() || it->second != " ")
      TEST_FAILED("Failed to extract 'space' value ' '");
    it = result.find("newline");
    if (it == result.end() || it->second != "\n")
      TEST_FAILED("Failed to extract 'newline' value '\n'");
    it = result.find("tabulator");
    if (it == result.end() || it->second != "\t")
      TEST_FAILED("Failed to extract 'tabulator' value '\t'");
  }

  {
    result = ParseQueryString("%20=space&%0A=newline&%09=tabulator");
    if (result.size() != 3)
      TEST_FAILED("Failed to parse '%20=space&%0A=newline&%09=tabulator'");
    it = result.find(" ");
    if (it == result.end() || it->second != "space")
      TEST_FAILED("Failed to extract ' ' value 'space'");
    it = result.find("\n");
    if (it == result.end() || it->second != "newline")
      TEST_FAILED("Failed to extract '\n' value 'newline'");
    it = result.find("\t");
    if (it == result.end() || it->second != "tabulator")
      TEST_FAILED("Failed to extract '\t' value 'tabulator'");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Basename()
 */
// ----------------------------------------------------------------------

void basename()
{
  using namespace NFmiStringTools;

  string tmp;

  if ((tmp = Basename("foobar")) != "foobar")
    TEST_FAILED("Failed to extract basename of foobar");
  if ((tmp = Basename("foo/bar")) != "bar")
    TEST_FAILED("Failed to extract basename of foo/bar");
  if ((tmp = Basename("foo/")) != "foo")
    TEST_FAILED("Failed to extract basename of foo/");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Suffix()
 */
// ----------------------------------------------------------------------

void suffix()
{
  using namespace NFmiStringTools;

  string tmp;

  if ((tmp = Suffix("foo.bar")) != "bar")
    TEST_FAILED("Failed to extract suffix of foo.bar");
  if ((tmp = Suffix("foobar")) != "")
    TEST_FAILED("Failed to extract suffix of foobar");
  if ((tmp = Suffix("foobar.")) != "")
    TEST_FAILED("Failed to extract suffix of foobar.");

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
    TEST(firstchartoupper);
    TEST(lowercase);
    TEST(uppercase);
    TEST(replacechars);
    TEST(triml);
    TEST(trimr);
    TEST(trim);
    TEST(split);
    TEST(join);
    TEST(convert);
    TEST(urlencode);
    TEST(urldecode);
    TEST(parsequerystring);
    TEST(basename);
    TEST(suffix);
  }
};

}  // namespace NFmiStringToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiStringTools tester" << endl << "======================" << endl;
  NFmiStringToolsTest::tests t;
  return t.run();
}

// ======================================================================
