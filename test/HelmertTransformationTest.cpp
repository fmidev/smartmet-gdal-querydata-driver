#include "HelmertTransformation.h"
#include <boost/test/included/unit_test.hpp>

#include <cstdio>

using namespace boost::unit_test;

namespace
{
const double GR = M_PI / 180.0;
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "HelmertTransformationTest tester";
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
  BOOST_TEST_MESSAGE("+ [Testing conversion from FMI sphere to WGS84]");

  const double R0 = 6371220.0;
  const double lat0 = 60;
  const double lon0 = 25;

  Fmi::ReferenceEllipsoid fmi(R0, 0);
  Fmi::HelmertTransformation conv, conv2;

  for (int i = 0; i <= 2; i++)
  {
    Fmi::HelmertTransformation::FmiSphereConvScalingType scaling_type =
        static_cast<Fmi::HelmertTransformation::FmiSphereConvScalingType>(i);

    conv.set_fmi_sphere_to_reference_ellipsoid_conv(
        R0, GR * lat0, GR * lon0, Fmi::ReferenceEllipsoid::wgs84, scaling_type);

    conv2.set_reference_ellipsoid_to_fmi_sphere_conv(
        R0, GR * lat0, GR * lon0, Fmi::ReferenceEllipsoid::wgs84, scaling_type);

    // printf("H: %16.5f %16.5f %16.5f %16.12f\n", conv.tx, conv.ty, conv.tz, conv.m);
    // printf("H: %16.5f %16.5f %16.5f %16.12f\n", conv2.tx, conv2.ty, conv2.tz, conv2.m);
    // printf("## %s\n", Fmi::get_fmi_sphere_towgs84_proj4_string(R0, lat0*GR, lon0*GR,
    //                                                           scaling_type).c_str());

    const int N = 1;
    double y_step = 0.01;
    double x_step = 2 * y_step;
    for (int ix = -N; ix <= N; ix++)
      for (int iy = -N; iy <= N; iy++)
      {
        double lat1 = GR * (lat0 + iy * y_step);
        double lon1 = GR * (lon0 + ix * x_step);
        const auto fmi_xyz = fmi.to_geocentric(lat1, lon1, 0.0);
        const auto wgs_xyz = conv(fmi_xyz);
        double lat2, lon2, h2;
        double lat3, lon3, h3;
        Fmi::ReferenceEllipsoid::wgs84.to_geodetic(wgs_xyz, &lat2, &lon2, &h2);

        BOOST_CHECK_CLOSE(lat2 / GR, lat1 / GR, 0.008);
        BOOST_CHECK_CLOSE(lon2 / GR, lon1 / GR, 0.008);
        BOOST_CHECK_SMALL(h2, 0.5);

        const auto fmi2_xyz = conv2(wgs_xyz);
        fmi.to_geodetic(fmi2_xyz, &lat3, &lon3, &h3);

        BOOST_CHECK_CLOSE(lat3 / GR, lat1 / GR, 1e-5);
        BOOST_CHECK_CLOSE(lon3 / GR, lon1 / GR, 1e-5);
        BOOST_CHECK_SMALL(h3, 0.001);

#if 0
            printf("## %15.8f %15.8f ", lat1/GR, lon1/GR);
            printf(" : %15.8f %15.8f %12.4f", (lat2-lat1)/GR, (lon2-lon1)/GR, h2);
            printf(" : %15.8f %15.8f %12.4f", (lat3-lat1)/GR, (lon3-lon1)/GR, h3);
            printf("\n");
            //printf(" : %15.3f %15.3f %15.3f ", fmi_xyz[0] , fmi_xyz[1], fmi_xyz[2]);
            //printf(" : %15.3f %15.3f %15.3f ", wgs_xyz[0] , wgs_xyz[1], wgs_xyz[2]);
            //printf(" : %15.3f %15.3f %15.3f ", fmi2_xyz[0] , fmi2_xyz[1], fmi2_xyz[2]);
            //printf("\n");
#endif
      }
  }
}
