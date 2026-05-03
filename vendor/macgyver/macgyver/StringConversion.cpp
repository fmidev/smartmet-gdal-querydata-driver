#include "StringConversion.h"
#include "Exception.h"
#include "NumericCast.h"
#include <boost/spirit/include/qi.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <array>
#include <cmath>
#include <stdexcept>

namespace
{
const char* weekdays = "SunMonTueWedThuFriSatSun";
const char* months = "   JanFebMarAprMayJunJulAugSepOctNovDec";

const std::array<char, 201> digits{
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899"};

}  // namespace

namespace Fmi
{
// ----------------------------------------------------------------------
/*
 * Convert numbers to strings without using the global locale.
 * The format specifiers are the same as in the standard, except
 * that we use %g instead of %g to avoid unnecessary trailing zeros.
 */
// ----------------------------------------------------------------------

std::string to_string(bool value)
{
  return value ? "1" : "0";
}

#if defined(_WIN32) || defined(WIN32)

std::string to_string(size_t value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(time_t value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

std::string to_string(float value)
{
  try
  {
    return fmt::sprintf("%g", value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(double value)
{
  try
  {
    return fmt::sprintf("%g", value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, int value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, long value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, unsigned int value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, unsigned long value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, float value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(const char* fmt, double value)
{
  try
  {
    return fmt::sprintf(fmt, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(int value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(unsigned int value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(long value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_string(unsigned long value)
{
  try
  {
    return fmt::format_int(value).str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * Convert strings to numbers.
 *
 * Throws:
 *
 *  std::invalid_argument if no conversion could be performed
 *  std::bad_cast if the converted value would fall out of
 *  the range of the result type or if the underlying function.
 */
// ----------------------------------------------------------------------

std::optional<int> stoi_opt(const std::string& str)
{
  try
  {
    long result;
    auto begin = str.cbegin();
    auto end = str.cend();
    if (boost::spirit::qi::parse(begin, end, boost::spirit::qi::long_, result))
      if (begin == end)
        return Fmi::numeric_cast<int>(result);
    return {};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

int stoi(const std::string& str)
{
  try
  {
    auto result = stoi_opt(str);
    if (result)
      return *result;

    throw Fmi::Exception(BCP, "Fmi::stoi failed to convert '" + str + "' to integer");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::optional<long> stol_opt(const std::string& str)
{
  try
  {
    long result;
    auto begin = str.cbegin();
    auto end = str.cend();
    if (boost::spirit::qi::parse(begin, end, boost::spirit::qi::long_, result))
      if (begin == end)
        return result;
    return {};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

long stol(const std::string& str)
{
  try
  {
    auto result = stol_opt(str);
    if (result)
      return *result;

    throw Fmi::Exception(BCP, "Fmi::stol failed to convert '" + str + "' to long");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::optional<unsigned long> stoul_opt(const std::string& str)
{
  try
  {
    unsigned long result;
    auto begin = str.cbegin();
    auto end = str.cend();
    if (boost::spirit::qi::parse(begin, end, boost::spirit::qi::ulong_, result))
      if (begin == end)
        return result;
    return {};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned long stoul(const std::string& str)
{
  try
  {
    auto result = stoul_opt(str);
    if (result)
      return *result;

    throw Fmi::Exception(BCP, "Fmi::stoul failed to convert '" + str + "' to unsigned long");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::optional<float> stof_opt(const std::string& str)
{
  try
  {
    // We parse as double because of this:
    // http://stackoverflow.com/questions/17391348/boost-spirit-floating-number-parser-precision

    double result;
    auto begin = str.cbegin();
    auto end = str.cend();
    if (boost::spirit::qi::parse(begin, end, boost::spirit::qi::double_, result))
    {
      if (begin == end)
      {
        if (std::isfinite(result))
          return Fmi::numeric_cast<float>(result);

        throw Fmi::Exception(BCP, "Infinite numbers are not allowed: '" + str + "' in Fmi::stof");
      }
    }
    return {};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float stof(const std::string& str)
{
  try
  {
    auto result = stof_opt(str);
    if (result)
      return *result;
    throw Fmi::Exception(BCP, "Fmi::stof failed to convert '" + str + "' to float");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::optional<double> stod_opt(const std::string& str)
{
  try
  {
    double result;
    auto begin = str.cbegin();
    auto end = str.cend();
    if (boost::spirit::qi::parse(begin, end, boost::spirit::qi::double_, result))
    {
      if (begin == end)
      {
        if (std::isfinite(result))
          return result;

        throw Fmi::Exception(BCP, "Infinite numbers are not allowed: '" + str + "' in Fmi::stod");
      }
    }
    return {};
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double stod(const std::string& str)
{
  try
  {
    auto result = stod_opt(str);
    if (result)
      return *result;
    throw Fmi::Exception(BCP, "Fmi::stod failed to convert '" + str + "' to double");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * Convert dates and times to strings. The code mimics the respective
 * code in boost, but avoids using the locale. Code heavily influenced by {fmt}.
 * Note that it might be tempting to make inlined functions for some of the
 * common code segments. However, g++ might not actually inline the code
 * and the result has been observed to be 16 times slower.
 */
// ----------------------------------------------------------------------

// Convert a duration to iso string of form HHMMSS[,fffffff]
std::string to_iso_string(const TimeDuration& duration)
{
  try
  {
    if (duration.is_special())
      return Fmi::date_time::to_iso_string(duration);

    const detail::hh_mm_ss hms(duration.get_impl());
    bool is_negative = duration.get_impl().count() < 0;
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();
    int sub_sec = hms.subseconds().count();

    std::array<char, 8> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    if (is_negative)
      *--ptr = '-';
    std::string ret(ptr);

    if (sub_sec != 0)
    {
      std::string fmt = ",%0" + Fmi::to_string(Fmi::date_time::num_fractional_digits) + "ld";
      ret += fmt::sprintf(fmt, sub_sec);
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert a duration to string of form HH:MM:SS[,fffffff]
std::string to_simple_string(const TimeDuration& duration)
{
  try
  {
    if (duration.is_special())
      return Fmi::date_time::to_simple_string(duration);

    const detail::hh_mm_ss hms(duration.get_impl());
    bool is_negative = duration.get_impl().count() < 0;
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();
    int sub_sec = hms.subseconds().count();

    std::array<char, 40> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';

    while (true)
    {
      auto h = hours % 100;

      index = h * 2;
      *--ptr = digits[index + 1];
      *--ptr = digits[index];

      hours /= 100;
      if (hours == 0)
        break;
    }

    // Avoid 0123:mm:ss
    if (duration.hours() >= 100 && *ptr == '0')
      ++ptr;

    if (is_negative)
      *--ptr = '-';
    std::string ret(ptr);

    if (sub_sec != 0)
    {
      std::string fmt = ",%0" + Fmi::to_string(Fmi::date_time::num_fractional_digits) + "ld";
      ret += fmt::sprintf(fmt, sub_sec);
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_iso_extended_string(const TimeDuration& duration)
{
  try
  {
    return Fmi::to_simple_string(duration);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert date to form YYYY-mmm-DD string where mmm 3 char month name
std::string to_simple_string(const Fmi::Date& date)
{
  try
  {
    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    std::array<char, 12> buffer;
    char* ptr = buffer.data() + buffer.size();
    unsigned index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = month * 3;
    *--ptr = months[index + 2];
    *--ptr = months[index + 1];
    *--ptr = months[index];
    *--ptr = '-';
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert date to form YYYYMMDD
std::string to_iso_string(const Fmi::Date& date)
{
  try
  {
    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    std::array<char, 9> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = month * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert date to form YYYY-MM-DD
std::string to_iso_extended_string(const Fmi::Date& date)
{
  try
  {
    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    std::array<char, 11> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = month * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to form YYYYMMDDTHHMMSS,fffffffff where T is the date-time separator
std::string to_iso_string(const Fmi::DateTime& time)
{
  try
  {
    if (time.is_special())
      return Fmi::date_time::to_iso_string(time);

    const date::year_month_day ymd(time.date().get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    const detail::duration_t duration = time.time_of_day().get_impl();
    const detail::hh_mm_ss hms(duration);
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();
    int sub_sec = hms.subseconds().count();

    std::array<char, 16> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = 'T';
    index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = month * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = year % 100 * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = year / 100 * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    std::string ret(ptr);

    if (sub_sec != 0)
    {
      std::string fmt = ",%0" + Fmi::to_string(Fmi::date_time::num_fractional_digits) + "ld";
      ret += fmt::sprintf(fmt, sub_sec);
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to form YYYYMMDDTHHMMSS where T is the date-time separator
std::string to_iso_string(const std::time_t time)
{
  try
  {
    struct tm tt;
    gmtime_r(&time, &tt);

    std::array<char, 16> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = tt.tm_sec * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = tt.tm_min * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = tt.tm_hour * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = 'T';
    index = tt.tm_mday * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (tt.tm_mon + 1) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    auto yy = tt.tm_year + 1900;
    index = (yy % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (yy / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];

    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to form YYYYMMDDTHHMM
std::string to_timestamp_string(const DateTime& time)
{
  try
  {
    if (time.is_special())
      return Fmi::date_time::to_iso_string(time);

    const auto& date = time.date();
    const auto& duration = time.time_of_day();

    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    const detail::hh_mm_ss hms(duration.get_impl());
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();

    std::array<char, 13> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    auto index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = month * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to form YYYY-MM-DDTHH:MM:SS,fffffffff where T is the date-time separator
std::string to_iso_extended_string(const DateTime& time)
{
  try
  {
    if (time.is_special())
      return Fmi::date_time::to_iso_extended_string(time);

    const auto& date = time.date();
    const auto& duration = time.time_of_day();

    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    const detail::hh_mm_ss hms(duration.get_impl());
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();
    int sub_sec = hms.subseconds().count();

    std::array<char, 20> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = 'T';
    index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = month * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    std::string ret(ptr);

    if (sub_sec != 0)
    {
      std::string fmt = ",%0" + Fmi::to_string(Fmi::date_time::num_fractional_digits) + "ld";
      ret += fmt::sprintf(fmt, sub_sec);
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_iso_string(const LocalDateTime& time)
{
  try
  {
    std::string result;
    result += to_iso_string(time.local_time());
    const auto offset = time.offset().total_seconds();
    if (offset == 0)
    {
      result += 'Z';
      return result;
    }
    result += offset < 0 ? '-' : '+';
    const auto hours = std::abs(offset) / 3600;
    const auto minutes = (std::abs(offset) % 3600) / 60;
    result += fmt::sprintf("%02d%02d", hours, minutes);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
std::string to_iso_extended_string(const LocalDateTime& time)
{
  try
  {
    std::string result;
    result += to_iso_extended_string(time.local_time());
    const auto offset = time.offset().total_seconds();
    if (offset == 0)
    {
      result += 'Z';
      return result;
    }

    result += offset < 0 ? '-' : '+';
    const auto hours = std::abs(offset) / 3600;
    const auto minutes = (std::abs(offset) % 3600) / 60;
    result += fmt::sprintf("%02d:%02d", hours, minutes);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string to_simple_string(const LocalDateTime& time)
{
  try
  {
    std::string result;
    result += to_simple_string(time.local_time());
    const auto offset = time.offset().total_seconds();
    if (offset == 0)
    {
      result += 'Z';
      return result;
    }
     result += offset < 0 ? '-' : '+';
    const auto hours = std::abs(offset) / 3600;
    const auto minutes = (std::abs(offset) % 3600) / 60;
    result += fmt::sprintf("%02d%02d", hours, minutes);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to form YYYY-mmm-DD HH:MM:SS.fffffffff string where mmm 3 char month name
std::string to_simple_string(const Fmi::date_time::DateTime& time)
{
  try
  {
    if (time.is_special())
      return Fmi::date_time::to_simple_string(time);

    const auto& date = time.date();
    const auto& duration = time.time_of_day();

    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    const detail::hh_mm_ss hms(duration.get_impl());
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();
    int sub_sec = hms.subseconds().count();

    std::array<char, 30> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ' ';
    index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = '-';
    index = month * 3;
    *--ptr = months[index + 2];
    *--ptr = months[index + 1];
    *--ptr = months[index];
    *--ptr = '-';
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];

    std::string ret(ptr);

    if (sub_sec != 0)
    {
      std::string fmt = ",%0" + Fmi::to_string(Fmi::date_time::num_fractional_digits) + "ld";
      ret += fmt::sprintf(fmt, sub_sec);
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert a valid time to form "Fri, 27 Jul 2018 11:26:04 GMT" suitable for HTTP responses
std::string to_http_string(const Fmi::date_time::DateTime& time)
{
  try
  {
    if (time.is_special())
      throw Fmi::Exception(
          BCP, "Unable to format special DateTime objects for HTTP responses");

    const auto& date = time.date();
    const auto& duration = time.time_of_day();

    const date::year_month_day ymd(date.get_impl());
    const int year = int(ymd.year());
    const unsigned month = unsigned(ymd.month());
    const unsigned day = unsigned(ymd.day());

    const detail::hh_mm_ss hms(duration.get_impl());
    int hours = hms.hours().count();
    int minutes = hms.minutes().count();
    int seconds = hms.seconds().count();

    std::array<char, 30> buffer;
    char* ptr = buffer.data() + buffer.size();
    *--ptr = '\0';
    *--ptr = 'T';
    *--ptr = 'M';
    *--ptr = 'G';
    *--ptr = ' ';
    unsigned index = seconds * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = minutes * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ':';
    index = hours * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ' ';
    index = (year % 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    index = (year / 100) * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ' ';
    index = month * 3;
    *--ptr = months[index + 2];
    *--ptr = months[index + 1];
    *--ptr = months[index];
    *--ptr = ' ';
    index = day * 2;
    *--ptr = digits[index + 1];
    *--ptr = digits[index];
    *--ptr = ' ';
    *--ptr = ',';
    index = date.day_of_week().iso_encoding() * 3;
    *--ptr = weekdays[index + 2];
    *--ptr = weekdays[index + 1];
    *--ptr = weekdays[index];
    return std::string(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to lower case with ASCII input
void ascii_tolower(std::string& input)
{
  try
  {
    for (auto& chr : input)
    {
      if (chr > 64 && chr < 91)
        chr += 32;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Convert to upper case with ASCII input
void ascii_toupper(std::string& input)
{
  try
  {
    for (auto& chr : input)
    {
      if (chr > 96 && chr < 123)
        chr -= 32;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ASCII lower case with copy
std::string ascii_tolower_copy(std::string input)
{
  try
  {
    ascii_tolower(input);
    return input;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ASCII upper case with copy
std::string ascii_toupper_copy(std::string input)
{
  try
  {
    ascii_toupper(input);
    return input;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool looks_unsigned_int(const std::string& value)
{
  try
  {
    if (value.empty())
      return false;

    for (const auto chr : value)
      if (chr < '0' || chr > '9')
        return false;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool looks_signed_int(const std::string& value)
{
  try
  {
    if (value.empty())
      return false;

    std::size_t i = 0;
    if (value[i] == '+' || value[i] == '-')
    {
      if (value.size() == 1)
        return false;
      ++i;
    }
    const auto sz = value.size();
    for (; i < sz; ++i)
    {
      const auto val = value[i];
      if (val < '0' || val > '9')
        return false;
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// In place trim
void trim(std::string& value)
{
  try
  {
    const char* spaces = " \t\n\v\f\r";
    value.erase(value.find_last_not_of(spaces) + 1);
    value.erase(0, value.find_first_not_of(spaces));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Trim to new copy
std::string trim_copy(const std::string& value)
{
  try
  {
    auto tmp = value;
    trim(tmp);
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Escape for HTML/SVG/XML
std::string xmlescape(const std::string& input)
{
  try
  {
    std::string output;
    output.reserve(input.size() + 20);  // 20 is just a guess to avoid one resize

    // https://stackoverflow.com/questions/5665231/most-efficient-way-to-escape-xml-html-in-c-string

    for (size_t pos = 0; pos != input.size(); ++pos)
    {
      switch (input[pos])
      {
        case '&':
          output += "&amp;";
          break;
        case '\"':
          output += "&quot;";
          break;
        case '\'':
          output += "&apos;";
          break;
        case '<':
          output += "&lt;";
          break;
        case '>':
          output += "&gt;";
          break;
        default:
          if (input[pos] < 32 || input[pos] > 126)
          {
            // Numeric character reference for non-printable ASCII characters
            output += "&#";
            output += Fmi::to_string(static_cast<int>(input[pos]));
            output += ";";
          }
          else
            output += input[pos];
          break;
      }
    }
    return output;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Do not escape parts which are already escaped
std::string safexmlescape(const std::string& input)
{
  try
  {
    std::string output;
    output.reserve(input.size() + 20);  // 20 is just a guess to avoid one resize

    for (size_t pos = 0; pos != input.size(); ++pos)
    {
      switch (input[pos])
      {
        case '\"':
          output += "&quot;";
          break;
        case '\'':
          output += "&apos;";
          break;
        case '<':
          output += "&lt;";
          break;
        case '>':
          output += "&gt;";
          break;
        case '&':
        {
          auto endpos = input.find_first_of("&;\"'<>", pos + 1);
          if (endpos != std::string::npos && endpos - pos > 1 && input[endpos] == ';')
          {
            // If we encountered an already encoded segment such as "&lt"; or "&32"; output it as is
            auto entity = input.substr(pos, endpos - pos + 1);
            output += entity;
            pos += entity.size() - 1;
          }
          else
            output += "&amp;";
          break;
        }
        default:
          output += input[pos];
          break;
      }
    }
    return output;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::size_t stosz(const std::string& str)
{
  try
  {
    if (str.size() < 2)
      throw Fmi::Exception(BCP, "Too few letters in size string");

    auto unit = str.back();
    auto snum = str.substr(0, str.size() - 1);
    auto base = Fmi::stoul(snum);

    switch (unit)
    {
      case 'B':
        return base;
      case 'K':
        return 1024UL * base;
      case 'M':
        return 1024UL * 1024UL * base;
      case 'G':
        return 1024UL * 1024UL * 1024UL * base;
      case 'T':
        return 1024UL * 1024UL * 1024UL * 1024UL * base;
      case 'P':
        return 1024UL * 1024UL * 1024UL * 1024UL * 1024UL * base;
      default:
        throw Fmi::Exception(BCP, "Unknown size unit for number of bytes");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Failed to parse a string defining a size in bytes")
        .addParameter("string", str);
  }
}

}  // namespace Fmi
