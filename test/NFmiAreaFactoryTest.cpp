// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiAreaFactory
 */
// ======================================================================

#include "NFmiAreaFactory.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiOrthographicArea.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiYKJArea.h"
#include <memory>
#include <macgyver/Exception.h>
#include <regression/tframe.h>

#include <stdexcept>
#include <string>

//! Protection against conflicts with global functions
namespace NFmiAreaFactoryTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for latlon projections
 */
// ----------------------------------------------------------------------

void create_latlon()
{
  {
    const std::string def = "latlon:10,20,30,40";
    NFmiLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "latlon:10,20,30,40:2,3";
    NFmiLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, 0), NFmiPoint(2, 3));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "latlon:10,20,30,40:0,1,2,3";
    NFmiLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, 1), NFmiPoint(2, 3));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for rotlatlon projections
 */
// ----------------------------------------------------------------------

void create_rotlatlon()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "rotlatlon:10,20,30,40";
    NFmiRotatedLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, -90));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "rotlatlon,-30:10,20,30,40";
    NFmiRotatedLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, -30));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "rotlatlon,-50,30:10,20,30,40";
    NFmiRotatedLatLonArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(30, -50));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for ykj projections
 */
// ----------------------------------------------------------------------

void create_ykj()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "ykj:10,20,30,40";
    NFmiYKJArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for mercator projections
 */
// ----------------------------------------------------------------------

void create_mercator()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "mercator:10,20,30,40";
    NFmiMercatorArea expected(NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for stereographic projections
 */
// ----------------------------------------------------------------------

void create_stereographic()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "stereographic:10,20,30,40";
    NFmiStereographicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 0, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 60);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "stereographic,20:10,20,30,40";
    NFmiStereographicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 60);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "stereographic,20,90,50:10,20,30,40";
    NFmiStereographicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 50);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for gnomonic projections
 */
// ----------------------------------------------------------------------

void create_gnomonic()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "gnomonic:10,20,30,40";
    NFmiGnomonicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 0, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 60);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "gnomonic,20:10,20,30,40";
    NFmiGnomonicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 60);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "gnomonic,20,90,50:10,20,30,40";
    NFmiGnomonicArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 90, 50);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for equidist projections
 */
// ----------------------------------------------------------------------

void create_equidist()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "equidist:10,20,30,40";
    NFmiEquidistArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 0, NFmiPoint(0, 0), NFmiPoint(1, 1), 90);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "equidist,20:10,20,30,40";
    NFmiEquidistArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 90);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "equidist,20,70:10,20,30,40";
    NFmiEquidistArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), 20, NFmiPoint(0, 0), NFmiPoint(1, 1), 70);
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test NFmiAreaFactory::Create() for Gdal projections
 */
// ----------------------------------------------------------------------

void create_gdal()
{
  std::shared_ptr<NFmiArea> area;

  {
    const std::string def = "EPSG:2393|10,20,30,40";
    NFmiGdalArea expected("WGS84", "EPSG:2393", NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "WGS84:EPSG:2393|10,20,30,40";
    NFmiGdalArea expected("WGS84", "EPSG:2393", NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    const std::string def = "FMI:EPSG:2393|10,20,30,40";
    NFmiGdalArea expected("FMI", "EPSG:2393", NFmiPoint(10, 20), NFmiPoint(30, 40));
    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));
    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void create_desired_resolution()
{
  std::shared_ptr<NFmiArea> area;

  {
    NFmiLatLonArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, 0), NFmiPoint(222, 222));

    const std::string def = "latlon:10,20,30,40:10,10km";

    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));

    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  {
    NFmiLatLonArea expected(
        NFmiPoint(10, 20), NFmiPoint(30, 40), NFmiPoint(0, 0), NFmiPoint(222, 222));

    const std::string def = "latlon:10,20,30,40:10000x10000m";

    std::shared_ptr<NFmiArea> area(NFmiAreaFactory::Create(def));

    if (!(expected == *area))
      TEST_FAILED("Failed to create " + def);
  }

  TEST_PASSED();
}

void create_latvia_stereographic_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  // Latvia stereographic projection
  const std::string def = "+lat_0=90 +proj=stere +lat_ts=60 +ellps=bessel +lon_0=14";
  NFmiStereographicArea expected(
      BottomLeftLatLon, TopRightLatLon, 14, TopLeftXY, BottomRightXY, 90, 60);
  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_FMI_stereographic_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  // Latvia stereographic projection
  const std::string def =
      "+proj=stere +lat_0=90 +lon_0=20 +lat_ts=60 +a=6371288 +x_0=124657.602609 "
      "+y_0=3252402.178534 +units=m";
  NFmiStereographicArea expected(
      BottomLeftLatLon, TopRightLatLon, 20, TopLeftXY, BottomRightXY, 90, 60);
  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_orthographic_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def = "+proj=ortho +lat_0=90N +lat_ts=60 +x_0=580886.31 +y_0=3595677.92";
  NFmiOrthographicArea expected(BottomLeftLatLon, TopRightLatLon, 0, TopLeftXY, BottomRightXY);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_gnomonic_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def =
      "+proj=gnom +lon_0=28E +lat_0=90N +lat_ts=60 +x_0=580886.31 +y_0=3595677.92";
  NFmiGnomonicArea expected(BottomLeftLatLon, TopRightLatLon, 28, TopLeftXY, BottomRightXY, 90, 60);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_latvia_equidist_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def = "+proj=aeqd +lon_0=23.9897 +lat_0=56.9143 +ellps=sphere +a=6371000";
  NFmiEquidistArea expected(
      BottomLeftLatLon, TopRightLatLon, 23.9897, TopLeftXY, BottomRightXY, 56.9143);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_latlon_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def = "+proj=latlon  +lon_0=22E";
  NFmiLatLonArea expected(BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_mercator_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def = "+proj=merc";
  NFmiMercatorArea expected(BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void create_ykj_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def =
      "+proj=tmerc +lat_0=0 +lon_0=27 +k=1 +x_0=3500000 +y_0=0 +ellps=intl +units=m +no_defs";
  NFmiYKJArea expected(BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void test_negative_params_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  const std::string def =
      "+proj=stere +lon_0=25w +lat_0=90 +lat_ts=10s +x_0=580886.31 +y_0=3595677.92";
  NFmiStereographicArea expected(
      BottomLeftLatLon, TopRightLatLon, -25, TopLeftXY, BottomRightXY, 90, -10);

  std::shared_ptr<NFmiArea> area =
      NFmiAreaFactory::CreateProj(def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
  if (!(expected == *area))
    TEST_FAILED("Failed to create " + def);

  TEST_PASSED();
}

void test_invalid_input_proj()
{
  const NFmiPoint BottomLeftLatLon(10, 20), TopRightLatLon(30, 40), TopLeftXY(0, 0),
      BottomRightXY(1, 1);
  {
    // Missing lon_0 parameter
    const std::string def = "+proj=stere +lat_0=90 +lat_ts=10s +x_0=580886.31 +y_0=3595677.92";
    try
    {
      std::shared_ptr<NFmiArea> area = NFmiAreaFactory::CreateProj(
          def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
      TEST_FAILED("No exception thrown for missing lon_0");
    }

    catch (Fmi::Exception& e)
    {
    }
    catch (...)
    {
      TEST_FAILED("Wrong kind of exception thrown");
    }
  }

  {
    // Missing proj definition
    const std::string def = "+lat_0=90 +lat_ts=10s +x_0=580886.31 +y_0=3595677.92";
    try
    {
      std::shared_ptr<NFmiArea> area = NFmiAreaFactory::CreateProj(
          def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
      TEST_FAILED("No exception thrown for missing proj definition");
    }

    catch (Fmi::Exception& e)
    {
    }
    catch (...)
    {
      TEST_FAILED("Wrong kind of exception thrown");
    }
  }

  {
    // Missing missing +-prefixes in parameters
    const std::string def = "+lat_0=90 lat_ts=10s x_0=580886.31 +y_0=3595677.92";
    try
    {
      std::shared_ptr<NFmiArea> area = NFmiAreaFactory::CreateProj(
          def, BottomLeftLatLon, TopRightLatLon, TopLeftXY, BottomRightXY);
      TEST_FAILED("No exception thrown for missing + prefix");
    }

    catch (Fmi::Exception& e)
    {
    }
    catch (...)
    {
      TEST_FAILED("Wrong kind of exception thrown");
    }
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
    TEST(create_latlon);
    TEST(create_rotlatlon);
    TEST(create_ykj);
    TEST(create_mercator);
    TEST(create_stereographic);
    TEST(create_gnomonic);
    TEST(create_equidist);
    TEST(create_latvia_stereographic_proj);
    TEST(create_FMI_stereographic_proj);
    TEST(create_orthographic_proj);
    TEST(create_gnomonic_proj);
    TEST(create_latvia_equidist_proj);
    TEST(create_latlon_proj);
    TEST(create_mercator_proj);
    TEST(create_ykj_proj);
    TEST(create_desired_resolution);
    TEST(create_gdal);
    TEST(test_negative_params_proj);
    TEST(test_invalid_input_proj);
  }
};

}  // namespace NFmiAreaFactoryTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiAreaFactory tester" << endl << "======================" << endl;
  NFmiAreaFactoryTest::tests t;
  return t.run();
}

// ======================================================================
