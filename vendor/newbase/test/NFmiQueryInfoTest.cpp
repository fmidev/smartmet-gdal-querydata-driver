// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiQueryInfo
 */
// ======================================================================

#include "NFmiProducerName.h"
#include "NFmiQueryData.h"
#include "NFmiQueryInfo.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

// The test data

NFmiQueryData qd;
NFmiQueryInfo qi;

//! Protection against conflicts with global functions
namespace NFmiQueryInfoTest
{
// Note! All other tests depend on the success of this test!

void reading()
{
  string filename = "data/hiladata.sqd";
  ifstream input(filename.c_str());
  if (!input)
    TEST_FAILED("Failed to open " + filename + " for reading");

  input >> qd;

  if (input.bad())
    TEST_FAILED("Input stream " + filename + " was left in bad state");

  qi = NFmiQueryInfo(&qd);

  TEST_PASSED();
}

void constructing()
{
  NFmiQueryInfo q("data/hiladata.sqd");
  NFmiQueryInfo gzipped("data/hiladata.sqd.gz");
  NFmiQueryInfo bzipped("data/hiladata.sqd.bz2");

  TEST_PASSED();
}

void nextparam()
{
  using NFmiStringTools::Convert;

  unsigned long id;

  qi.Reset();
  if ((id = qi.Param().GetParamIdent()) != kFmiBadParameter)
    TEST_FAILED("Param after reset should be BadParameter, not " + Convert(id));

  if (!qi.NextParam())
    TEST_FAILED("Expecting next parameter to be Temperature");

  if ((id = qi.Param().GetParamIdent()) != kFmiTemperature)
    TEST_FAILED("First param should be Temperature, not " + Convert(id));

  if (!qi.NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qi.Param().GetParamIdent()) != kFmiDewPoint)
    TEST_FAILED("Second param should be DewPoint, not " + Convert(id));

  if (!qi.NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qi.Param().GetParamIdent()) != kFmiTotalWindMS)
    TEST_FAILED("Third param should be TotalWindMS, not " + Convert(id));

  if (!qi.NextParam())
    TEST_FAILED("Expecting next parameter to be WeatherAndCloudiness");

  if ((id = qi.Param().GetParamIdent()) != kFmiWeatherAndCloudiness)
    TEST_FAILED("Fourth param should be WeatherAndCloudiness, not " + Convert(id));

  if (qi.NextParam())
    TEST_FAILED("Not expecting more parameters after WeatherAndCloudiness");

  TEST_PASSED();
}

void nexttime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  qi.Reset();

  if ((t = qi.ValidTime()) != NFmiMetTime(2002, 10, 9, 5, 0, 0))
    TEST_FAILED("ValidTime after reset should be 2002 10 9 5 0 0, not " + Convert(t));

  if (!qi.NextTime())
    TEST_FAILED("Expecting more than 1 timestep in test data");

  if ((t = qi.ValidTime()) != NFmiMetTime(2002, 10, 9, 6, 0, 0))
    TEST_FAILED("ValidTime after 1 nexttime should be 2002 10 9 6 0 0, not " + Convert(t));

  if (!qi.NextTime())
    TEST_FAILED("Expecting more than 2 timesteps in test data");

  if ((t = qi.ValidTime()) != NFmiMetTime(2002, 10, 9, 7, 0, 0))
    TEST_FAILED("ValidTime after 2 nexttimes should be 2002 10 9 7 0 0, not " + Convert(t));

  TEST_PASSED();
}

void islocation()
{
  if (qi.IsLocation())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void islevel()
{
  if (!qi.IsLevel())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void isarea()
{
  if (qi.IsArea())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void isgrid()
{
  if (!qi.IsGrid())
    TEST_FAILED("Should return true for gridded data");

  TEST_PASSED();
}

void isvalidtime()
{
  if (!qi.IsValidTime())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void isorigintime()
{
  if (qi.IsOriginTime())
    TEST_FAILED("Should return false for the test data");

  TEST_PASSED();
}

void param()
{
  if (!qi.Param(NFmiDataIdent(NFmiParam(kFmiTemperature), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Temperature parameter on");

  if (!qi.Param(NFmiDataIdent(NFmiParam(kFmiDewPoint), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set DewPoint parameter on");

  if (!qi.Param(NFmiDataIdent(NFmiParam(kFmiTotalWindMS), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set TotalWindMS parameter on");

  if (!qi.Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Precipitation1h parameter on");

  if (qi.Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiSYNOP))))
    TEST_FAILED("Should fail to set Precipitation1h parameter on with wrong producer");

  if (qi.Param(NFmiDataIdent(NFmiParam(kFmiPoP), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Should have failed to set PoP parameter on");

  TEST_PASSED();
}

void level()
{
  using NFmiStringTools::Convert;

  qi.First();
  const NFmiLevel& lev = *qi.Level();

  unsigned long l;

  if ((l = lev.LevelTypeId()) != kFmiAnyLevelType)
    TEST_FAILED("Level typeid should be kFmiAnyLevelType, not " + Convert(l));

  if ((l = lev.LevelValue()) != 0)
    TEST_FAILED("Level value should be 0, not " + Convert(l));

  TEST_PASSED();
}

void time()
{
  using NFmiStringTools::Convert;

  qi.Reset();

  NFmiMetTime t;

  if ((t = qi.Time()) != NFmiMetTime(2002, 10, 9, 5))
    TEST_FAILED("First time should be 9.10.2002 05:00, not " + Convert(t));

  if (!qi.NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qi.Time()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Second time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void validtime()
{
  using NFmiStringTools::Convert;

  qi.Reset();

  NFmiMetTime t;

  if ((t = qi.ValidTime()) != NFmiMetTime(2002, 10, 9, 5))
    TEST_FAILED("First time should be 9.10.2002 05:00, not " + Convert(t));

  if (!qi.NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qi.ValidTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("First time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void origintime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  if ((t = qi.OriginTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Origin time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void forecastperiod()
{
  using NFmiStringTools::Convert;

  unsigned long p;

  if ((p = qi.ForecastPeriod()) != 0)
    TEST_FAILED("Forecast period should be 0, not " + Convert(p));

  TEST_PASSED();
}

void floatvalue()
{
  using NFmiStringTools::Convert;

  qi.First();

  float val;

  val = qi.FloatValue();
  if (abs(val - 5.9) > 1e-5)
    TEST_FAILED("First param first time value should be 5.9, not " + Convert(val));

  qi.NextTime();
  val = qi.FloatValue();
  if (abs(val - 6.42002) > 1e-5)
    TEST_FAILED("First param second time value should be 6.42002, not " + Convert(val));

  qi.NextTime();
  val = qi.FloatValue();
  if (abs(val - 6.64012) > 1e-5)
    TEST_FAILED("First param third time value should be 6.64012, not " + Convert(val));

  qi.First();
  qi.NextParam();

  val = qi.FloatValue();
  if (abs(val - 1.44126) > 1e-5)
    TEST_FAILED("Second param first time value should be 1.44126, not " + Convert(val));

  qi.NextTime();
  val = qi.FloatValue();
  if (abs(val - 2.32046) > 1e-5)
    TEST_FAILED("Second param second time value should be 2.32046, not " + Convert(val));

  qi.NextTime();
  val = qi.FloatValue();
  if (abs(val - 1.56875) > 1e-5)
    TEST_FAILED("Second param third time value should be 1.56875, not " + Convert(val));

  TEST_PASSED();
}

void interpolatedvalue()
{
  using NFmiStringTools::Convert;

  qi.First();
  qi.Param(kFmiTemperature);
  float value = qi.InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 1.80367) > 1e-5)
    TEST_FAILED("Expected t2m 1.80367 for 25,60 at time 1, got " + Convert(value));

  qi.NextTime();
  value = qi.InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 2.27819) > 1e-5)
    TEST_FAILED("Expected t2m 2.27819 for 25,60 at time 2, got " + Convert(value));

  value = qi.InterpolatedValue(NFmiPoint(27, 62));
  if (abs(value - 0.226417) > 1e-5)
    TEST_FAILED("Expected t2m 0.226417 for 27,62 at time 2, got " + Convert(value));

  qi.First();
  qi.Param(kFmiPrecipitation1h);

  value = qi.InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 0) > 1e-5)
    TEST_FAILED("Expected rr1h 0 for 25,60 at time 1, got " + Convert(value));

  qi.NextTime();
  value = qi.InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 0) > 1e-5)
    TEST_FAILED("Expected rr1h 0 for 25,60 at time 2, got " + Convert(value));

  value = qi.InterpolatedValue(NFmiPoint(22, 65));
  if (abs(value - 0.252077) > 1e-5)
    TEST_FAILED("Expected rr1h 0.252077 for 22,65 at time 2, got " + Convert(value));

  {
    // special tests for fixing UV interpolation bug

    NFmiQueryData qdata("data/uv.sqd");
    NFmiQueryInfo qinfo(&qdata);
    qinfo.First();
    qinfo.Param(kFmiUVIndexMax);

    for (int h = 0; h <= 23; h++)
    {
      value = qinfo.InterpolatedValue(NFmiPoint(25, 60), NFmiTime(2009, 4, 21, h, 0), 6 * 60);
      if (h == 12)
      {
        if (abs(value - 3.35) > 1e-5)
          TEST_FAILED("Expected uv 3.35 at 12 UTC, got " + Convert(value));
      }
      else
      {
        if (value != kFloatMissing)
          TEST_FAILED("Expected uv kFloatMissing at " + Convert(h) + " UTC, got " + Convert(value));
      }
    }
  }

  TEST_PASSED();
}
void classid()
{
  using NFmiStringTools::Convert;

  unsigned long id = qi.ClassId();

  if (qi.ClassId() != kNFmiQueryInfo)
    TEST_FAILED("Class id should be kNFmiQueryInfo, not " + Convert(id));

  TEST_PASSED();
}

void infoversion()
{
  using NFmiStringTools::Convert;

  double version = qi.InfoVersion();
  if (version != 6)
    TEST_FAILED("Info Version should be 6, not " + Convert(version));

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
    TEST(reading);
    TEST(constructing);
    TEST(nextparam);
    TEST(nexttime);
    TEST(islocation);
    TEST(islevel);
    TEST(isarea);
    TEST(isgrid);
    TEST(isvalidtime);
    TEST(isorigintime);
    TEST(param);
    TEST(level);
    TEST(time);
    TEST(validtime);
    TEST(origintime);
    TEST(forecastperiod);
    TEST(floatvalue);
    TEST(interpolatedvalue);
    TEST(classid);
    TEST(infoversion);
  }
};

}  // namespace NFmiQueryInfoTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiQueryInfo tester" << endl << "====================" << endl;
  NFmiQueryInfoTest::tests t;
  return t.run();
}

// ======================================================================
