// ======================================================================
/*!
 * \file
 * \brief Regression tests for class DistanceParser
 */
// ======================================================================

#include "DistanceParser.h"
#include "StringConversion.h"
#include <regression/tframe.h>


// Protection against conflicts with global functions
namespace DistanceParserTest
{
// ----------------------------------------------------------------------

void parse_kilometer()
{
  auto parsed_value = Fmi::DistanceParser::parse_kilometer("1km");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (km->km) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1.0km");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (km->km) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1.0");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (km->km) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (km->km) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1m");
  if(parsed_value != 0.001)
	TEST_FAILED("Expected (m->km) " + Fmi::to_string(0.001) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1cm");
  if(parsed_value != 0.00001)
	TEST_FAILED("Expected (cm->km) " + Fmi::to_string(0.00001) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1mm");
  if(parsed_value != 0.000001)
	TEST_FAILED("Expected (mm->km) " + Fmi::to_string(0.000001) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1mi");
  if(parsed_value != 1.609344)
	TEST_FAILED("Expected (mi->km) " + Fmi::to_string(1.609344) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1nmi");
  if(parsed_value != 1.852)
	TEST_FAILED("Expected (nmi->km) " + Fmi::to_string(1.852) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1ft");
  if(parsed_value != 0.0003048)
	TEST_FAILED("Expected (ft->km) " + Fmi::to_string(0.0003048) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1yd");
  if(parsed_value != 0.0009144)
	TEST_FAILED("Expected (yd->km) " + Fmi::to_string(0.0009144) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_kilometer("1in");
  if(parsed_value != 0.0000254)
	TEST_FAILED("Expected (in->km) " + Fmi::to_string(0.0000254) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_meter()
{
  auto parsed_value = Fmi::DistanceParser::parse_meter("1km");
  if(parsed_value != 1000.0)
	TEST_FAILED("Expected (km->m) " + Fmi::to_string(1000.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1m");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (m->m) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1cm");
  if(parsed_value != 0.01)
	TEST_FAILED("Expected (cm->m) " + Fmi::to_string(0.01) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1mm");
  if(parsed_value != 0.001)
	TEST_FAILED("Expected (mm->m) " + Fmi::to_string(0.001) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1mi");
  if(parsed_value != 1609.344)
	TEST_FAILED("Expected (mi->m) " + Fmi::to_string(1609.344) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1nmi");
  if(parsed_value != 1852.0)
	TEST_FAILED("Expected (nmi->m) " + Fmi::to_string(1852.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1ft");
  if(parsed_value != 0.3048)
	TEST_FAILED("Expected (ft->m) " + Fmi::to_string(0.3048) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1yd");
  if(parsed_value != 0.9144)
	TEST_FAILED("Expected (yd->m) " + Fmi::to_string(0.9144) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_meter("1in");
  if(parsed_value != 0.0254)
	TEST_FAILED("Expected (in->m) " + Fmi::to_string(0.0254) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_centimeter()
{
  auto parsed_value = Fmi::DistanceParser::parse_centimeter("1km");
  if(parsed_value != 100000.0)
	TEST_FAILED("Expected (km->cm) " + Fmi::to_string(100000.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1m");
  if(parsed_value != 100.0)
	TEST_FAILED("Expected (m->cm) " + Fmi::to_string(100.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1cm");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (cm->cm) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1mm");
  if(parsed_value != 0.1)
	TEST_FAILED("Expected (mm->cm) " + Fmi::to_string(0.1) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1mi");
  if(parsed_value != 160934.4)
	TEST_FAILED("Expected (mi->cm) " + Fmi::to_string(160934.4) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1nmi");
  if(parsed_value != 185200.0)
	TEST_FAILED("Expected (nmi->cm) " + Fmi::to_string(185200.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1ft");
  if(parsed_value != 30.48)
	TEST_FAILED("Expected (ft->cm) " + Fmi::to_string(30.48) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1yd");
  if(parsed_value != 91.44)
	TEST_FAILED("Expected (yd->cm) " + Fmi::to_string(91.44) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_centimeter("1in");
  if(parsed_value != 2.54)
	TEST_FAILED("Expected (in->cm) " + Fmi::to_string(2.54) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_millimeter()
{
  auto parsed_value = Fmi::DistanceParser::parse_millimeter("1km");
  if(parsed_value != 1000000.0)
	TEST_FAILED("Expected (km->mm) " + Fmi::to_string(1000000.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1m");
  if(parsed_value != 1000.0)
	TEST_FAILED("Expected (m->mm) " + Fmi::to_string(1000.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1cm");
  if(parsed_value != 10.0)
	TEST_FAILED("Expected (cm->mm) " + Fmi::to_string(10.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1mm");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (mm->mm) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));


  parsed_value = Fmi::DistanceParser::parse_millimeter("1mi");
  if(parsed_value != 1609344.0)
	TEST_FAILED("Expected (mi->mm) " + Fmi::to_string(1609344.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1nmi");
  if(parsed_value != 1852000.0)
	TEST_FAILED("Expected (nmi->mm) " + Fmi::to_string(1852000.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1ft");
  if(parsed_value != 304.8)
	TEST_FAILED("Expected (ft->mm) " + Fmi::to_string(304.8) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1yd");
  if(parsed_value != 914.4)
	TEST_FAILED("Expected (yd->mm) " + Fmi::to_string(914.4) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_millimeter("1in");
  if(parsed_value != 25.4)
	TEST_FAILED("Expected (in->mm) " + Fmi::to_string(25.4) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_mile()
{
  auto parsed_value = Fmi::DistanceParser::parse_mile("1km");
  if(parsed_value != 0.621371)
	TEST_FAILED("Expected (km->mi) " + Fmi::to_string(0.621371) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1m");
  if(parsed_value != 0.000621371)
	TEST_FAILED("Expected (m->mi) " + Fmi::to_string(0.000621371) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1cm");
  if(parsed_value != 0.00000621371)
	TEST_FAILED("Expected (cm->mi) " + Fmi::to_string(0.00000621371) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1mm");
  if(parsed_value != 0.000000621371)
	TEST_FAILED("Expected (mm->mi) " + Fmi::to_string(0.000000621371) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1mi");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (mi->mi) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1nmi");
  if(parsed_value != 1.15078)
	TEST_FAILED("Expected (nmi->mi) " + Fmi::to_string(1.15078) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1ft");
  if(parsed_value != 0.000189394)
	TEST_FAILED("Expected (ft->mi) " + Fmi::to_string(0.000189394) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1yd");
  if(parsed_value != 0.0005681820)
	TEST_FAILED("Expected (ft->mi) " + Fmi::to_string(0.0005681820) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_mile("1in");
  if(parsed_value != 0.0000157828)
	TEST_FAILED("Expected (ft->mi) " + Fmi::to_string(0.0000157828) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_nautical_mile()
{
  auto parsed_value = Fmi::DistanceParser::parse_nautical_mile("1km");
  if(parsed_value != 0.539957)
	TEST_FAILED("Expected (km->nmi) " + Fmi::to_string(0.539957) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1m");
  if(parsed_value != 0.000539957)
	TEST_FAILED("Expected (m->nmi) " + Fmi::to_string(0.000539957) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1cm");
  if(parsed_value != 0.00000539957)
	TEST_FAILED("Expected (cm->nmi) " + Fmi::to_string(0.00000539957) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1mm");
  if(parsed_value != 0.000000539957)
	TEST_FAILED("Expected (mm->nmi) " + Fmi::to_string(0.000000539957) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1mi");
  if(parsed_value != 0.868976)
	TEST_FAILED("Expected (mi->nmi) " + Fmi::to_string(0.868976) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1nmi");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (nmi->nmi) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1ft");
  if(parsed_value != 0.000164579)
	TEST_FAILED("Expected (ft->nmi) " + Fmi::to_string(0.000164579) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1yd");
  if(parsed_value != 0.000493737)
	TEST_FAILED("Expected (yd->nmi) " + Fmi::to_string(0.000493737) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_nautical_mile("1in");
  if(parsed_value != 0.0000137149)
	TEST_FAILED("Expected (in->nmi) " + Fmi::to_string(0.0000137149) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_yard()
{
  auto parsed_value = Fmi::DistanceParser::parse_yard("1km");
  if(parsed_value != 1093.61)
	TEST_FAILED("Expected (km->yd) " + Fmi::to_string(1093.61) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1m");
  if(parsed_value != 1.09361)
	TEST_FAILED("Expected (m->yd) " + Fmi::to_string(1.09361) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1cm");
  if(parsed_value != 0.0109361)
	TEST_FAILED("Expected (cm->yd) " + Fmi::to_string(0.0109361) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1mm");
  if(parsed_value != 0.00109361)
	TEST_FAILED("Expected (mm->yd) " + Fmi::to_string(0.00109361) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1mi");
  if(parsed_value != 1760.0)
	TEST_FAILED("Expected (mi->yd) " + Fmi::to_string(1760.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1nmi");
  if(parsed_value != 2025.37)
	TEST_FAILED("Expected (nmi->yd) " + Fmi::to_string(2025.37) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1ft");
  if(parsed_value != 0.333333)
	TEST_FAILED("Expected (ft->yd) " + Fmi::to_string(0.333333) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1yd");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (yd->yd) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_yard("1in");
  if(parsed_value != 0.0277778)
	TEST_FAILED("Expected (in->yd) " + Fmi::to_string(0.0277778) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_foot()
{
  auto parsed_value = Fmi::DistanceParser::parse_foot("1km");
  if(parsed_value != 3280.84)
	TEST_FAILED("Expected (km->ft) " + Fmi::to_string(3280.84) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1m");
  if(parsed_value != 3.28084)
	TEST_FAILED("Expected (m->ft) " + Fmi::to_string(3.28084) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1cm");
  if(parsed_value != 0.0328084)
	TEST_FAILED("Expected (cm->ft) " + Fmi::to_string(0.0328084) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1mm");
  if(parsed_value != 0.00328084)
	TEST_FAILED("Expected (mm->ft) " + Fmi::to_string(0.00328084) + ", got " + Fmi::to_string(parsed_value));


  parsed_value = Fmi::DistanceParser::parse_foot("1mi");
  if(parsed_value != 5280.0)
	TEST_FAILED("Expected (mi->ft) " + Fmi::to_string(5280.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1nmi");
  if(parsed_value != 6076.12)
	TEST_FAILED("Expected (nmi->ft) " + Fmi::to_string(6076.12) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1ft");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (ft->ft) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1yd");
  if(parsed_value != 3.0)
	TEST_FAILED("Expected (yd->ft) " + Fmi::to_string(3.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_foot("1in");
  if(parsed_value != 0.0833333)
	TEST_FAILED("Expected (in->ft) " + Fmi::to_string(0.0833333) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void parse_inch()
{
  auto parsed_value = Fmi::DistanceParser::parse_inch("1km");
  if(parsed_value != 39370.1)
	TEST_FAILED("Expected (km->in) " + Fmi::to_string(39370.1) + ", got " + Fmi::to_string(parsed_value));
 
  parsed_value = Fmi::DistanceParser::parse_inch("1m");
  if(parsed_value != 39.3701)
	TEST_FAILED("Expected (km->in) " + Fmi::to_string(39.3701) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1cm");
  if(parsed_value != 0.393701)
	TEST_FAILED("Expected (km->in) " + Fmi::to_string(0.393701) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1mm");
  if(parsed_value != 0.0393701)
	TEST_FAILED("Expected (km->in) " + Fmi::to_string(0.0393701) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1mi");
  if(parsed_value != 63360.0)
	TEST_FAILED("Expected (mi->in) " + Fmi::to_string(63360.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1nmi");
  if(parsed_value != 72913.4)
	TEST_FAILED("Expected (nmi->in) " + Fmi::to_string(72913.4) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1ft");
  if(parsed_value != 12.0)
	TEST_FAILED("Expected (ft->in) " + Fmi::to_string(12.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1yd");
  if(parsed_value != 36.0)
	TEST_FAILED("Expected (yd->in) " + Fmi::to_string(36.0) + ", got " + Fmi::to_string(parsed_value));

  parsed_value = Fmi::DistanceParser::parse_inch("1in");
  if(parsed_value != 1.0)
	TEST_FAILED("Expected (in->in) " + Fmi::to_string(1.0) + ", got " + Fmi::to_string(parsed_value));

  TEST_PASSED();
}

void print_conversions()
{
  std::vector<std::string> units = {"km","m","cm","mm","mi","nmi","yd","ft","in"};
  std::vector<std::string> formats = {"kilometer","meter","centimeter","millimeter","mile","nautical_mine","yard","foot","inch"};
  
  for(unsigned int from_index = 0; from_index < units.size(); from_index++)
	for(unsigned int to_index = 0; to_index < units.size(); to_index++)
	  {
		std::string from_unit = ("1" + units.at(from_index));
		std::string format = formats.at(to_index);
		std::string to_unit = units.at(to_index);
		std::cout << from_unit << "=" << Fmi::DistanceParser::parse(from_unit, format) << to_unit << std::endl;		  
	  }
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  //  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(parse_kilometer);
    TEST(parse_meter);
    TEST(parse_centimeter);
    TEST(parse_millimeter);
    TEST(parse_mile);
    TEST(parse_nautical_mile);
    TEST(parse_yard);
    TEST(parse_foot);
    TEST(parse_inch);
	
	//print_conversions();
  }
};

}  // namespace DistanceParserTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "DistanceParser tester" << endl << "=================" << endl;
  DistanceParserTest::tests t;
  return t.run();
}

// ======================================================================
