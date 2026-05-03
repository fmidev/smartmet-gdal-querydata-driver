// ======================================================================
/*!
 * \brief Replacements for std strings conversions which are locale dependent
 *
 * Locale dependent conversions are slow when using gcc, since a global mutex
 * is invoked when constructing an output stream.
 *
 * Types such as long long are intentionally omitted.
 */
// ======================================================================

#pragma once
#include "LocalDateTime.h"
#include <optional>
#include <string>

namespace Fmi
{
std::string to_string(bool value);
std::string to_string(int value);
std::string to_string(long value);
std::string to_string(unsigned int value);
std::string to_string(unsigned long value);
std::string to_string(float value);
std::string to_string(double value);
inline std::string to_string(const char* value) { return std::string(value); }
inline std::string to_string(const unsigned char* value) { return std::string(reinterpret_cast<const char*>(value)); }
#if defined(_WIN32) || defined(WIN32)
std::string to_string(size_t value);
std::string to_string(time_t value);
#endif

std::string to_string(const char* fmt, int value);
std::string to_string(const char* fmt, long value);
std::string to_string(const char* fmt, unsigned int value);
std::string to_string(const char* fmt, unsigned long value);
std::string to_string(const char* fmt, float value);
std::string to_string(const char* fmt, double value);

int stoi(const std::string& str);
long stol(const std::string& str);
unsigned long stoul(const std::string& str);
float stof(const std::string& str);
double stod(const std::string& str);

std::optional<int> stoi_opt(const std::string& str);
std::optional<long> stol_opt(const std::string& str);
std::optional<unsigned long> stoul_opt(const std::string& str);
std::optional<float> stof_opt(const std::string& str);
std::optional<double> stod_opt(const std::string& str);

std::size_t stosz(const std::string& str);  // B,K,M,G,T,P suffixes allowed

std::string to_iso_string(const std::time_t time);
std::string to_iso_string(const TimeDuration& duration);
std::string to_iso_string(const Fmi::Date& date);
std::string to_iso_string(const DateTime& time);
std::string to_iso_string(const LocalDateTime& time);

std::string to_iso_extended_string(const TimeDuration& duration);
std::string to_iso_extended_string(const Fmi::Date& date);
std::string to_iso_extended_string(const DateTime& time);
std::string to_iso_extended_string(const LocalDateTime& time);

std::string to_simple_string(const TimeDuration& duration);
std::string to_simple_string(const Fmi::Date& date);
std::string to_simple_string(const DateTime& time);
std::string to_simple_string(const LocalDateTime& time);

std::string to_http_string(const DateTime& time);
std::string to_timestamp_string(const DateTime& time);

void ascii_tolower(std::string& input);
void ascii_toupper(std::string& input);

std::string ascii_tolower_copy(std::string input);
std::string ascii_toupper_copy(std::string input);

bool looks_unsigned_int(const std::string& value);
bool looks_signed_int(const std::string& value);

// Boost trim uses the locale and is hence slow
void trim(std::string& value);
std::string trim_copy(const std::string& value);

// Plain xmlescape
std::string xmlescape(const std::string& input);

// Version which does nothing for input which is already escaped
std::string safexmlescape(const std::string& input);

}  // namespace Fmi
