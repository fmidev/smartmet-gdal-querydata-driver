// ======================================================================
/*!
 * \file
 * \brief Regression tests for CharsetTools
 */
// ======================================================================

#include "CharsetTools.h"
#include <regression/tframe.h>
#include <string>

using std::string;

// Protection against conflicts with global functions
namespace CharsetToolsTest
{
// ----------------------------------------------------------------------

void toupper()
{
  if (Fmi::toupper('a') != 'A')
    TEST_FAILED("Failed to convert a to A");
  if (Fmi::toupper('A') != 'A')
    TEST_FAILED("Failed to convert A to A");
  if (Fmi::toupper('ö') != (unsigned char)'Ö')
    TEST_FAILED("Failed to convert ö to Ö");
  if (Fmi::toupper('1') != '1')
    TEST_FAILED("Failed to convert 1 to 1");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void tolower()
{
  if (Fmi::tolower('A') != 'a')
    TEST_FAILED("Failed to convert A to a");
  if (Fmi::tolower('a') != 'a')
    TEST_FAILED("Failed to convert a to a");
  if (Fmi::tolower('Ö') != (unsigned char)'ö')
    TEST_FAILED("Failed to convert ö to Ö");
  if (Fmi::tolower('1') != '1')
    TEST_FAILED("Failed to convert 1 to 1");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void tonordic()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void tolowernordic()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void utf8_to_latin1()
{
  string res;
  if ((res = Fmi::utf8_to_latin1("Janakkala")) != "Janakkala")
    TEST_FAILED("Failed on string 'Janakkala': " + res);

  if ((res = Fmi::utf8_to_latin1("Ã„Ã¤nekoski")) != "Äänekoski")
    TEST_FAILED("Failed on string 'Äänekoski': " + res);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void latin1_to_utf8()
{
  string res;
  if ((res = Fmi::latin1_to_utf8("Janakkala")) != "Janakkala")
    TEST_FAILED("Failed on string 'Janakkala': " + res);

  if ((res = Fmi::latin1_to_utf8("Äänekoski")) != "Ã„Ã¤nekoski")
    TEST_FAILED("Failed on string 'Äänekoski': " + res);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void utf16_to_utf8()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void utf8_to_utf16()
{
  TEST_NOT_IMPLEMENTED();
}
// ----------------------------------------------------------------------

void utf8_validation()
{
  const auto check = [](bool expected, const std::string& src)
  {
    bool result = Fmi::is_utf8(src);
    if (result ^ expected)
    {
      std::ostringstream msg;
      msg << "is_utf8('";
      for (std::size_t i = 0; i < src.length(); i++)
      {
        if (src[i] > 0x20 && src[i] < 0x7F)
        {
          msg << src[i];
        }
        else
        {
          char buf[10];
          snprintf(buf, sizeof(buf), "\\%03o", (unsigned char)src[i]);
          msg << buf;
        }
      }
      msg << "')=" << result << " (expected " << expected << ")";
      std::string err = msg.str();
      TEST_FAILED(err.c_str());
    }
  };

  // Examples: http://www.zedwood.com/article/cpp-is-valid-utf8-string-function
  check(true, "Hello world");
  check(true, "ol\xc3\xa1 mundo");
  check(true, "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c");
  check(false, "\xa0\xa1");

  // Examples: https://stackoverflow.com/questions/1301402/example-invalid-utf8-string
  check(true, "\xc3\xb1");                   // Valid 2 Octet Sequence
  check(false, "\xc3\x28");                  // Invalid 2 Octet Sequence
  check(true, "\xe2\x82\xa1");               // Valid 3 Octet Sequence
  check(false, "\xe2\x28\xa1");              // Invalid 3 Octet Sequence (in 2nd Octet)
  check(false, "\xe2\x82\x28");              // Invalid 3 Octet Sequence (in 3rd Octet)
  check(true, "\xf0\x90\x8c\xbc");           // Valid 4 Octet Sequence
  check(false, "\xf0\x28\x8c\xbc");          // Invalid 4 Octet Sequence (in 2nd Octet)
  check(false, "\xf0\x90\x28\xbc");          // Invalid 4 Octet Sequence (in 3rd Octet)
  check(false, "\xf0\x28\x8c\x28");          // Invalid 4 Octet Sequence (in 4th Octet)
  check(false, "\xf8\xa1\xa1\xa1\xa1");      // Valid 5 Octet Sequence (but not Unicode!)
  check(false, "\xfc\xa1\xa1\xa1\xa1\xa1");  // Valid 6 Octet Sequence (but not Unicode!)

  // Examples: smartmetd logs
  check(false,
        "/wfs?place=Sein\xE4joki&request=getFeature&storedquery_id=fmi::forecast::hirlam::surface::"
        "point::simple");

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
    TEST(toupper);
    TEST(tolower);
    TEST(tonordic);
    TEST(tolowernordic);
    TEST(utf8_to_latin1);
    TEST(latin1_to_utf8);
    TEST(utf16_to_utf8);
    TEST(utf8_to_utf16);
    TEST(utf8_validation);
  }
};

}  // namespace CharsetToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "CharsetTools tester" << endl << "===================" << endl;
  CharsetToolsTest::tests t;
  return t.run();
}

// ======================================================================
