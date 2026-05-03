// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiQueryInfo
 */
// ======================================================================

#include "NFmiMultiQueryInfo.h"
#include "NFmiProducerName.h"
#include "NFmiQueryData.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <stdexcept>
#include <string>

#include <memory>

using namespace std;

// The test data

NFmiMultiQueryInfo* qi;

//! Protection against conflicts with global functions
namespace NFmiMultiQueryInfoTest
{
// Note! All other tests depend on the success of this test!

void reading()
{
  qi = new NFmiMultiQueryInfo("data/hiladata.sqd");

  TEST_PASSED();
}

void constructing()
{
  std::vector<std::shared_ptr<NFmiQueryData> > qdv;
  std::vector<std::shared_ptr<NFmiQueryData> > qdv_gzipped;
  std::vector<std::shared_ptr<NFmiQueryData> > qdv_bzipped;
  NFmiMultiQueryInfo qd("data/hiladata.sqd");
  NFmiMultiQueryInfo qd_gzipped("data/hiladata.sqd.gz");
  NFmiMultiQueryInfo qd_bzipped("data/hiladata.sqd.bz2");

  TEST_PASSED();
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

void nexttime()
{
  using NFmiStringTools::Convert;

  NFmiMetTime t;

  qi->Reset();

  try
  {
    t = qi->ValidTime();
    TEST_FAILED("Should not get ValidTime after Reset");
  }
  catch (...)
  {
  }

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

  if (!qi->NextTime())
    TEST_FAILED("Data should have more than 1 timestep");

  if ((t = qi->Time()) != NFmiMetTime(2002, 10, 9, 6))
    TEST_FAILED("Second time should be 9.10.2002 06:00, not " + Convert(t));

  TEST_PASSED();
}

void validtime()
{
  using NFmiStringTools::Convert;

  qi->Reset();

  NFmiMetTime t;

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

void forecastperiod()
{
  using NFmiStringTools::Convert;

  unsigned long p;

  if ((p = qi->ForecastPeriod()) != 0)
    TEST_FAILED("Forecast period should be 0, not " + Convert(p));

  TEST_PASSED();
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

    NFmiMultiQueryInfo qinfo("data/uv.sqd");

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

  unsigned long id = qi->ClassId();

  if (qi->ClassId() != kNFmiQueryInfo)
    TEST_FAILED("Class id should be kNFmiQueryInfo, not " + Convert(id));

  TEST_PASSED();
}

void infoversion()
{
  using NFmiStringTools::Convert;

  double version = qi->InfoVersion();
  if (version != 6)
    TEST_FAILED("Info Version should be 6, not " + Convert(version));

  TEST_PASSED();
}

void multifile_non_overlapping()
{
  using NFmiStringTools::Convert;

  std::list<std::string> files;
  files.push_back("data/201309110600_tutka_suomi_rr.sqd");
  files.push_back("data/201309110605_tutka_suomi_rr.sqd");
  files.push_back("data/201309110610_tutka_suomi_rr.sqd");
  files.push_back("data/201309110615_tutka_suomi_rr.sqd");

  NFmiMultiQueryInfo mqi(files);
  mqi.ResetTime();
  mqi.Param(kFmiPrecipitationRate);
  mqi.Location(NFmiLocation(25.7333, 60.8083));

  vector<float> result_vector;

  // testing NextTime()
  while (mqi.NextTime())
  {
    result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  }
  if (result_vector.size() != 4)
    TEST_FAILED("Not enough result values: expected 4, got " + Convert(result_vector.size()));

  if (Convert(result_vector[0]) != "0.71" || Convert(result_vector[1]) != "0.27" ||
      Convert(result_vector[2]) != "0" || Convert(result_vector[3]) != "0")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    TEST_FAILED("Expected PrecipitationRate 0.71, 0.27, 0, 0, got " + result_str);
  }
  // testing ResetTime()
  result_vector.clear();
  mqi.ResetTime();
  while (mqi.NextTime())
  {
    result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  }
  if (result_vector.size() != 4)
    TEST_FAILED("Not enough result values after ResetTime(): expected 4, got " +
                Convert(result_vector.size()));
  if (Convert(result_vector[0]) != "0.71" || Convert(result_vector[1]) != "0.27" ||
      Convert(result_vector[2]) != "0" || Convert(result_vector[3]) != "0")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    TEST_FAILED("Expected PrecipitationRate 0.71, 0.27, 0, 0, got " + result_str);
  }
  // testing LastTime()
  mqi.ResetTime();
  mqi.LastTime();
  float value = mqi.InterpolatedValue(mqi.ValidTime(), 0);
  if (Convert(value) != "0")
    TEST_FAILED("Expected PrecipitationRate 0 got " + Convert(value));

  // testing FirstTime()
  mqi.ResetTime();
  mqi.FirstTime();
  value = mqi.InterpolatedValue(mqi.ValidTime(), 0);
  if (Convert(value) != "0.71")
    TEST_FAILED("Expected PrecipitationRate 0.71 got " + Convert(value));

  NFmiTime tim;
  // testing PreviousTime()
  result_vector.clear();
  mqi.LastTime();
  result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  while (mqi.PreviousTime())
  {
    if (result_vector.size() == 2)
      tim = mqi.ValidTime();
    result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  }
  if (Convert(result_vector[0]) != "0" || Convert(result_vector[1]) != "0" ||
      Convert(result_vector[2]) != "0.27" || Convert(result_vector[3]) != "0.71")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    TEST_FAILED("Expected PrecipitationRate 0, 0, 0.27, 0.71, got " + result_str);
  }

  /*
  mqi.FirstTime();
  NFmiMetTime mytime(mqi.ValidTime());
  mytime.ChangeBySeconds(120);

  NFmiDataMatrix<NFmiLocationCache> locationCache;

  mqi.CalcLatlonCachePoints(mqi, locationCache);

  NFmiTimeCache timeCache = mqi.CalcTimeCache(mytime);

  NFmiGrid grid = *mqi.Grid();
  unsigned long xs = grid.XNumber();


  for( grid.Reset(); grid.Next(); )
    {
          NFmiLocationCache & lc = locationCache[grid.Index() % xs][grid.Index() / xs];
          mqi.CachedInterpolation(lc, timeCache);
    }
*/
  /*
  std::cout << "ANSSI0: " << tim << endl;
  std::cout << "ANSSI1: " << mqi.ValidTime() << endl;
  std::cout << "ANSSI2: " << mqi.Time() << endl;
  std::cout << "ANSSI3: " << mqi.OriginTime() << endl;
  */

  TEST_PASSED();
}

void multifile_overlapping()
{
  using NFmiStringTools::Convert;

  std::list<std::string> files;
  files.push_back("data/201309120708_pal_skandinavia_maanpinta.sqd");
  files.push_back("data/201309111610_pal_skandinavia_maanpinta.sqd");

  NFmiMultiQueryInfo mqi(files);
  mqi.ResetTime();
  mqi.Param(kFmiGroundTemperature);
  mqi.Location(NFmiLocation(24.9375, 60.1708));
  vector<float> result_vector;

  // testing NextTime()
  while (mqi.NextTime())
  {
    float value = mqi.InterpolatedValue(mqi.ValidTime(), 0);
    result_vector.push_back(value);
  }
  if (result_vector.size() != 10)
    TEST_FAILED("Not enough result values: expected 10, got " + Convert(result_vector.size()));
  if (Convert(result_vector[0]) != "8.51755" || Convert(result_vector[1]) != "7.63031" ||
      Convert(result_vector[2]) != "9.33667" || Convert(result_vector[3]) != "10.4724" ||
      Convert(result_vector[4]) != "12.3766" || Convert(result_vector[5]) != "9.70439" ||
      Convert(result_vector[6]) != "9.00446" || Convert(result_vector[7]) != "9.86303" ||
      Convert(result_vector[8]) != "11.807" || Convert(result_vector[9]) != "12.9554")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[4]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[5]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[6]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[7]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[8]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[9]));
    TEST_FAILED(
        "Expected GroudTemperature 8.51755, 7.63031, 9.33667, 10.4724, 12.3766, 9.70439, 9.00446, "
        "9.86303, 11.807, 12.9554 got " +
        result_str);
  }

  // testing ResetTime()
  result_vector.clear();
  mqi.ResetTime();
  while (mqi.NextTime())
  {
    result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  }
  if (result_vector.size() != 10)
    TEST_FAILED("Not enough result values: expected 10, got " + Convert(result_vector.size()));
  if (Convert(result_vector[0]) != "8.51755" || Convert(result_vector[1]) != "7.63031" ||
      Convert(result_vector[2]) != "9.33667" || Convert(result_vector[3]) != "10.4724" ||
      Convert(result_vector[4]) != "12.3766" || Convert(result_vector[5]) != "9.70439" ||
      Convert(result_vector[6]) != "9.00446" || Convert(result_vector[7]) != "9.86303" ||
      Convert(result_vector[8]) != "11.807" || Convert(result_vector[9]) != "12.9554")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[4]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[5]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[6]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[7]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[8]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[9]));
    TEST_FAILED(
        "Expected GroudTemperature 8.51755, 7.63031, 9.33667, 10.4724, 12.3766, 9.70439, 9.00446, "
        "9.86303, 11.807, 12.9554 got " +
        result_str);
  }

  // testing LastTime()
  mqi.ResetTime();
  mqi.LastTime();
  float value = mqi.InterpolatedValue(mqi.ValidTime(), 0);
  if (Convert(value) != "12.9554")
    TEST_FAILED("Expected PrecipitationRate 12.9554 got " + Convert(value));

  // testing FirstTime()
  mqi.ResetTime();
  mqi.FirstTime();
  value = mqi.InterpolatedValue(mqi.ValidTime(), 0);
  if (Convert(value) != "8.51755")
    TEST_FAILED("Expected PrecipitationRate 8.51755 got " + Convert(value));

  // testing PreviousTime()
  result_vector.clear();
  mqi.LastTime();
  result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  while (mqi.PreviousTime())
  {
    result_vector.push_back(mqi.InterpolatedValue(mqi.ValidTime(), 0));
  }

  /*
  qi->FirstTime();
  while(qi->NextTime())
    {
          std::cout << "qi->Time(): " << qi->Time() << endl;
          std::cout << "qi->ValidTime(): " << qi->ValidTime() << endl;
          std::cout << "qi->IsValidTime(): " << qi->IsValidTime() << endl;
    }
  if(!qi->NextTime())
    {
          std::cout << "qi->IsValidTime()2: " << qi->IsValidTime() << endl;
          qi->FirstTime();
          qi->NextTime();
          NFmiMetTime tim(qi->ValidTime());
          std::cout << "timmi: " << tim << endl;
          qi->ResetTime();
          std::cout << "qi->IsValidTime()3: " << qi->IsValidTime() << endl;
          std::cout << "qi->ValidTime()3: " << qi->ValidTime() << endl;
          qi->Time(tim);
          std::cout << "qi->IsValidTime()4: " << qi->IsValidTime() << endl;
          std::cout << "qi->Time()4: " << qi->Time() << endl;
          std::cout << "qi->ValidTime()4: " << qi->ValidTime() << endl;
    }
  std::cout << "qi->IsValidTime()5: " << qi->IsValidTime() << endl;
  std::cout << "qi->Time()5: " << qi->ValidTime() << endl;
  std::cout << "qi->ValidTime()5: " << qi->ValidTime() << endl;

  qi->Time(NFmiMetTime(2013,9,9,10,10,0,0));
  std::cout << "qi->IsValidTime()6: " << qi->IsValidTime() << endl;
  std::cout << "qi->ValidTime()6: " << qi->ValidTime() << endl;
  std::cout << "qi->Time()6: " << qi->Time() << endl;
  */

  if (Convert(result_vector[0]) != "12.9554" || Convert(result_vector[1]) != "11.807" ||
      Convert(result_vector[2]) != "9.86303" || Convert(result_vector[3]) != "9.00446" ||
      Convert(result_vector[4]) != "9.70439" || Convert(result_vector[5]) != "12.3766" ||
      Convert(result_vector[6]) != "10.4724" || Convert(result_vector[7]) != "9.33667" ||
      Convert(result_vector[8]) != "7.63031" || Convert(result_vector[9]) != "8.51755")
  {
    std::string result_str;
    result_str.append(Convert(result_vector[0]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[1]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[2]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[3]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[4]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[5]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[6]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[7]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[8]));
    result_str.append(", ");
    result_str.append(Convert(result_vector[9]));
    TEST_FAILED(
        "Expected PrecipitationRate 12.9554, 11.807, 9.86303, 9.00446, 9.70439, 12.3766, 10.4724, "
        "9.33667, 7.63031, 8.51755 got " +
        result_str);
  }

  TEST_PASSED();
}

void multifile_grid_interpolation()
{
  using NFmiStringTools::Convert;

  std::list<std::string> files;
  files.push_back("data/201309110600_tutka_suomi_rr.sqd");
  files.push_back("data/201309110605_tutka_suomi_rr.sqd");

  NFmiMultiQueryInfo mqi(files);
  mqi.ResetTime();
  mqi.Param(kFmiPrecipitationRate);
  mqi.Location(NFmiLocation(25.7333, 60.8083));
  vector<float> result_vector;

  /*
  NFmiDataMatrix<float> dataMatrix;

  NFmiFastQueryInfo& fqi = mqi;
  mqi.Values(dataMatrix);
  fqi->Values(dataMatrix);
  */

  /*
  std::cout << "dataMatrix.size()1: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;

  mqi.FirstTime();
  mqi.Values(dataMatrix, mqi.ValidTime());
  std::cout << "dataMatrix.size()2: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;
  dataMatrix = NFmiDataMatrix<float>();
  std::cout << "dataMatrix.size()3: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;
  mqi.ResetTime();
  std::cout << "dataMatrix.size()31: " << mqi.ValidTime() << endl;
  mqi.LastTime();
  std::cout << "dataMatrix.size()32: " << mqi.ValidTime() << endl;
  mqi.Values(dataMatrix, mqi.ValidTime());
  std::cout << "dataMatrix.size()4: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;
  std::cout << "dataMatrix.value4: " << dataMatrix[0][0] << ", " << dataMatrix[0][1] << endl;
  dataMatrix = NFmiDataMatrix<float>();
  std::cout << "dataMatrix.size()5: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;

  NFmiMetTime intermediateTime(2013,9,11,6,3,0,1);
  mqi.Values(dataMatrix, intermediateTime);
  std::cout << "dataMatrix.size()6: " << dataMatrix.NX() << ", " << dataMatrix.NY() << endl;
  std::cout << "dataMatrix.value6: " << dataMatrix[0][0] << ", " << dataMatrix[0][1] << endl;
  */

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
    TEST(multifile_non_overlapping);
    TEST(multifile_overlapping);
    TEST(multifile_grid_interpolation);
  }
};

}  // namespace NFmiMultiQueryInfoTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiMultiQueryInfo tester" << endl << "====================" << endl;
  NFmiMultiQueryInfoTest::tests t;
  return t.run();
}

// ======================================================================
