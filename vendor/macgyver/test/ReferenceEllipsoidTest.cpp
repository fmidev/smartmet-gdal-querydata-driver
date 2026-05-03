#include "ReferenceEllipsoid.h"
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <cmath>

using namespace boost::unit_test;

namespace
{
const double GR = M_PI / 180.0;
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "ReferenceEllipsoid tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(toGeocentricTestZeroHeight)
{
  unit_test_log.set_threshold_level(log_messages);
  BOOST_TEST_MESSAGE("+ [Testing conversion to geocentric coordinates (zero height)]");

  Fmi::ReferenceEllipsoid wgs84(6378137.0, 1.0 / 298.257223563);
  const double x_exp = 2897560.7831;
  const double y_exp = 1351154.7831;
  const double z_exp = 5500477.13;
  auto x = wgs84.to_geocentric(GR * 60.0, GR * 25.0, 0.0);

  BOOST_CHECK_CLOSE(x[0], x_exp, 1e-3);
  BOOST_CHECK_CLOSE(x[1], y_exp, 1e-3);
  BOOST_CHECK_CLOSE(x[2], z_exp, 1e-3);
}

BOOST_AUTO_TEST_CASE(toGeocentricTest)
{
  unit_test_log.set_threshold_level(log_messages);
  BOOST_TEST_MESSAGE("+ [Testing conversion to geocentric coordinates (non-zero height)]");

  Fmi::ReferenceEllipsoid wgs84(6378137.0, 1.0 / 298.257223563);
  const double x_exp = 2899826.5526;
  const double y_exp = 1352211.3288;
  const double z_exp = 5504807.26;
  auto x = wgs84.to_geocentric(GR * 60.0, GR * 25.0, 5000.0);

  BOOST_CHECK_CLOSE(x[0], x_exp, 1e-7);
  BOOST_CHECK_CLOSE(x[1], y_exp, 1e-7);
  BOOST_CHECK_CLOSE(x[2], z_exp, 1e-7);
}

BOOST_AUTO_TEST_CASE(toGeodeticZeroHeight)
{
  unit_test_log.set_threshold_level(log_messages);
  BOOST_TEST_MESSAGE("+ [Testing conversion to geocentric coordinates (zero height)]");

  Fmi::ReferenceEllipsoid fmi(6378220.0, 0.0);
  Fmi::ReferenceEllipsoid wgs84(6378137.0, 1.0 / 298.257223563);

  for (int i = -90; i <= 90; i++)
  {
    const double lat0 = i;
    const double lon0 = 0.4 * i;
    auto x = wgs84.to_geocentric(GR * lat0, GR * lon0, 0.0);
    double lat, lon, height;
    wgs84.to_geodetic(x, &lat, &lon, &height);
    BOOST_CHECK_CLOSE(lat / GR, lat0, 1e-8);
    BOOST_CHECK_CLOSE(lon / GR, lon0, 1e-8);
    BOOST_CHECK_SMALL(height, 1e-4);
    // printf("### %15.6f %15.6f %15.6f ", lat0, lon0, 0.0);
    // printf(": %15.6f %15.6f %15.6f", lat/GR, lon/GR, height);
    x = fmi.to_geocentric(GR * lat0, GR * lon0, 0.0);
    fmi.to_geodetic(x, &lat, &lon, &height);
    BOOST_CHECK_CLOSE(lat / GR, lat0, 1e-8);
    BOOST_CHECK_CLOSE(lon / GR, lon0, 1e-8);
    BOOST_CHECK_SMALL(height, 1e-4);

    // printf(": %15.6f %15.6f %15.6f", lat/GR, lon/GR, height);
    // printf("\n");
  }
}

BOOST_AUTO_TEST_CASE(toGeodetic)
{
  unit_test_log.set_threshold_level(log_messages);
  BOOST_TEST_MESSAGE("+ [Testing conversion to geodetic coordinates (non-zero height)]");

  Fmi::ReferenceEllipsoid wgs84(6378137.0, 1.0 / 298.257223563);

  for (int i = -90; i <= 90; i++)
  {
    const double lat0 = i;
    const double lon0 = 0.4 * i;
    const double h0 = 1000.0 + 5 * i;
    auto x = wgs84.to_geocentric(GR * lat0, GR * lon0, h0);
    double lat, lon, height;
    wgs84.to_geodetic(x, &lat, &lon, &height);
    BOOST_CHECK_CLOSE(lat / GR, lat0, 1e-8);
    BOOST_CHECK_CLOSE(lon / GR, lon0, 1e-8);
    BOOST_CHECK_CLOSE(height, h0, 1e-4);
  }
}
