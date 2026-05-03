// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiQueryData
 */
// ======================================================================

#include "NFmiProducerName.h"
#include "NFmiQueryData.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

NFmiQueryData* qd;

//! Protection against conflicts with global functions
namespace NFmiQueryDataTest
{
// Note! All but 1 test depends on the success of this test!
void reading()
{
  string filename = "data/hiladata.sqd";
  ifstream input(filename.c_str());
  if (!input)
    TEST_FAILED("Failed to open " + filename + " for reading");

  NFmiQueryData q;
  input >> q;

  if (input.bad())
    TEST_FAILED("Input stream " + filename + " was left in bad state");

  TEST_PASSED();
}

void constructing()
{
  qd = new NFmiQueryData("data/hiladata.sqd");
  if (qd == 0)
    TEST_FAILED("Failed to create mmapped file with new");

  NFmiQueryData dir("data/");
  NFmiQueryData gzipped("data/hiladata.sqd.gz");
  NFmiQueryData bzipped("data/hiladata.sqd.bz2");

  NFmiQueryData copy(*qd);

  TEST_PASSED();
}

void infoversion()
{
  using NFmiStringTools::Convert;

  double version = qd->InfoVersion();
  if (version != 6)
    TEST_FAILED("Info Version should be 6, not " + Convert(version));

  TEST_PASSED();
}

void islocation()
{
  if (qd->IsLocation())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void isarea()
{
  if (qd->IsArea())
    TEST_FAILED("Should return false for gridded data");

  TEST_PASSED();
}

void isgrid()
{
  if (!qd->IsGrid())
    TEST_FAILED("Should return true for gridded data");

  TEST_PASSED();
}

void isvalidtime()
{
  if (!qd->IsValidTime())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void isorigintime()
{
  if (qd->IsOriginTime())
    TEST_FAILED("Should return false for the test data");

  TEST_PASSED();
}

void islevel()
{
  if (!qd->IsLevel())
    TEST_FAILED("Should return true for the test data");

  TEST_PASSED();
}

void nextparam()
{
  using NFmiStringTools::Convert;

  unsigned long id;

  qd->Reset();
  if ((id = qd->Param().GetParamIdent()) != kFmiBadParameter)
    TEST_FAILED("Param after reset should be BadParameter, not " + Convert(id));

  if (!qd->NextParam())
    TEST_FAILED("Expecting next parameter to be Temperature");

  if ((id = qd->Param().GetParamIdent()) != kFmiTemperature)
    TEST_FAILED("First param should be Temperature, not " + Convert(id));

  if (!qd->NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qd->Param().GetParamIdent()) != kFmiDewPoint)
    TEST_FAILED("Second param should be DewPoint, not " + Convert(id));

  if (!qd->NextParam())
    TEST_FAILED("Expecting next parameter to be DewPoint");

  if ((id = qd->Param().GetParamIdent()) != kFmiTotalWindMS)
    TEST_FAILED("Third param should be TotalWindMS, not " + Convert(id));

  if (!qd->NextParam())
    TEST_FAILED("Expecting next parameter to be WeatherAndCloudiness");

  if ((id = qd->Param().GetParamIdent()) != kFmiWeatherAndCloudiness)
    TEST_FAILED("Fourth param should be WeatherAndCloudiness, not " + Convert(id));

  if (qd->NextParam())
    TEST_FAILED("Not expecting more parameters after WeatherAndCloudiness");

  TEST_PASSED();
}

void nexttime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  qd->Reset();

  if ((t = qd->ValidTime()) != NFmiMetTime(2002, 10, 9, 5, 0, 0))
    TEST_FAILED("ValidTime after reset should be 2002 10 9 5 0 0, not " + Convert(t));

  if (!qd->NextTime())
    TEST_FAILED("Expecting more than 1 timestep in test data");

  if ((t = qd->ValidTime()) != NFmiMetTime(2002, 10, 9, 6, 0, 0))
    TEST_FAILED("ValidTime after 1 nexttime should be 2002 10 9 6 0 0, not " + Convert(t));

  if (!qd->NextTime())
    TEST_FAILED("Expecting more than 2 timesteps in test data");

  if ((t = qd->ValidTime()) != NFmiMetTime(2002, 10, 9, 7, 0, 0))
    TEST_FAILED("ValidTime after 2 nexttimes should be 2002 10 9 7 0 0, not " + Convert(t));

  TEST_PASSED();
}

void traversedata()
{
  float sum = 0;
  for (qd->ResetLevel(); qd->NextLevel();)
    for (qd->ResetParam(); qd->NextParam();)
      for (qd->ResetTime(); qd->NextTime();)
        for (qd->ResetLocation(); qd->NextLocation();)
          sum += qd->FloatValue();

  if (abs(sum + 2.42812e+34) / 2.42812e+34 > 1e-4)
    TEST_FAILED("Sum of all values should be -2.42812e+34, not " + NFmiStringTools::Convert(sum));

  TEST_PASSED();
}

void param()
{
  if (!qd->Param(NFmiDataIdent(NFmiParam(kFmiTemperature), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Temperature parameter on");

  if (!qd->Param(NFmiDataIdent(NFmiParam(kFmiDewPoint), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set DewPoint parameter on");

  if (!qd->Param(NFmiDataIdent(NFmiParam(kFmiTotalWindMS), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set TotalWindMS parameter on");

  if (!qd->Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Failed to set Precipitation1h parameter on");

  if (qd->Param(NFmiDataIdent(NFmiParam(kFmiPrecipitation1h), NFmiProducer(kFmiSYNOP))))
    TEST_FAILED("Should fail to set Precipitation1h parameter on with wrong producer");

  if (qd->Param(NFmiDataIdent(NFmiParam(kFmiPoP), NFmiProducer(kFmiMETEOR))))
    TEST_FAILED("Should have failed to set PoP parameter on");

  TEST_PASSED();
}

void time()
{
  using NFmiStringTools::Convert;

  qd->Reset();

  NFmiMetTime t;

  if ((t = qd->Time()) != NFmiMetTime(2002, 10, 9, 5))
    TEST_FAILED("First time should be 9.10.2002 05:00, not " + Convert(t));

  if (!qd->NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qd->Time()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Second time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void validtime()
{
  using NFmiStringTools::Convert;

  qd->Reset();

  NFmiMetTime t;

  if ((t = qd->ValidTime()) != NFmiMetTime(2002, 10, 9, 5))
    TEST_FAILED("First time should be 9.10.2002 05:00, not " + Convert(t));

  if (!qd->NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qd->ValidTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("First time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void origintime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  if ((t = qd->OriginTime()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Origin time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void forecastperiod()
{
  using NFmiStringTools::Convert;

  unsigned long p;

  if ((p = qd->ForecastPeriod()) != 0)
    TEST_FAILED("Forecast period should be 0, not " + Convert(p));

  TEST_PASSED();
}

void level()
{
  using NFmiStringTools::Convert;

  qd->First();
  NFmiLevel lev = qd->Level();

  unsigned long l;

  if ((l = lev.LevelTypeId()) != kFmiAnyLevelType)
    TEST_FAILED("Level typeid should be kFmiAnyLevelType, not " + Convert(l));

  if ((l = lev.LevelValue()) != 0)
    TEST_FAILED("Level value should be 0, not " + Convert(l));

  TEST_PASSED();
}

void paramsize()
{
  using NFmiStringTools::Convert;

  unsigned long n = qd->ParamSize();

  if (n != 4)
    TEST_FAILED("Param size should be 4, not " + Convert(n));

  TEST_PASSED();
}

void quality()
{
  using NFmiStringTools::Convert;

  float q = qd->Quality();
  if (q != 100)
    TEST_FAILED("Quality should be 100, not " + Convert(q));

  TEST_PASSED();
}

void floatvalue()
{
  using NFmiStringTools::Convert;

  qd->First();

  float val;

  val = qd->FloatValue();
  if (abs(val - 5.9) > 1e-5)
    TEST_FAILED("First param first time value should be 5.9, not " + Convert(val));

  qd->NextTime();
  val = qd->FloatValue();
  if (abs(val - 6.42002) > 1e-5)
    TEST_FAILED("First param second time value should be 6.42002, not " + Convert(val));

  qd->NextTime();
  val = qd->FloatValue();
  if (abs(val - 6.64012) > 1e-5)
    TEST_FAILED("First param third time value should be 6.64012, not " + Convert(val));

  qd->First();
  qd->NextParam();

  val = qd->FloatValue();
  if (abs(val - 1.44126) > 1e-5)
    TEST_FAILED("Second param first time value should be 1.44126, not " + Convert(val));

  qd->NextTime();
  val = qd->FloatValue();
  if (abs(val - 2.32046) > 1e-5)
    TEST_FAILED("Second param second time value should be 2.32046, not " + Convert(val));

  qd->NextTime();
  val = qd->FloatValue();
  if (abs(val - 1.56875) > 1e-5)
    TEST_FAILED("Second param third time value should be 1.56875, not " + Convert(val));

  if (!qd->Time(NFmiTime(2002, 10, 12, 6)))
    TEST_FAILED("Failed to set last time on");

  val = qd->FloatValue();
  if (abs(val - 2.91922) > 1e-5)
    TEST_FAILED("Second param last time value should be 2.91922, not " + Convert(val));

  TEST_PASSED();
}

void isbinarystorageused()
{
  if (!qd->IsBinaryStorageUsed())
    TEST_FAILED("Binary storage should be on for the test data");

  TEST_PASSED();
}

void usebinarystorage()
{
  bool oldstate = qd->IsBinaryStorageUsed();

  qd->UseBinaryStorage(true);
  if (!qd->IsBinaryStorageUsed())
    TEST_FAILED("Setting binary storage on failed");

  qd->UseBinaryStorage(false);
  if (qd->IsBinaryStorageUsed())
    TEST_FAILED("Setting binary storage off failed");

  qd->UseBinaryStorage(oldstate);

  if (qd->IsBinaryStorageUsed() != oldstate)
    TEST_FAILED("Restoring old state failed");

  TEST_PASSED();
}

void classid()
{
  using NFmiStringTools::Convert;

  unsigned long id = qd->ClassId();

  if (qd->ClassId() != kNFmiQueryData)
    TEST_FAILED("Class id should be kNFmiQueryData, not " + Convert(id));

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

    TEST(traversedata);

    TEST(infoversion);
    TEST(islocation);
    TEST(isarea);
    TEST(isgrid);
    TEST(isvalidtime);
    TEST(isorigintime);
    TEST(islevel);
    TEST(nextparam);
    TEST(nexttime);
    TEST(param);
    TEST(time);
    TEST(validtime);
    TEST(origintime);
    TEST(forecastperiod);
    TEST(level);
    TEST(paramsize);
    TEST(quality);
    TEST(floatvalue);
    TEST(usebinarystorage);
    TEST(isbinarystorageused);
    TEST(classid);
  }
};

}  // namespace NFmiQueryDataTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiQueryData tester" << endl << "====================" << endl;
  NFmiQueryDataTest::tests t;
  return t.run();
}

// ======================================================================
