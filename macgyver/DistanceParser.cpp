#include "DistanceParser.h"
#include "Exception.h"
#include "StringConversion.h"

namespace Fmi
{
namespace DistanceParser
{
static const char* KM = "km";
static const char* M = "m";
static const char* CM = "cm";
static const char* MM = "mm";
static const char* MI = "mi";
static const char* NMI = "nmi";
static const char* YD = "yd";
static const char* FT = "ft";
static const char* IN = "in";
static const char* KILOMETER = "kilometer";
static const char* METER = "meter";
static const char* CENTIMETER = "centimeter";
static const char* MILLIMETER = "millimeter";
static const char* MILE = "mile";
static const char* NAUTICAL_MILE = "nautical_mile";
static const char* YARD = "yard";
static const char* FOOT = "foot";
static const char* INCH = "inch";
static const char* NUMBER_SIGNS = "+-. 1234567890";

double convert(double value, const std::string& unit, const std::string& format)
{
  if (unit.empty())
    return value;

  if (unit == KM)
  {
    if (format == METER)
      return (value * 1000.0);
    if (format == CENTIMETER)
      return (value * 100000.0);
    if (format == MILLIMETER)
      return (value * 1000000.0);
    if (format == MILE)
      return (value * 0.621371);
    if (format == NAUTICAL_MILE)
      return (value * 0.539957);
    if (format == YARD)
      return (value * 1093.61);
    if (format == FOOT)
      return (value * 3280.84);
    if (format == INCH)
      return (value * 39370.1);
  }
  else if (unit == M)
  {
    if (format == KILOMETER)
      return (value / 1000.0);
    if (format == CENTIMETER)
      return (value / 0.01);
    if (format == MILLIMETER)
      return (value / 0.001);
    if (format == MILE)
      return (value * 0.000621371);
    if (format == NAUTICAL_MILE)
      return (value * 0.000539957);
    if (format == YARD)
      return (value * 1.09361);
    if (format == FOOT)
      return (value * 3.28084);
    if (format == INCH)
      return (value * 39.3701);
  }
  else if (unit == CM)
  {
    if (format == METER)
      return (value / 100.0);
    if (format == KILOMETER)
      return (value / 100000.0);
    if (format == MILLIMETER)
      return (value * 10.0);
    if (format == MILE)
      return (value * 0.00000621371);
    if (format == NAUTICAL_MILE)
      return (value * 0.00000539957);
    if (format == YARD)
      return (value * 0.0109361);
    if (format == FOOT)
      return (value * 0.0328084);
    if (format == INCH)
      return (value * 0.393701);
  }
  else if (unit == MM)
  {
    if (format == KILOMETER)
      return (value / 1000000.0);
    if (format == METER)
      return (value / 1000.0);
    if (format == CENTIMETER)
      return (value / 10.0);
    if (format == MILE)
      return (value * 0.000000621371);
    if (format == NAUTICAL_MILE)
      return (value * 0.000000539957);
    if (format == YARD)
      return (value * 0.00109361);
    if (format == FOOT)
      return (value * 0.00328084);
    if (format == INCH)
      return (value * 0.0393701);
  }
  else if (unit == MI)
  {
    if (format == KILOMETER)
      return (value * 1.609344);
    if (format == METER)
      return (value * 1609.344);
    if (format == CENTIMETER)
      return (value * 160934.4);
    if (format == MILLIMETER)
      return (value * 1609344.0);
    if (format == NAUTICAL_MILE)
      return (value * 0.868976);
    if (format == YARD)
      return (value * 1760.0);
    if (format == FOOT)
      return (value * 5280.0);
    if (format == INCH)
      return (value * 63360);
  }
  else if (unit == NMI)
  {
    if (format == KILOMETER)
      return (value * 1.852);
    if (format == METER)
      return (value * 1852.0);
    if (format == CENTIMETER)
      return (value * 185200.0);
    if (format == MILLIMETER)
      return (value * 1852000.0);
    if (format == MILE)
      return (value * 1.15078);
    if (format == YARD)
      return (value * 2025.37);
    if (format == FOOT)
      return (value * 6076.12);
    if (format == INCH)
      return (value * 72913.4);
  }
  else if (unit == YD)
  {
    if (format == KILOMETER)
      return (value * 0.0009144);
    if (format == METER)
      return (value * 0.9144);
    if (format == CENTIMETER)
      return (value * 91.44);
    if (format == MILLIMETER)
      return (value * 914.40);
    if (format == MILE)
      return (value * 0.000568182);
    if (format == NAUTICAL_MILE)
      return (value * 0.000493737);
    if (format == FOOT)
      return (value * 3.0);
    if (format == INCH)
      return (value * 36.0);
  }
  else if (unit == FT)
  {
    if (format == KILOMETER)
      return (value * 0.0003048);
    if (format == METER)
      return (value * 0.3048);
    if (format == CENTIMETER)
      return (value * 30.48);
    if (format == MILLIMETER)
      return (value * 304.80);
    if (format == MILE)
      return (value * 0.000189394);
    if (format == NAUTICAL_MILE)
      return (value * 0.000164579);
    if (format == YARD)
      return (value * 0.333333);
    if (format == INCH)
      return (value * 12.0);
  }
  else if (unit == IN)
  {
    if (format == KILOMETER)
      return (value * 0.0000254);
    if (format == METER)
      return (value * 0.0254);
    if (format == CENTIMETER)
      return (value * 2.54);
    if (format == MILLIMETER)
      return (value * 25.4);
    if (format == MILE)
      return (value * 0.0000157828);
    if (format == NAUTICAL_MILE)
      return (value * 0.0000137149);
    if (format == YARD)
      return (value * 0.0277778);
    if (format == FOOT)
      return (value * 0.0833333);
  }
  else
    throw Fmi::Exception(BCP, "Invalid unit: " + unit);

  return value;
}

double parse_kilometer(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, KILOMETER);
  }

  return Fmi::stod(str);
}

double parse_meter(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, METER);
  }

  return Fmi::stod(str);
}

double parse_centimeter(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, CENTIMETER);
  }

  return Fmi::stod(str);
}

double parse_millimeter(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, MILLIMETER);
  }

  return Fmi::stod(str);
}

double parse_mile(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, MILE);
  }

  return Fmi::stod(str);
}

double parse_nautical_mile(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, NAUTICAL_MILE);
  }

  return Fmi::stod(str);
}

double parse_foot(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, FOOT);
  }

  return Fmi::stod(str);
}

double parse_yard(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, YARD);
  }

  return Fmi::stod(str);
}

double parse_inch(const std::string& str)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);

  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));

    return convert(value, unit, INCH);
  }

  return Fmi::stod(str);
}

double parse(const std::string& str, const std::string& format)
{
  std::size_t unit_pos = str.find_first_not_of(NUMBER_SIGNS);
  if (unit_pos != std::string::npos)
  {
    std::string unit = str.substr(unit_pos);
    double value = Fmi::stod(str.substr(0, unit_pos));
    return convert(value, unit, format);
  }

  return Fmi::stod(str);
}

}  // namespace DistanceParser
}  // namespace Fmi
