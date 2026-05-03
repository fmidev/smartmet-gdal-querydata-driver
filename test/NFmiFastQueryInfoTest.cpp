// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiFastQueryInfo
 */
// ======================================================================

#include "NFmiFastQueryInfo.h"
#include "NFmiProducerName.h"
#include "NFmiQueryData.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace std;

// The test data

std::unique_ptr<NFmiQueryData> qd;
std::unique_ptr<NFmiFastQueryInfo> qi;

//! Protection against conflicts with global functions
namespace NFmiFastQueryInfoTest
{
// Note! All other tests depend on the success of this test!

void reading()
{
  string filename = "data/hiladata.sqd";
  ifstream input(filename.c_str());
  if (!input)
    TEST_FAILED("Failed to open " + filename + " for reading");

  input >> *qd;

  if (input.bad())
    TEST_FAILED("Input stream " + filename + " was left in bad state");

  qi.reset(new NFmiFastQueryInfo(qd.get()));

  TEST_PASSED();
}

void constructing()
{
  NFmiQueryData qd1("data/hiladata.sqd");
  NFmiQueryData qd2("data/hiladata.sqd.gz");
  NFmiQueryData qd3("data/hiladata.sqd.bz2");

  NFmiFastQueryInfo q(&qd1);
  NFmiFastQueryInfo gzipped(&qd2);
  NFmiFastQueryInfo bzipped(&qd3);

  TEST_PASSED();
}

void reset()
{
  TEST_NOT_IMPLEMENTED();
}
void resetparam()
{
  TEST_NOT_IMPLEMENTED();
}
void resetlocation()
{
  TEST_NOT_IMPLEMENTED();
}
void resetlevel()
{
  TEST_NOT_IMPLEMENTED();
}
void resettime()
{
  TEST_NOT_IMPLEMENTED();
}
void resetheader()
{
  TEST_NOT_IMPLEMENTED();
}
void first()
{
  TEST_NOT_IMPLEMENTED();
}
void firstparam()
{
  TEST_NOT_IMPLEMENTED();
}
void firstlocation()
{
  TEST_NOT_IMPLEMENTED();
}
void firstlevel()
{
  TEST_NOT_IMPLEMENTED();
}
void firsttime()
{
  TEST_NOT_IMPLEMENTED();
}
void firstheader()
{
  TEST_NOT_IMPLEMENTED();
}
void nextparam()
{
  using NFmiStringTools::Convert;

  unsigned long id;

  qi->Reset();

  if ((id = qi->Param().GetParamIdent()) != kFmiTemperature)
    TEST_FAILED("Param after reset should be Temperature, not " + Convert(id));

  if (!qi->NextParam())
    TEST_FAILED("Expecting next parameter to be Temperature");

  if ((id = qi->Param().GetParamIdent()) != kFmiTemperature)
    TEST_FAILED("First param should be Temperature, not " + Convert(id));

  if (!qi->NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qi->Param().GetParamIdent()) != kFmiDewPoint)
    TEST_FAILED("Second param should be DewPoint, not " + Convert(id));

  if (!qi->NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qi->Param().GetParamIdent()) != kFmiTotalWindMS)
    TEST_FAILED("Third param should be TotalWindMS, not " + Convert(id));

  if (!qi->NextParam())
    TEST_FAILED("Expecting next parameter to be WeatherAndCloudiness");

  if ((id = qi->Param().GetParamIdent()) != kFmiWeatherAndCloudiness)
    TEST_FAILED("Fourth param should be WeatherAndCloudiness, not " + Convert(id));

  if (qi->NextParam())
    TEST_FAILED("Not expecting more parameters after WeatherAndCloudiness");

  TEST_PASSED();
}

void nextlocation()
{
  TEST_NOT_IMPLEMENTED();
}
void nextlevel()
{
  TEST_NOT_IMPLEMENTED();
}
void nexttime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  qi->Reset();

  if ((t = qi->ValidTime()) != NFmiMetTime(1900, 1, 1, 1, 0, 0))
    TEST_FAILED("ValidTime after reset should be 1900 1 1 1 0 0, not " + Convert(t));

  if (!qi->NextTime())
    TEST_FAILED("Expecting more than 1 timestep in test data");

  if ((t = qi->ValidTime()) != NFmiMetTime(2002, 10, 9, 6, 0, 0))
    TEST_FAILED("ValidTime after 1 nexttime should be 2002 10 9 6 0 0, not " + Convert(t));

  if (!qi->NextTime())
    TEST_FAILED("Expecting more than 2 timesteps in test data");

  if ((t = qi->ValidTime()) != NFmiMetTime(2002, 10, 9, 7, 0, 0))
    TEST_FAILED("ValidTime after 2 nexttimes should be 2002 10 9 7 0 0, not " + Convert(t));

  TEST_PASSED();
}

void nextheader()
{
  TEST_NOT_IMPLEMENTED();
}
void previousparam()
{
  TEST_NOT_IMPLEMENTED();
}
void previoustime()
{
  TEST_NOT_IMPLEMENTED();
}
void previouslocation()
{
  TEST_NOT_IMPLEMENTED();
}
void previouslevel()
{
  TEST_NOT_IMPLEMENTED();
}
void nextactiveparam()
{
  TEST_NOT_IMPLEMENTED();
}
void nextactivelocation()
{
  TEST_NOT_IMPLEMENTED();
}
void nextactivelevel()
{
  TEST_NOT_IMPLEMENTED();
}
void nextactivetime()
{
  TEST_NOT_IMPLEMENTED();
}
void previousactivelevel()
{
  TEST_NOT_IMPLEMENTED();
}
void previousactivetime()
{
  TEST_NOT_IMPLEMENTED();
}
void isactiveparam()
{
  TEST_NOT_IMPLEMENTED();
}
void activateparam()
{
  TEST_NOT_IMPLEMENTED();
}
void activatelocation()
{
  TEST_NOT_IMPLEMENTED();
}
void activatelevel()
{
  TEST_NOT_IMPLEMENTED();
}
void activatetime()
{
  TEST_NOT_IMPLEMENTED();
}
void activatetimeperiod()
{
  TEST_NOT_IMPLEMENTED();
}
void getactivetimeperiod()
{
  TEST_NOT_IMPLEMENTED();
}
void islocation()
{
  if (qi->IsLocation())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void islevel()
{
  if (!qi->IsLevel())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void isarea()
{
  if (qi->IsArea())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void isgrid()
{
  if (!qi->IsGrid())
    TEST_FAILED("Should return true for gridded data");

  TEST_PASSED();
}

void isvalidtime()
{
  if (!qi->IsValidTime())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void isorigintime()
{
  if (qi->IsOriginTime())
    TEST_FAILED("Should return false for the test data");

  TEST_PASSED();
}

void isheader()
{
  TEST_NOT_IMPLEMENTED();
}
void param()
{
  if (!qi->Param(NFmiDataIdent(NFmiParam(kFmiTemperature), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Temperature parameter on");

  if (!qi->Param(NFmiDataIdent(NFmiParam(kFmiDewPoint), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set DewPoint parameter on");

  if (!qi->Param(NFmiDataIdent(NFmiParam(kFmiTotalWindMS), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set TotalWindMS parameter on");

  if (!qi->Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Precipitation1h parameter on");

  if (qi->Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiSYNOP))))
    TEST_FAILED("Should fail to set Precipitation1h parameter on with wrong producer");

  if (qi->Param(NFmiDataIdent(NFmiParam(kFmiPoP), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Should have failed to set PoP parameter on");

  TEST_PASSED();
}

void location()
{
  TEST_NOT_IMPLEMENTED();
}
void usestaticmask()
{
  TEST_NOT_IMPLEMENTED();
}
void staticdatamask()
{
  TEST_NOT_IMPLEMENTED();
}
void validtimewindow()
{
  TEST_NOT_IMPLEMENTED();
}
void nearestlocation()
{
  TEST_NOT_IMPLEMENTED();
}
void nearestpoint()
{
  TEST_NOT_IMPLEMENTED();
}
void nearestlocations()
{
  TEST_NOT_IMPLEMENTED();
}
void level()
{
  using NFmiStringTools::Convert;

  qi->First();
  const NFmiLevel& lev = *qi->Level();

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

  qi->Reset();

  NFmiMetTime t;

  if ((t = qi->Time()) != NFmiMetTime(1900, 1, 1, 1))
    TEST_FAILED("First time should be 1.1.1900 01:00, not " + Convert(t));

  if (!qi->NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qi->Time()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Second time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void timetoneareststep()
{
  TEST_NOT_IMPLEMENTED();
}
void area()
{
  TEST_NOT_IMPLEMENTED();
}
void grid()
{
  TEST_NOT_IMPLEMENTED();
}
void latlon()
{
  TEST_NOT_IMPLEMENTED();
}
void relativepoint()
{
  TEST_NOT_IMPLEMENTED();
}
void producer()
{
  TEST_NOT_IMPLEMENTED();
}
void setproducer()
{
  TEST_NOT_IMPLEMENTED();
}
void validtime()
{
  using NFmiStringTools::Convert;

  qi->Reset();

  NFmiMetTime t;

  if ((t = qi->ValidTime()) != NFmiMetTime(1900, 1, 1, 1))
    TEST_FAILED("First time should be 1.1.1900 01:00, not " + Convert(t));

  if (!qi->NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qi->ValidTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("First time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void origintime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  if ((t = qi->OriginTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Origin time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void validtimes()
{
  TEST_NOT_IMPLEMENTED();
}
void parambag()
{
  TEST_NOT_IMPLEMENTED();
}
void hplacedescriptor()
{
  TEST_NOT_IMPLEMENTED();
}
void vplacedescriptor()
{
  TEST_NOT_IMPLEMENTED();
}
void timedescriptor()
{
  TEST_NOT_IMPLEMENTED();
}
void paramdescriptor()
{
  TEST_NOT_IMPLEMENTED();
}
void setlocaltimes()
{
  TEST_NOT_IMPLEMENTED();
}
void islocaltime()
{
  TEST_NOT_IMPLEMENTED();
}
void forecastperiod()
{
  using NFmiStringTools::Convert;

  unsigned long p;

  if ((p = qi->ForecastPeriod()) != 0)
    TEST_FAILED("Forecast period should be 0, not " + Convert(p));

  TEST_PASSED();
}

void owner()
{
  TEST_NOT_IMPLEMENTED();
}
void timestamp()
{
  TEST_NOT_IMPLEMENTED();
}
void leader()
{
  TEST_NOT_IMPLEMENTED();
}
void header()
{
  TEST_NOT_IMPLEMENTED();
}
void resettext()
{
  TEST_NOT_IMPLEMENTED();
}
void nexttext()
{
  TEST_NOT_IMPLEMENTED();
}
void text()
{
  TEST_NOT_IMPLEMENTED();
}
void resetpostproc()
{
  TEST_NOT_IMPLEMENTED();
}
void nextpostproc()
{
  TEST_NOT_IMPLEMENTED();
}
void postproc()
{
  TEST_NOT_IMPLEMENTED();
}
void size()
{
  TEST_NOT_IMPLEMENTED();
}
void sizeparams()
{
  TEST_NOT_IMPLEMENTED();
}
void sizelocations()
{
  TEST_NOT_IMPLEMENTED();
}
void sizelevels()
{
  TEST_NOT_IMPLEMENTED();
}
void sizetimes()
{
  TEST_NOT_IMPLEMENTED();
}
void sizeactiveparams()
{
  TEST_NOT_IMPLEMENTED();
}
void sizeactivelocations()
{
  TEST_NOT_IMPLEMENTED();
}
void sizeactivelevels()
{
  TEST_NOT_IMPLEMENTED();
}
void sizeactivetimes()
{
  TEST_NOT_IMPLEMENTED();
}
void floatvalue()
{
  using NFmiStringTools::Convert;

  qi->First();

  float val;

  val = qi->FloatValue();
  if (abs(val - 5.9) > 1e-5)
    TEST_FAILED("First param first time value should be 5.9, not " + Convert(val));

  qi->NextTime();
  val = qi->FloatValue();
  if (abs(val - 6.42002) > 1e-5)
    TEST_FAILED("First param second time value should be 6.42002, not " + Convert(val));

  qi->NextTime();
  val = qi->FloatValue();
  if (abs(val - 6.64012) > 1e-5)
    TEST_FAILED("First param third time value should be 6.64012, not " + Convert(val));

  qi->First();
  qi->NextParam();

  val = qi->FloatValue();
  if (abs(val - 1.44126) > 1e-5)
    TEST_FAILED("Second param first time value should be 1.44126, not " + Convert(val));

  qi->NextTime();
  val = qi->FloatValue();
  if (abs(val - 2.32046) > 1e-5)
    TEST_FAILED("Second param second time value should be 2.32046, not " + Convert(val));

  qi->NextTime();
  val = qi->FloatValue();
  if (abs(val - 1.56875) > 1e-5)
    TEST_FAILED("Second param third time value should be 1.56875, not " + Convert(val));

  TEST_PASSED();
}

void interpolatedvalue()
{
  using NFmiStringTools::Convert;

  qi->First();
  qi->Param(kFmiTemperature);
  float value = qi->InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 1.80367) > 1e-5)
    TEST_FAILED("Expected t2m 1.80367 for 25,60 at time 1, got " + Convert(value));

  qi->NextTime();
  value = qi->InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 2.27819) > 1e-5)
    TEST_FAILED("Expected t2m 2.27819 for 25,60 at time 2, got " + Convert(value));

  value = qi->InterpolatedValue(NFmiPoint(27, 62));
  if (abs(value - 0.226417) > 1e-5)
    TEST_FAILED("Expected t2m 0.226417 for 27,62 at time 2, got " + Convert(value));

  qi->First();
  qi->Param(kFmiPrecipitation1h);

  value = qi->InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 0) > 1e-5)
    TEST_FAILED("Expected rr1h 0 for 25,60 at time 1, got " + Convert(value));

  qi->NextTime();
  value = qi->InterpolatedValue(NFmiPoint(25, 60));
  if (abs(value - 0) > 1e-5)
    TEST_FAILED("Expected rr1h 0 for 25,60 at time 2, got " + Convert(value));

  value = qi->InterpolatedValue(NFmiPoint(22, 65));
  if (abs(value - 0.252077) > 1e-5)
    TEST_FAILED("Expected rr1h 0.252077 for 22,65 at time 2, got " + Convert(value));

  {
    // special tests for fixing UV interpolation bug

    NFmiQueryData qdata("data/uv.sqd");
    NFmiFastQueryInfo qinfo(&qdata);
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

void peeklocationvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void peeklocationlatlon()
{
  TEST_NOT_IMPLEMENTED();
}
void peektimevalue()
{
  TEST_NOT_IMPLEMENTED();
}
void peekvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void gridvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void heightvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void pressurelevelvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void findnearesttime()
{
  TEST_NOT_IMPLEMENTED();
}
void classid()
{
  using NFmiStringTools::Convert;

  unsigned long id = qi->ClassId();

  if (qi->ClassId() != kNFmiQueryInfo)
    TEST_FAILED("Class id should be kNFmiFastQueryInfo, not " + Convert(id));

  TEST_PASSED();
}

void classname()
{
  TEST_NOT_IMPLEMENTED();
}
void refdatapool()
{
  TEST_NOT_IMPLEMENTED();
}
void refquerydata()
{
  TEST_NOT_IMPLEMENTED();
}
void usesubparam()
{
  TEST_NOT_IMPLEMENTED();
}
void timeperiodfloatvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void interpolatedtimeperiodfloatvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void calcleveldata()
{
  TEST_NOT_IMPLEMENTED();
}
void calclocationdata()
{
  TEST_NOT_IMPLEMENTED();
}
void calclocationdatawithextremeplace()
{
  TEST_NOT_IMPLEMENTED();
}
void calctimedata()
{
  TEST_NOT_IMPLEMENTED();
}
void calctimedatawithextremetime()
{
  TEST_NOT_IMPLEMENTED();
}
void calinterpolatedtimedata()
{
  TEST_NOT_IMPLEMENTED();
}
void modifylocationdata()
{
  TEST_NOT_IMPLEMENTED();
}
void modifyleveldata()
{
  TEST_NOT_IMPLEMENTED();
}
void modifytimeslocationdata()
{
  TEST_NOT_IMPLEMENTED();
}
void timeresolution()
{
  TEST_NOT_IMPLEMENTED();
}
void paramindex()
{
  TEST_NOT_IMPLEMENTED();
}
void locationindex()
{
  TEST_NOT_IMPLEMENTED();
}
void levelindex()
{
  TEST_NOT_IMPLEMENTED();
}
void timeindex()
{
  TEST_NOT_IMPLEMENTED();
}
void calctimeuncertainty()
{
  TEST_NOT_IMPLEMENTED();
}
void calcareauncertainy()
{
  TEST_NOT_IMPLEMENTED();
}
void timeuncertaintystart()
{
  TEST_NOT_IMPLEMENTED();
}
void timeuncertaintyend()
{
  TEST_NOT_IMPLEMENTED();
}
void areauncertaintystart()
{
  TEST_NOT_IMPLEMENTED();
}
void areauncertaintyend()
{
  TEST_NOT_IMPLEMENTED();
}
void top()
{
  TEST_NOT_IMPLEMENTED();
}
void bottom()
{
  TEST_NOT_IMPLEMENTED();
}
void left()
{
  TEST_NOT_IMPLEMENTED();
}
void right()
{
  TEST_NOT_IMPLEMENTED();
}
void findfirstkey()
{
  TEST_NOT_IMPLEMENTED();
}
void findnextkey()
{
  TEST_NOT_IMPLEMENTED();
}
void addkey()
{
  TEST_NOT_IMPLEMENTED();
}
void getallkeys()
{
  TEST_NOT_IMPLEMENTED();
}
void getcurrentkey()
{
  TEST_NOT_IMPLEMENTED();
}
void setcurrentkeyvalue()
{
  TEST_NOT_IMPLEMENTED();
}
void removecurrentkey()
{
  TEST_NOT_IMPLEMENTED();
}
void removeallkeys()
{
  TEST_NOT_IMPLEMENTED();
}
void doendianbyteswap()
{
  TEST_NOT_IMPLEMENTED();
}
void infoversion()
{
  using NFmiStringTools::Convert;

  double version = qi->InfoVersion();
  if (version != 6)
    TEST_FAILED("Info Version should be 6, not " + Convert(version));

  TEST_PASSED();
}

void masktype()
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
 public:
  tests()
  {
    qd.reset(new NFmiQueryData);
    qi.reset(new NFmiFastQueryInfo);
  }

  virtual ~tests()
  {
    qd.reset();
    qi.reset();
  }

 private:
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

}  // namespace NFmiFastQueryInfoTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiFastQueryInfo tester" << endl << "====================" << endl;
  NFmiFastQueryInfoTest::tests t;
  return t.run();
}

// ======================================================================
