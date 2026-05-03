// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiSettingsImpl
 */
// ======================================================================

#include "NFmiFileSystem.h"
#include "NFmiSettingsImpl.h"
#include <regression/tframe.h>

#include <fstream>

//! Protection against conflicts with global functions
namespace NFmiSettingsImplTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test the set() method
 */
// ----------------------------------------------------------------------

void set(void)
{
  NFmiSettingsImpl::Instance().Clear();

  NFmiSettingsImpl::Instance().Set("foo", "bar", false);
  if (!NFmiSettingsImpl::Instance().IsSet("foo"))
    TEST_FAILED("set(foo,bar) failed to create variable");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the value() method
 */
// ----------------------------------------------------------------------

void value(void)
{
  NFmiSettingsImpl::Instance().Clear();

  NFmiSettingsImpl::Instance().Set("foo", "bar", false);
  if (!NFmiSettingsImpl::Instance().IsSet("foo"))
    TEST_FAILED("set(foo,bar) failed to create variable");

  NFmiSettingsImpl::Instance().Set("foo2", "${foo}", false);
  if (NFmiSettingsImpl::Instance().Value("foo2") != "bar")
    TEST_FAILED("Failed to expand foo2 = ${foo}");

  NFmiSettingsImpl::Instance().Set("foo3", "${foo2}/${foo}", false);
  if (NFmiSettingsImpl::Instance().Value("foo3") != "bar/bar")
    TEST_FAILED("Failed to expand ${foo2}/${foo}");

  NFmiSettingsImpl::Instance().Set("foo4", "$(foo)/foo", false);
  if (NFmiSettingsImpl::Instance().Value("foo4") != "bar/foo")
    TEST_FAILED("Failed to expand $(foo)/bar");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test the read() method
 */
// ----------------------------------------------------------------------

void read()
{
  {
    NFmiSettingsImpl::Instance().Clear();

    if (!NFmiSettingsImpl::Instance().Read("data/settings.cnf"))
      TEST_FAILED("read(settings.conf) failed");

    if (!NFmiSettingsImpl::Instance().IsSet("var1"))
      TEST_FAILED("var1 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("var2"))
      TEST_FAILED("var2 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("var3"))
      TEST_FAILED("var3 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("var4"))
      TEST_FAILED("var4 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space1::x1"))
      TEST_FAILED("space1::x1 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space1::x2"))
      TEST_FAILED("space1::x2 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space2::y1"))
      TEST_FAILED("space2::y1 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space2::y2"))
      TEST_FAILED("space2::y2 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space3::space4::z1"))
      TEST_FAILED("space3::space4::z1 should be set after reading settings.conf");

    if (!NFmiSettingsImpl::Instance().IsSet("space3::space4::z2"))
      TEST_FAILED("space3::space4::z2 should be set after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("var1") != "1")
      TEST_FAILED("var1 should be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("var2") != "2")
      TEST_FAILED("var2 should be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("var3") != "1")
      TEST_FAILED("var3 should be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("var4") != "2")
      TEST_FAILED("var4 should be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space1::x1") != "1")
      TEST_FAILED("space1::x1 should be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space1::x2") != "2")
      TEST_FAILED("space1::x2 should be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space2::y1") != "1")
      TEST_FAILED("space2::y1 should be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space2::y2") != "2")
      TEST_FAILED("space2::y2 should be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space3::space4::z1") != "1")
      TEST_FAILED("space3::space4::z1 should be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space3::space4::z2") != "2")
      TEST_FAILED("space3::space4::z2 should be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space5::y1") != "1")
      TEST_FAILED("space5::y1 be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space5::y2") != "2")
      TEST_FAILED("space5::y2 be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space5::z1") != "1")
      TEST_FAILED("space5::z1 be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space5::z2") != "2")
      TEST_FAILED("space5::z2 be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space6::space7::y1") != "1")
      TEST_FAILED("space6::space7::y1 be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space6::space7::y2") != "2")
      TEST_FAILED("space6::space7::y2 be 2 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space6::space7::z1") != "1")
      TEST_FAILED("space6::space7::z1 be 1 after reading settings.conf");

    if (NFmiSettingsImpl::Instance().Value("space6::space7::z2") != "2")
      TEST_FAILED("space6::space7::z2 be 2 after reading settings.conf");
  }

  {
    NFmiSettingsImpl::Instance().Clear();

    if (!NFmiSettingsImpl::Instance().Read("data/settings_spaces.cnf"))
      TEST_FAILED("read(settings_spaces.conf) failed");
  }

  try
  {
    NFmiSettingsImpl::Instance().Clear();

    if (NFmiSettingsImpl::Instance().Read("data/settings_newline.cnf"))
      TEST_FAILED("read(settings_newline.conf) should have failed");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test NFmiSettingsImpl::save()
 */
// ----------------------------------------------------------------------

void save()
{
  using namespace std;

  NFmiSettingsImpl::Instance().Clear();

  // create the dummy configuration files
  const string conffile1 = "test1.conf";
  const string conffile2 = "test2.conf";

  {
    ofstream out1(conffile1.c_str(), ios::out);
    if (!out1)
      TEST_FAILED("Failed to open " + conffile1 + " for writing");
    out1 << "a = a" << endl << "b = b" << endl << "c\t=\tc" << endl;
    out1.close();
  }

  {
    ofstream out2(conffile2.c_str(), ios::out);
    if (!out2)
      TEST_FAILED("Failed to open " + conffile2 + " for writing");
    out2 << "A = A" << endl << "B = B" << endl << "c\t=\tc" << endl;
    out2.close();
  }

  if (!NFmiSettingsImpl::Instance().Read(conffile1))
    TEST_FAILED("Failed to read settings from " + conffile1);
  if (!NFmiSettingsImpl::Instance().Read(conffile2))
    TEST_FAILED("Failed to read settings from " + conffile2);

  NFmiSettingsImpl::Instance().Set("a", "new a", false);
  NFmiSettingsImpl::Instance().Set("b", "new b", false);
  NFmiSettingsImpl::Instance().Set("c", "new c", false);

  NFmiSettingsImpl::Instance().Set("A", "new A", false);
  NFmiSettingsImpl::Instance().Set("B", "new B", false);
  NFmiSettingsImpl::Instance().Set("C", "new C", false);

  NFmiSettingsImpl::Instance().Save();

  // now test the saved values
  NFmiSettingsImpl::Instance().Clear();
  if (!NFmiSettingsImpl::Instance().Read(conffile1))
    TEST_FAILED("Failed to read settings from " + conffile1);
  if (NFmiSettingsImpl::Instance().Value("a") != "new a")
    TEST_FAILED("Failed to change 'a' to 'new a'");
  if (NFmiSettingsImpl::Instance().Value("b") != "new b")
    TEST_FAILED("Failed to change 'b' to 'new b'");
  if (NFmiSettingsImpl::Instance().Value("c") != "c")
    TEST_FAILED("Failed to keep 'c' as 'c'");

  NFmiSettingsImpl::Instance().Clear();
  if (!NFmiSettingsImpl::Instance().Read(conffile2))
    TEST_FAILED("Failed to read settings from " + conffile2);
  if (NFmiSettingsImpl::Instance().Value("A") != "new A")
    TEST_FAILED("Failed to change 'A' to 'new A'");
  if (NFmiSettingsImpl::Instance().Value("B") != "new B")
    TEST_FAILED("Failed to change 'b' to 'new b'");
  if (NFmiSettingsImpl::Instance().Value("c") != "new c")
    TEST_FAILED("Failed to change 'c' to 'new c'");

  NFmiFileSystem::RemoveFile(conffile1);
  NFmiFileSystem::RemoveFile(conffile2);

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * Test NFmiSettingsImpl::ToString()
 */
// ----------------------------------------------------------------------

void tostring()
{
  using namespace std;

  NFmiSettingsImpl::Instance().Clear();

  NFmiSettingsImpl::Instance().Set("foo", "bar", false);
  if (NFmiSettingsImpl::Instance().ToString() != "foo = bar\n")
    TEST_FAILED("Failed to convert settings to string after foo=bar");

  NFmiSettingsImpl::Instance().Set("foo2", "bar2", false);
  if (NFmiSettingsImpl::Instance().ToString() != "foo = bar\nfoo2 = bar2\n")
    TEST_FAILED("Failed to convert settings to string after foo2=bar2");

  NFmiSettingsImpl::Instance().Set("foo", "bar2", false);
  if (NFmiSettingsImpl::Instance().ToString() != "foo = bar2\nfoo2 = bar2\n")
    TEST_FAILED("Failed to convert settings to string after foo=bar2");

  NFmiSettingsImpl::Instance().Set("bar", "foo", false);

  if (NFmiSettingsImpl::Instance().ToString("foo") != "foo = bar2\nfoo2 = bar2\n")
    TEST_FAILED("Failed to filter out foo* settings");
  if (NFmiSettingsImpl::Instance().ToString("bar") != "bar = foo\n")
    TEST_FAILED("Failed to filter out bar* settings");

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
    TEST(set);
    TEST(value);
    TEST(read);
    TEST(save);
    TEST(tostring);
  }
};

}  // namespace NFmiSettingsImplTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiSettingsImpl tester" << endl << "=======================" << endl;
  NFmiSettingsImplTest::tests t;
  return t.run();
}

// ======================================================================
