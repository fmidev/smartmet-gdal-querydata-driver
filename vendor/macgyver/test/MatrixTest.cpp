// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::Matrix
 */
// ======================================================================

#include "Matrix.h"
#include <regression/tframe.h>
#include <iostream>
#include <string>

using namespace std;

namespace MatrixTest
{
// ----------------------------------------------------------------------
/*
 * Test constructors
 */
// ----------------------------------------------------------------------

void constructors()
{
  Fmi::Matrix<int> m1;
  if (!m1.empty())
    TEST_FAILED("Default constructor should yield an empty matrix");

  Fmi::Matrix<int> m2(2, 3);
  if (m2.empty())
    TEST_FAILED("m(2,3) should yield a non-empty matrix");
  if (m2(0, 0) != 0)
    TEST_FAILED("m(2,3) should contain zeros only");

  Fmi::Matrix<int> m3(m2);
  if (m3.empty())
    TEST_FAILED("copy constructor failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test assignment operator
 */
// ----------------------------------------------------------------------

void assignment()
{
  Fmi::Matrix<int> m1(2, 3);
  Fmi::Matrix<int> m2;

  m2 = m1;
  if (m2.empty())
    TEST_FAILED("Assignment failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test size accessors
 */
// ----------------------------------------------------------------------

void size()
{
  Fmi::Matrix<int> m1;

  if (m1.xsize() != 0)
    TEST_FAILED("empty matrix xsize should be 0");
  if (m1.ysize() != 0)
    TEST_FAILED("empty matrix ysize should be 0");
  if (m1.width() != 0)
    TEST_FAILED("empty matrix width should be 0");
  if (m1.height() != 0)
    TEST_FAILED("empty matrix height should be 0");
  if (m1.rows() != 0)
    TEST_FAILED("empty matrix columns should be 0");
  if (m1.columns() != 0)
    TEST_FAILED("empty matrix rows should be 0");

  Fmi::Matrix<int> m2(2, 3);

  if (m2.xsize() != 2)
    TEST_FAILED("matrix(2,3) xsize should be 2");
  if (m2.ysize() != 3)
    TEST_FAILED("matrix(2,3) ysize should be 3");
  if (m2.width() != 2)
    TEST_FAILED("matrix(2,3) width should be 2");
  if (m2.height() != 3)
    TEST_FAILED("matrix(2,3) height should be 3");
  if (m2.rows() != 3)
    TEST_FAILED("matrix(2,3) columns should be 3");
  if (m2.columns() != 2)
    TEST_FAILED("matrix(2,3) rows should be 2");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test elements
 */
// ----------------------------------------------------------------------

void elements()
{
  Fmi::Matrix<int, true> m(2, 3);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 3; j++)
      m(i, j) = i + j;

  if (m(0, 0) != 0)
    TEST_FAILED("m(0,0) should be 0");
  if (m(0, 1) != 1)
    TEST_FAILED("m(0,1) should be 1");
  if (m(0, 2) != 2)
    TEST_FAILED("m(0,2) should be 2");
  if (m(1, 0) != 1)
    TEST_FAILED("m(1,0) should be 1");
  if (m(1, 1) != 2)
    TEST_FAILED("m(1,1) should be 2");
  if (m(1, 2) != 3)
    TEST_FAILED("m(1,2) should be 3");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*
 * Test safety
 */
// ----------------------------------------------------------------------

void safety()
{
  Fmi::Matrix<int, true> m(2, 3);

  try
  {
    int a = m(0, 0);
    a += m(0, 3);
    TEST_FAILED("Access of m(0,3) should have thrown");
  }
  catch (...)
  {
  }

  try
  {
    int a = m(1, 0);
    a += m(-1, 0);
    TEST_FAILED("Access of m(-1,0) should have thrown");
  }
  catch (...)
  {
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
    TEST(constructors);
    TEST(assignment);
    TEST(size);
    TEST(elements);
    TEST(safety);
  }
};

}  // namespace MatrixTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "Matrix" << endl << "======" << endl;
  MatrixTest::tests t;
  return t.run();
}

// ======================================================================
