// ======================================================================
/*!
 * \file
 * \brief Regression tests for Base64
 */
// ======================================================================

#include "Base64.h"
#include <regression/tframe.h>
#include <string>

using std::string;

// Protection against conflicts with global functions
namespace Base64Test
{
// ----------------------------------------------------------------------

void encode()
{
  using Fmi::Base64::encode;
  string result;

  if ((result = encode("1")) != "MQ==")
    TEST_FAILED("Failed to encode string '1', got result '" + result + "'");
  if ((result = encode("12")) != "MTI=")
    TEST_FAILED("Failed to encode string '12', got result '" + result + "'");
  if ((result = encode("123")) != "MTIz")
    TEST_FAILED("Failed to encode string '123', got result '" + result + "'");
  if ((result = encode("1234")) != "MTIzNA==")
    TEST_FAILED("Failed to encode string '1234', got result '" + result + "'");
  if ((result = encode("12345")) != "MTIzNDU=")
    TEST_FAILED("Failed to encode string '12345', got result '" + result + "'");
  if ((result = encode("123456")) != "MTIzNDU2")
    TEST_FAILED("Failed to encode string '123456', got result '" + result + "'");
  if ((result = encode("1234567")) != "MTIzNDU2Nw==")
    TEST_FAILED("Failed to encode string '1234567', got result '" + result + "'");
  if ((result = encode("12345678")) != "MTIzNDU2Nzg=")
    TEST_FAILED("Failed to encode string '12345678', got result '" + result + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void decode()
{
  using Fmi::Base64::decode;
  string result;

  if ((result = decode("MQ==")) != "1")
    TEST_FAILED("Failed to decode string 'MQ==', got result '" + result + "'");
  if ((result = decode("MTI=")) != "12")
    TEST_FAILED("Failed to decode string 'MTI=', got result '" + result + "'");
  if ((result = decode("MTIz")) != "123")
    TEST_FAILED("Failed to decode string 'MTIz', got result '" + result + "'");
  if ((result = decode("MTIzNA==")) != "1234")
    TEST_FAILED("Failed to decode string 'MTIzNA==', got result '" + result + "'");
  if ((result = decode("MTIzNDU=")) != "12345")
    TEST_FAILED("Failed to decode string 'MTIzNDU=', got result '" + result + "'");
  if ((result = decode("MTIzNDU2")) != "123456")
    TEST_FAILED("Failed to decode string 'MTIzNDU2', got result '" + result + "'");
  if ((result = decode("MTIzNDU2Nw==")) != "1234567")
    TEST_FAILED("Failed to decode string 'MTIzNDU2Nw==', got result '" + result + "'");
  if ((result = decode("MTIzNDU2Nzg=")) != "12345678")
    TEST_FAILED("Failed to decode string 'MTIzNDU2Nzg=', got result '" + result + "'");

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
    TEST(encode);
    TEST(decode);
  }
};

}  // namespace Base64Test

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "Base64 tester" << endl << "=============" << endl;
  Base64Test::tests t;
  return t.run();
}

// ======================================================================
