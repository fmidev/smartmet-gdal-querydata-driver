// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiInterpolation
 */
// ======================================================================

#include "NFmiGlobals.h"
#include "NFmiInterpolation.h"
#include "NFmiPoint.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <cmath>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiInterpolationTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test linear 1d interpolation from coordinates
 */
// ----------------------------------------------------------------------

void linearcoords()
{
  using namespace NFmiInterpolation;

  if (abs(Linear(0, 0, 1, 10, 20) - 10) > 1e-5)
    TEST_FAILED("Test Linear(0,0,1,10,20)=10 failed");

  if (abs(Linear(1, 0, 1, 10, 20) - 20) > 1e-5)
    TEST_FAILED("Test Linear(1,0,1,10,20)=20 failed");

  if (abs(Linear(0.5, 0, 1, 10, 20) - 15) > 1e-5)
    TEST_FAILED("Test Linear(0.5,0,1,10,20)=15 failed");

  if (Linear(0, 0, 1, kFloatMissing, 20) != kFloatMissing)
    TEST_FAILED("Test Linear(0,0,1,?,20)=? failed");

  if (Linear(1, 0, 1, kFloatMissing, 20) != 20)
    TEST_FAILED("Test Linear(1,0,1,?,20)=20 failed");

  if (Linear(0.5, 0, 1, kFloatMissing, 20) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,0,1,?,20)=? failed");

  if (Linear(0, 0, 1, 10, kFloatMissing) != 10)
    TEST_FAILED("Test Linear(0,0,1,10,?)=10 failed");

  if (Linear(1, 0, 1, 10, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(1,0,1,10,?)=? failed");

  if (Linear(0.5, 0, 1, 10, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,0,1,10,?)=? failed");

  if (Linear(0, 0, 1, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0,0,1,?,?)=? failed");

  if (Linear(1, 0, 1, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(1,0,1,?,?)=? failed");

  if (Linear(0.5, 0, 1, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,0,1,?,?)=? failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test linear 1d interpolation from relative coordinates
 */
// ----------------------------------------------------------------------

void linearfactor()
{
  using namespace NFmiInterpolation;

  if (abs(Linear(0, 10, 20) - 10) > 1e-5)
    TEST_FAILED("Test Linear(0,10,20)=10 failed");

  if (abs(Linear(1, 10, 20) - 20) > 1e-5)
    TEST_FAILED("Test Linear(1,10,20)=20 failed");

  if (abs(Linear(0.5, 10, 20) - 15) > 1e-5)
    TEST_FAILED("Test Linear(0.5,10,20)=15 failed");

  if (Linear(0, kFloatMissing, 20) != kFloatMissing)
    TEST_FAILED("Test Linear(0,?,20)=? failed");

  if (Linear(1, kFloatMissing, 20) != 20)
    TEST_FAILED("Test Linear(1,?,20)=20 failed");

  if (Linear(0.5, kFloatMissing, 20) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,?,20)=? failed");

  if (Linear(0, 10, kFloatMissing) != 10)
    TEST_FAILED("Test Linear(0,10,?)=10 failed");

  if (Linear(1, 10, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(1,10,?)=? failed");

  if (Linear(0.5, 10, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,10,?)=? failed");

  if (Linear(0, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0,?,?)=? failed");

  if (Linear(1, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(1,?,?)=? failed");

  if (Linear(0.5, kFloatMissing, kFloatMissing) != kFloatMissing)
    TEST_FAILED("Test Linear(0.5,?,?)=? failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test linear 1d interpolation from coordinates with modulo
 */
// ----------------------------------------------------------------------

void linearmodulo()
{
  using namespace NFmiInterpolation;

  if (abs(ModLinear(0, 0, 1, 350, 30, 360) - 350) > 1e-5)
    TEST_FAILED("Test ModLinear(0,0,1,350,30,360)=350 failed");

  if (abs(ModLinear(1, 0, 1, 350, 30, 360) - 30) > 1e-5)
    TEST_FAILED("Test ModLinear(1,0,1,350,30,360)=30 failed");

  if (abs(ModLinear(0.5, 0, 1, 350, 30, 360) - 10) > 1e-5)
    TEST_FAILED("Test ModLinear(0.5,0,1,350,30,360)=10 failed");

  if (ModLinear(0, 0, 1, kFloatMissing, 30, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(0,0,1,?,30,360)=? failed");

  if (ModLinear(1, 0, 1, kFloatMissing, 30, 360) != 30)
    TEST_FAILED("Test ModLinear(1,0,1,?,30,360)=30 failed");

  if (ModLinear(0.5, 0, 1, kFloatMissing, 30, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(0.5,0,1,?,30,360)=? failed");

  if (ModLinear(0, 0, 1, 350, kFloatMissing, 360) != 350)
    TEST_FAILED("Test ModLinear(0,0,1,350,?,360)=350 failed");

  if (ModLinear(1, 0, 1, 350, kFloatMissing, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(1,0,1,350,?,360)=? failed");

  if (ModLinear(0.5, 0, 1, 350, kFloatMissing, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(0.5,0,1,350,?,360)=? failed");

  if (ModLinear(0, 0, 1, kFloatMissing, kFloatMissing, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(0,0,1,?,?,360)=? failed");

  if (ModLinear(1, 0, 1, kFloatMissing, kFloatMissing, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(1,0,1,?,?,360)=? failed");

  if (ModLinear(0.5, 0, 1, kFloatMissing, kFloatMissing, 360) != kFloatMissing)
    TEST_FAILED("Test ModLinear(0.5,0,1,?,?,360)=? failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test linear 2d interpolation
 */
// ----------------------------------------------------------------------

void bilinear()
{
  using namespace NFmiInterpolation;

  // We use this rectangle:
  //
  // (0,1)   (1,1)
  //
  // (0,0)   (1,0)
  //
  // with values
  //
  //  3      4
  //
  //  1      2

  const double bl = 1;
  const double br = 2;
  const double tl = 3;
  const double tr = 4;

  if (BiLinear(0, 0, tl, tr, bl, br) != bl)
    TEST_FAILED("Test BiLinear(0,0,tl,tr,bl,br) = bl failed");
  if (BiLinear(1, 0, tl, tr, bl, br) != br)
    TEST_FAILED("Test BiLinear(1,0,tl,tr,bl,br) = br failed");
  if (BiLinear(0, 1, tl, tr, bl, br) != tl)
    TEST_FAILED("Test BiLinear(0,1,tl,tr,bl,br) = tl failed");
  if (BiLinear(1, 1, tl, tr, bl, br) != tr)
    TEST_FAILED("Test BiLinear(1,1,tl,tr,bl,br) = tr failed");

  if (BiLinear(0.5, 0, tl, tr, bl, br) != 1.5)
    TEST_FAILED("Test BiLinear(0.5,0,tl,tr,bl,br) = 1.5 failed");
  if (BiLinear(0, 0.5, tl, tr, bl, br) != 2)
    TEST_FAILED("Test BiLinear(0,0.5,tl,tr,bl,br) = 2 failed");
  if (BiLinear(1, 0.5, tl, tr, bl, br) != 3)
    TEST_FAILED("Test BiLinear(1,0.5,tl,tr,bl,br) = 3 failed");
  if (BiLinear(0.5, 1, tl, tr, bl, br) != 3.5)
    TEST_FAILED("Test BiLinear(0.5,1,tl,tr,bl,br) = 3.5 failed");

  if (BiLinear(0.5, 0.5, tl, tr, bl, br) != 2.5)
    TEST_FAILED("Test BiLinear(0.5,0.5,tl,tr,bl,br) = 2.5 failed");

  if (BiLinear(0.25, 0.5, tl, tr, bl, br) != 2.25)
    TEST_FAILED("Test BiLinear(0.25,0.5,tl,tr,bl,br) = 2.25 failed");

  if (BiLinear(0.25, 0.25, tl, tr, bl, br) != 1.75)
    TEST_FAILED("Test BiLinear(0.25,0.25,tl,tr,bl,br) = 1.75 failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test linear 2d interpolation
 */
// ----------------------------------------------------------------------

void bilinearpoint()
{
  using namespace NFmiInterpolation;

  // We use this rectangle:
  //
  // (0,1)   (1,1)
  //
  // (0,0)   (1,0)
  //
  // with values
  //
  //  (3,-3)  (4,-4)
  //
  //  (1,-1)  (2,-2)

  const NFmiPoint bl(1, -1);
  const NFmiPoint br(2, -2);
  const NFmiPoint tl(3, -3);
  const NFmiPoint tr(4, -4);

  if (BiLinear(0, 0, tl, tr, bl, br) != bl)
    TEST_FAILED("Test BiLinear(0,0,tl,tr,bl,br) = bl failed");
  if (BiLinear(1, 0, tl, tr, bl, br) != br)
    TEST_FAILED("Test BiLinear(1,0,tl,tr,bl,br) = br failed");
  if (BiLinear(0, 1, tl, tr, bl, br) != tl)
    TEST_FAILED("Test BiLinear(0,1,tl,tr,bl,br) = tl failed");
  if (BiLinear(1, 1, tl, tr, bl, br) != tr)
    TEST_FAILED("Test BiLinear(1,1,tl,tr,bl,br) = tr failed");

  if (BiLinear(0.5, 0, tl, tr, bl, br) != NFmiPoint(1.5, -1.5))
    TEST_FAILED("Test BiLinear(0.5,0,tl,tr,bl,br) = (1.5,-1.5) failed");
  if (BiLinear(0, 0.5, tl, tr, bl, br) != NFmiPoint(2, -2))
    TEST_FAILED("Test BiLinear(0,0.5,tl,tr,bl,br) = (2,-2) failed");
  if (BiLinear(1, 0.5, tl, tr, bl, br) != NFmiPoint(3, -3))
    TEST_FAILED("Test BiLinear(1,0.5,tl,tr,bl,br) = (3,-3) failed");
  if (BiLinear(0.5, 1, tl, tr, bl, br) != NFmiPoint(3.5, -3.5))
    TEST_FAILED("Test BiLinear(0.5,1,tl,tr,bl,br) = (3.5,-3.5) failed");

  if (BiLinear(0.5, 0.5, tl, tr, bl, br) != NFmiPoint(2.5, -2.5))
    TEST_FAILED("Test BiLinear(0.5,0.5,tl,tr,bl,br) = NFmiPoint(2.5,-2.5) failed");

  if (BiLinear(0.25, 0.5, tl, tr, bl, br) != NFmiPoint(2.25, -2.25))
    TEST_FAILED("Test BiLinear(0.25,0.5,tl,tr,bl,br) = NFmiPoint(2.25,-2.25) failed");

  if (BiLinear(0.25, 0.25, tl, tr, bl, br) != NFmiPoint(1.75, -1.75))
    TEST_FAILED("Test BiLinear(0.25,0.25,tl,tr,bl,br) = NFmiPoint(1.75,-1.75) failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test linear 2d interpolation with modulo
 */
// ----------------------------------------------------------------------

void modbilinear()
{
  using namespace NFmiInterpolation;

  // We use this rectangle:
  //
  // (0,1)   (1,1)
  //
  // (0,0)   (1,0)
  //
  // with values
  //
  //  350    340
  //
  //  0     10

  const double bl = 0;
  const double br = 10;
  const double tl = 350;
  const double tr = 340;

  if (ModBiLinear(0, 0, tl, tr, bl, br, 360) != bl)
    TEST_FAILED("Test ModBiLinear(0,0,tl,tr,bl,br,360) = bl failed");
  if (ModBiLinear(1, 0, tl, tr, bl, br, 360) != br)
    TEST_FAILED("Test ModBiLinear(1,0,tl,tr,bl,br,360) = br failed");
  if (ModBiLinear(0, 1, tl, tr, bl, br, 360) != tl)
    TEST_FAILED("Test ModBiLinear(0,1,tl,tr,bl,br,360) = tl failed");
  if (ModBiLinear(1, 1, tl, tr, bl, br, 360) != tr)
    TEST_FAILED("Test ModBiLinear(1,1,tl,tr,bl,br,360) = tr failed");

  if (ModBiLinear(0.5, 0, tl, tr, bl, br, 360) != 5)
    TEST_FAILED("Test ModBiLinear(0.5,0,tl,tr,bl,br,360) = 5 failed");
  if (ModBiLinear(0, 0.5, tl, tr, bl, br, 360) != 355)
    TEST_FAILED("Test ModBiLinear(0,0.5,tl,tr,bl,br,360) = 355 failed");
  if (ModBiLinear(1, 0.5, tl, tr, bl, br, 360) != 355)
    TEST_FAILED("Test ModBiLinear(1,0.5,tl,tr,bl,br,360) = 355 failed");
  if (ModBiLinear(0.5, 1, tl, tr, bl, br, 360) != 345)
    TEST_FAILED("Test ModBiLinear(0.5,1,tl,tr,bl,br,360) = 345 failed");

  if (ModBiLinear(0.5, 0.5, tl, tr, bl, br, 360) != 355)
    TEST_FAILED("Test ModBiLinear(0.5,0.5,tl,tr,bl,br,360) = 355 failed");

  if (ModBiLinear(0.25, 0.5, tl, tr, bl, br, 360) != 355)
    TEST_FAILED("Test ModBiLinear(0.25,0.5,tl,tr,bl,br,360) = 355 failed");

  if (ModBiLinear(0.25, 0.25, tl, tr, bl, br, 360) != 358.75)
    TEST_FAILED("Test ModBiLinear(0.25,0.25,tl,tr,bl,br,360) = 358.75 failed");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test WindInterpolator class
 */
// ----------------------------------------------------------------------

void windinterpolator()
{
  // Some simple 1D cases first

  NFmiInterpolation::WindInterpolator calc;
  using NFmiStringTools::Convert;

  double res;

  calc(10, 150, 1);
  if ((res = calc.Speed()) != 10)
    TEST_FAILED("Should return speed 10 when it is the only number, not " + Convert(res));
  res = calc.Direction();
  if (res < 149.99999 || res > 150.000001)
    TEST_FAILED("Should return direction 150 when it is the only number, not " + Convert(res));
  calc.Reset();
  calc(10, 0, 0.5);
  calc(20, 90, 0.5);
  if ((res = calc.Speed()) != 15)
    TEST_FAILED("Mean speed between 10...20 should be 15, not " + Convert(res));
  if ((res = calc.Direction()) != 45)
    TEST_FAILED("Mean direction between 0...90 should be 45, not " + Convert(res));

  // This is a design requirement, we do not want 10*sqrt(2)/2, but 10!
  calc.Reset();
  calc(10, 0, 0.5);
  calc(10, 90, 0.5);
  if ((res = calc.Speed()) != 10)
    TEST_FAILED("Mean speed between 10...10 should be 10, not " + Convert(res));
  if ((res = calc.Direction()) != 45)
    TEST_FAILED("Mean direction between 0...90 should be 45, not " + Convert(res));

  // This is a design requirement, we do not want variable wind
  calc.Reset();
  calc(10, 0, 0.5);
  calc(10, 180, 0.5);
  if ((res = calc.Direction()) != 0)
    TEST_FAILED("Mean direction between 0...180 with equal weights should be 0, not " +
                Convert(res));

  // A special case of +-dir values. The direction with most weight will win.
  // In this case 0.37+0.24+0.15 wins

  calc.Reset();
  calc(10, 20, 0.37);
  calc(10, 20, 0.24);
  calc(10, 200, 0.23);
  calc(10, 20, 0.15);
  if ((res = calc.Speed()) != 10)
    TEST_FAILED("Mean speed should be 10 for 4 identical values, not " + Convert(res));
  if (abs((res = calc.Direction()) - 20) > 0.001)
    TEST_FAILED("Mean direction should be 20 for 20,20,20,200 test, not " + Convert(res));

  // Same special case with 2 values

  calc.Reset();
  calc(10, 20, 0.49);
  calc(10, 200, 0.51);
  if (abs((res = calc.Direction()) - 200) > 0.001)
    TEST_FAILED("20..200 mean should be 200 when latter is closer, not " + Convert(res));

  calc.Reset();
  calc(10, 20, 0.51);
  calc(10, 200, 0.49);
  if (abs((res = calc.Direction()) - 20) > 0.001)
    TEST_FAILED("20..200 mean should be 20 when former is closer, not " + Convert(res));

  // A normal case

  calc.Reset();
  calc(10, 0, 0.25);
  calc(20, 90, 0.75);
  if (abs((res = calc.Speed()) - 17.50) > 0.001)
    TEST_FAILED("10..20 (0.25,0.75) mean should be 17.5, not " + Convert(res));
  if (abs((res = calc.Direction()) - 71.5651) > 0.001)
    TEST_FAILED("0..90 (0.25,0.75) mean should be 71.5651, not " + Convert(res));

  // Wrap around test. Note that we do not get exactly 5 or exactly 355 since
  // 2D weighting does not work that way in polar coordinates.

  calc.Reset();
  calc(10, 350, 0.25);
  calc(10, 10, 0.75);
  if (abs((res = calc.Direction()) - 5.03837) > 0.001)
    TEST_FAILED("350..10 (0.25,0.75) mean should be 5.03837, not " + Convert(res));

  calc.Reset();
  calc(10, 350, 0.75);
  calc(10, 10, 0.25);
  if (abs((res = calc.Direction()) - 354.962) > 0.001)
    TEST_FAILED("350..10 (0.75,0.25) mean should be 354.962, not " + Convert(res));

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
    TEST(linearcoords);
    TEST(linearfactor);
    TEST(linearmodulo);
    TEST(bilinear);
    TEST(bilinearpoint);
    TEST(modbilinear);
    TEST(windinterpolator);
  }
};

}  // namespace NFmiInterpolationTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiInterpolation tester" << endl << "========================" << endl;
  NFmiInterpolationTest::tests t;
  return t.run();
}

// ======================================================================
