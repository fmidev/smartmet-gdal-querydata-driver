// ======================================================================
/*!
 * \brief Parse distances.
 *
 * Known formats: kilometer,meter,centimeter,millimeter,mile,nautical_mile,yard,foot,inch
 *  Known inits: km,m,cm,mm,mi,nmi,yd,ft,in
 *
 */
// ======================================================================

#pragma once

#include <string>

namespace Fmi
{
namespace DistanceParser
{
double parse(const std::string& str, const std::string& format);

double parse_kilometer(const std::string& str);
double parse_meter(const std::string& str);
double parse_centimeter(const std::string& str);
double parse_millimeter(const std::string& str);
double parse_mile(const std::string& str);
double parse_nautical_mile(const std::string& str);
double parse_foot(const std::string& str);
double parse_yard(const std::string& str);
double parse_inch(const std::string& str);

}  // namespace DistanceParser
}  // namespace Fmi

// ======================================================================
