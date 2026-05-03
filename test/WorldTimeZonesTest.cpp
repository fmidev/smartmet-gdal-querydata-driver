#include "WorldTimeZones.h"
#include <regression/tframe.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace WorldTimeZonesTest
{
const char* db = "/usr/share/smartmet/timezones/timezone.shz";

// ----------------------------------------------------------------------

void finland()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(25, 60);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Helsinki, not " + ret);

  ret = tq.zone_name(22.35, 60.57);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Turku, not " + ret);

  ret = tq.zone_name(25.72, 66.49);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Rovaniemi, not " + ret);

  ret = tq.zone_name(25.72, 66.49);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Rovaniemi, not " + ret);

  ret = tq.zone_name(27.02, 68.91);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Inari, not " + ret);

  ret = tq.zone_name(27.02, 68.91);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Inari, not " + ret);

  ret = tq.zone_name(19.94, 60.09);
  if (ret != "Europe/Mariehamn")
    TEST_FAILED("Should get Europe/Mariehamn for Maarianhamina, not " + ret);

  ret = tq.zone_name(28.85, 61.18);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Imatra, not " + ret);

  ret = tq.zone_name(24.37, 66.00);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Tornio, not " + ret);

  ret = tq.zone_name(27.00, 69.62);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Utsjoki, not " + ret);

  ret = tq.zone_name(24.38, 66.74);
  if (ret != "Europe/Helsinki")
    TEST_FAILED("Should get Europe/Helsinki for Pello, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void sweden()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(18.08, 59.32);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Tukholma, not " + ret);

  ret = tq.zone_name(24.138, 65.842);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Haaparanta, not " + ret);

  ret = tq.zone_name(12.7208, 56.0366);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Helsingborg, not " + ret);

  ret = tq.zone_name(23.3879, 67.2124);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Pajala, not " + ret);

  ret = tq.zone_name(12.00, 57.72);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Göteborg, not " + ret);

  ret = tq.zone_name(13.01095, 55.59062);
  if (ret != "Europe/Stockholm")
    TEST_FAILED("Should get Europe/Stockholm for Malmö, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void norway()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(10.72, 59.90);
  if (ret != "Europe/Oslo")
    TEST_FAILED("Should get Europe/Oslo for Oslo, not " + ret);

  ret = tq.zone_name(11.3710669396975, 59.1204641429058);
  if (ret != "Europe/Oslo")
    TEST_FAILED("Should get Europe/Oslo for Halden, not " + ret);

  ret = tq.zone_name(19.00, 69.67);
  if (ret != "Europe/Oslo")
    TEST_FAILED("Should get Europe/Oslo for Tromssa, not " + ret);

  ret = tq.zone_name(25.7844792987151, 71.1605954746137);
  if (ret != "Europe/Oslo")
    TEST_FAILED("Should get Europe/Oslo for Nordkapp, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void denmark()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(12.54, 55.69);
  if (ret != "Europe/Copenhagen")
    TEST_FAILED("Should get Europe/Copenhagen for Kööpenhamina, not " + ret);

  ret = tq.zone_name(12.6041, 56.0319);
  if (ret != "Europe/Copenhagen")
    TEST_FAILED("Should get Europe/Copenhagen for Helsingör, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void estonia()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(24.74, 59.45);
  if (ret != "Europe/Tallinn")
    TEST_FAILED("Should get Europe/Tallinn for Tallinna, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void russia()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(33.13, 68.94);
  if (ret != "Europe/Moscow")
    TEST_FAILED("Should get Europe/Moscow for Murmansk, not " + ret);

  ret = tq.zone_name(30.26, 59.89);
  if (ret != "Europe/Moscow")
    TEST_FAILED("Should get Europe/Moscow for Pietari, not " + ret);

  ret = tq.zone_name(37.67, 55.72);
  if (ret != "Europe/Moscow")
    TEST_FAILED("Should get Europe/Moscow for Moskova, not " + ret);

  ret = tq.zone_name(104.248, 52.3174);
  if (ret != "Asia/Irkutsk")
    TEST_FAILED("Should get Asia/Irkutsk for Irkutsk, not " + ret);

  ret = tq.zone_name(131.907, 43.1273);
  if (ret != "Asia/Vladivostok")
    TEST_FAILED("Should get Asia/Vladivostok for Vladivostok, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void ocean()
{
  Fmi::WorldTimeZones tq(db);

  string ret;

  ret = tq.zone_name(-47, 30);
  if (ret != "Etc/GMT-3")
    TEST_FAILED("Should get Etc/GMT-3 for coordinate -47,20 in the Atlantic, not " + ret);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

// The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(finland);
    TEST(sweden);
    TEST(norway);
    TEST(denmark);
    TEST(estonia);
    TEST(russia);
    // Disabled: waiting for fix to brainstorm-912
    // TEST(ocean);
  }

};  // class tests

}  // namespace WorldTimeZonesTest

int main(void)
{
  cout << endl << "WorldTimeZones tester" << endl << "=====================" << endl;
  WorldTimeZonesTest::tests t;
  return t.run();
}
