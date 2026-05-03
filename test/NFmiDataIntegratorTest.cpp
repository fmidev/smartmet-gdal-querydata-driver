// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiDataIntegrator
 */
// ======================================================================

#include "NFmiDataIntegrator.h"
#include "NFmiDataModifierAvg.h"
#include "NFmiDataModifierMax.h"
#include "NFmiDataModifierMin.h"
#include "NFmiDataModifierSum.h"
#include "NFmiGrid.h"
#include "NFmiIndexMask.h"
#include "NFmiIndexMaskSource.h"
#include "NFmiIndexMaskTools.h"
#include "NFmiStreamQueryData.h"
#include "NFmiSvgPath.h"
#include <regression/tframe.h>
#include <fstream>

//! Protection against conflicts with global functions
namespace NFmiDataIntegratorTest
{
NFmiStreamQueryData theQD;
const NFmiGrid* theGrid;

NFmiSvgPath theSuomi;

void read_querydata(const std::string& theFilename)
{
  theQD.ReadData(theFilename);
  theGrid = theQD.QueryInfoIter()->Grid();
}

void read_svg()
{
  std::ifstream insuomi("data/suomi.svg", std::ios::in);
  insuomi >> theSuomi;
  insuomi.close();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Integrate() over time
 */
// ----------------------------------------------------------------------

void integrate_time(void)
{
  using namespace std;
  using namespace NFmiDataIntegrator;

  NFmiFastQueryInfo* q = theQD.QueryInfoIter();
  q->First();
  q->Param(kFmiTemperature);
  NFmiMetTime time1 = q->Time();
  NFmiMetTime time2 = time1;
  time2.ChangeByHours(10);

  {
    NFmiDataModifierMax modifier;
    float result = Integrate(*q, time1, time2, modifier);
    float expected = 7.77182;

    if (std::abs(result - expected) > 0.1)
      TEST_FAILED("max over time failed");
  }

  {
    NFmiDataModifierMin modifier;
    float result = Integrate(*q, time1, time2, modifier);
    float expected = 5.9;

    if (std::abs(result - expected) > 0.1)
      TEST_FAILED("min over time failed");
  }

  {
    NFmiDataModifierSum modifier;
    float result = Integrate(*q, time1, time2, modifier);
    float expected = 75.5807;

    if (std::abs(result - expected) > 0.1)
      TEST_FAILED("sum over time failed");
  }

  {
    NFmiDataModifierAvg modifier;
    float result = Integrate(*q, time1, time2, modifier);
    float expected = 6.87097;

    if (std::abs(result - expected) > 0.1)
      TEST_FAILED("average over time failed");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Integrate() over space
 */
// ----------------------------------------------------------------------

void integrate_space(void)
{
  using namespace std;
  using namespace NFmiDataIntegrator;
  using namespace NFmiIndexMaskTools;

  NFmiFastQueryInfo* q = theQD.QueryInfoIter();
  q->First();
  q->Param(kFmiTemperature);

  // this covers a square of 12 points
  // the temperature values are:
  //
  // -0.7 -0.2 -0.3
  //  0.2  1.8  1.3
  //  1.4  2.3  2.1
  //  2.0  2.6  2.3
  //
  // max  =  2.6
  // min  = -0.7
  // sum  = 14.8
  // mean = 1.2333...

  NFmiIndexMask helsinki = MaskDistance(*theGrid, NFmiPoint(25, 60), 50);

  // too many to check manually
  NFmiIndexMask suomi = MaskInside(*theGrid, theSuomi);

  {
    NFmiDataModifierMax modifier;
    float result = Integrate(*q, helsinki, modifier);
    float expected = 2.6;

    if (result != expected)
      TEST_FAILED("maximum over helsinki failed");
  }

  {
    NFmiDataModifierMin modifier;
    float result = Integrate(*q, helsinki, modifier);
    float expected = -0.7;

    if (result != expected)
      TEST_FAILED("minimum over helsinki failed");
  }

  {
    NFmiDataModifierAvg modifier;
    float result = Integrate(*q, helsinki, modifier);
    float expected = 14.8 / 12;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("average over helsinki failed");
  }

  {
    NFmiDataModifierMax modifier;
    float result = Integrate(*q, suomi, modifier);
    float expected = 5.3;

    if (result != expected)
      TEST_FAILED("maximum over suomi failed");
  }

  {
    NFmiDataModifierMin modifier;
    float result = Integrate(*q, suomi, modifier);
    float expected = -7.7;

    if (result != expected)
      TEST_FAILED("minimum over suomi failed");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Integrate() over time and space
 */
// ----------------------------------------------------------------------

void integrate_time_space(void)
{
  using namespace std;
  using namespace NFmiDataIntegrator;
  using namespace NFmiIndexMaskTools;

  NFmiFastQueryInfo* q = theQD.QueryInfoIter();
  q->First();
  q->Param(kFmiTemperature);
  NFmiMetTime time1 = q->Time();
  NFmiMetTime time2 = time1;
  time2.ChangeByHours(10);

  NFmiIndexMask helsinki = MaskDistance(*theGrid, NFmiPoint(25, 60), 50);
  NFmiIndexMask suomi = MaskInside(*theGrid, theSuomi);

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, helsinki, spacemodifier, time1, time2, timemodifier);

    float expected = 5.12285;

    if (std::abs(result - expected) > 00.1)
      TEST_FAILED("maximum over helsinki and time failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, helsinki, spacemodifier, time1, time2, timemodifier);

    float expected = -0.7;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minimum over helsinki and time failed");
  }

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, suomi, spacemodifier, time1, time2, timemodifier);

    float expected = 6.86388;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("maximum over suomi and time failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, suomi, spacemodifier, time1, time2, timemodifier);

    float expected = -7.7;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minimum over suomi and time failed");
  }

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, helsinki, spacemodifier, time1, time2, timemodifier);

    float expected = 2.6;

    if (std::abs(result - expected) > 00.1)
      TEST_FAILED("maxmin over helsinki and time failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, helsinki, spacemodifier, time1, time2, timemodifier);

    float expected = -0.7;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minmax over helsinki and time failed");
  }

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, suomi, spacemodifier, time1, time2, timemodifier);

    float expected = 5.3;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("maxmin over suomi and time failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, suomi, spacemodifier, time1, time2, timemodifier);

    float expected = -7.7;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minmax over suomi and time failed");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Integrate() over space and time
 */
// ----------------------------------------------------------------------

void integrate_space_time(void)
{
  using namespace std;
  using namespace NFmiDataIntegrator;
  using namespace NFmiIndexMaskTools;

  NFmiFastQueryInfo* q = theQD.QueryInfoIter();
  q->First();
  q->Param(kFmiTemperature);
  NFmiMetTime time1 = q->Time();
  NFmiMetTime time2 = time1;
  time2.ChangeByHours(10);

  NFmiIndexMask helsinki = MaskDistance(*theGrid, NFmiPoint(25, 60), 50);
  NFmiIndexMask suomi = MaskInside(*theGrid, theSuomi);

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, time1, time2, timemodifier, helsinki, spacemodifier);

    float expected = 2.6;

    if (std::abs(result - expected) > 00.1)
      TEST_FAILED("maxmin over time and helsinki failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, time1, time2, timemodifier, helsinki, spacemodifier);

    float expected = 1.71659;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minmax over time and helsinki failed");
  }

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, time1, time2, timemodifier, suomi, spacemodifier);

    float expected = 3.68;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("maxmin over time and suomi failed");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, time1, time2, timemodifier, suomi, spacemodifier);

    float expected = 0.783088;

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minmax over time and suomi failed");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Integrate() over time and space with changing area
 */
// ----------------------------------------------------------------------

void integrate_time_space_masks(void)
{
  using namespace std;
  using namespace NFmiDataIntegrator;
  using namespace NFmiIndexMaskTools;

  NFmiFastQueryInfo* q = theQD.QueryInfoIter();
  q->First();
  q->Param(kFmiTemperature);
  NFmiMetTime time1 = q->Time();
  NFmiMetTime time2 = time1;
  time2.ChangeByHours(1);

  NFmiIndexMask suomi1 = MaskExpand(*theGrid, theSuomi, 0);
  NFmiIndexMask suomi2 = MaskExpand(*theGrid, theSuomi, 100);

  NFmiIndexMaskSource source;
  source.Insert(time1, suomi1);
  source.Insert(time2, suomi2);

  {
    NFmiDataModifierMax spacemodifier;
    NFmiDataModifierMax timemodifier;
    float result = Integrate(*q, source, spacemodifier, time1, time2, timemodifier);

    float expected1 = Integrate(*q, suomi1, spacemodifier, time1, time1, timemodifier);
    float expected2 = Integrate(*q, suomi2, spacemodifier, time2, time2, timemodifier);

    float expected = std::max(expected1, expected2);

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("maximum over suomi and time failed with expanding masks");
  }

  {
    NFmiDataModifierMin spacemodifier;
    NFmiDataModifierMin timemodifier;
    float result = Integrate(*q, source, spacemodifier, time1, time2, timemodifier);

    float expected1 = Integrate(*q, suomi1, spacemodifier, time1, time1, timemodifier);
    float expected2 = Integrate(*q, suomi2, spacemodifier, time2, time2, timemodifier);

    float expected = std::min(expected1, expected2);

    if (std::abs(result - expected) > 0.01)
      TEST_FAILED("minimum over suomi and time failed with expanding masks");
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
    TEST(integrate_time);
    TEST(integrate_space);
    TEST(integrate_time_space);
    TEST(integrate_space_time);
    TEST(integrate_time_space_masks);
  }
};

}  // namespace NFmiDataIntegratorTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiDataIntegrator tester" << endl << "=========================" << endl;

  NFmiDataIntegratorTest::read_querydata("data/hiladata.sqd");
  NFmiDataIntegratorTest::read_svg();

  NFmiDataIntegratorTest::tests t;
  return t.run();
}

// ======================================================================
