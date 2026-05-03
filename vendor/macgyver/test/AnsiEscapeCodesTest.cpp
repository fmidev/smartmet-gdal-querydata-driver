// ======================================================================
/*!
 * \file
 * \brief Regression tests for class AnsiEscapeCodes
 */
// ======================================================================

#include "AnsiEscapeCodes.h"
#include <regression/tframe.h>

template <typename T>
std::string tostring(const T& obj)
{
  std::ostringstream out;
  out << obj;
  return out.str();
}

// Protection against conflicts with global functions
namespace AnsiEscapeCodesTest
{
void bold()
{
  std::cout << ANSI_BOLD_ON << "Bold" << ANSI_BOLD_OFF << ' ';
  TEST_PASSED();
}

void italic()
{
  std::cout << ANSI_ITALIC_ON << "Italic" << ANSI_ITALIC_OFF << ' ';
  TEST_PASSED();
}

void underline()
{
  std::cout << ANSI_UNDERLINE_ON << "Underline" << ANSI_UNDERLINE_OFF << ' ';
  TEST_PASSED();
}

void inverse()
{
  std::cout << ANSI_INVERSE_ON << "Inverse" << ANSI_INVERSE_OFF << ' ';
  TEST_PASSED();
}

void strike()
{
  std::cout << ANSI_STRIKE_ON << "Strike" << ANSI_STRIKE_OFF << ' ';
  TEST_PASSED();
}

void black()
{
  std::cout << ANSI_FG_BLACK << "Black" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void red()
{
  std::cout << ANSI_FG_RED << "Red" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void green()
{
  std::cout << ANSI_FG_GREEN << "Green" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void yellow()
{
  std::cout << ANSI_FG_YELLOW << "Yellow" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void blue()
{
  std::cout << ANSI_FG_BLUE << "Blue" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void magenta()
{
  std::cout << ANSI_FG_MAGENTA << "Magenta" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void cyan()
{
  std::cout << ANSI_FG_CYAN << "Cyan" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void white()
{
  std::cout << ANSI_FG_WHITE << "White" << ANSI_FG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_black()
{
  std::cout << ANSI_BG_BLACK << "Black" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_red()
{
  std::cout << ANSI_BG_RED << "Red" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_green()
{
  std::cout << ANSI_BG_GREEN << "Green" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_yellow()
{
  std::cout << ANSI_BG_YELLOW << "Yellow" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_blue()
{
  std::cout << ANSI_BG_BLUE << "Blue" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_magenta()
{
  std::cout << ANSI_BG_MAGENTA << "Magenta" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_cyan()
{
  std::cout << ANSI_BG_CYAN << "Cyan" << ANSI_BG_DEFAULT << ' ';
  TEST_PASSED();
}

void bg_white()
{
  std::cout << ANSI_BG_WHITE << "White" << ANSI_BG_DEFAULT << ' ';
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
    TEST(bold);
    TEST(italic);
    TEST(underline);
    TEST(inverse);
    TEST(strike);
    TEST(black);
    TEST(red);
    TEST(green);
    TEST(yellow);
    TEST(blue);
    TEST(magenta);
    TEST(cyan);
    TEST(white);
    TEST(bg_black);
    TEST(bg_red);
    TEST(bg_green);
    TEST(bg_yellow);
    TEST(bg_blue);
    TEST(bg_magenta);
    TEST(bg_cyan);
    TEST(bg_white);
  }
};

}  // namespace AnsiEscapeCodesTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "AnsiEscapeCodes tester" << endl << "=======================" << endl;
  AnsiEscapeCodesTest::tests t;
  return t.run();
}

// ======================================================================
