#pragma once

//#define MYDEBUG

#ifdef MYDEBUG
#define BOOST_SPIRIT_DEBUG
#endif

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

#include <string>

namespace Fmi
{
namespace TimeParser
{
enum ParserId
{

  SQL,
  ISO,
  EPOCH,
  OFFSET,
  FMI

};

using OptionalInt = std::optional<unsigned int>;
using OptionalString = std::optional<std::string>;
using OptionalChar = std::optional<char>;

struct TimeZoneOffset
{
  bool present;

  char sign;

  unsigned int hours;

  unsigned int minutes;
};

using UnixTime = unsigned int;

struct TimeOffset
{
  char sign;

  unsigned int value;

  OptionalChar unit;
};

struct TimeStamp
{
  unsigned short year;

  unsigned short month;

  unsigned short day;

  OptionalInt hour;

  OptionalInt minute;

  OptionalInt second;

  TimeZoneOffset tz;
};
}  // namespace TimeParser
}  // namespace Fmi

// Adapt structs, so they behave like boost::fusion::vector's

BOOST_FUSION_ADAPT_STRUCT(Fmi::TimeParser::TimeZoneOffset,
                          (char, sign)(unsigned int, hours)(unsigned int, minutes)(bool, present))

BOOST_FUSION_ADAPT_STRUCT(Fmi::TimeParser::TimeOffset,
                          (char, sign)(unsigned int, value)(Fmi::TimeParser::OptionalChar, unit))

BOOST_FUSION_ADAPT_STRUCT(Fmi::TimeParser::TimeStamp,
                          (unsigned short,
                           year)(unsigned short,
                                 month)(unsigned short,
                                        day)(Fmi::TimeParser::OptionalInt,
                                             hour)(Fmi::TimeParser::OptionalInt,
                                                   minute)(Fmi::TimeParser::OptionalInt,
                                                           second)(Fmi::TimeParser::TimeZoneOffset,
                                                                   tz))

namespace Fmi
{
namespace TimeParser
{
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;

//########################################################
// Timezone parser
//########################################################

template <typename Iterator>
struct TimeZoneParser : qi::grammar<Iterator, TimeZoneOffset()>
{
  using TwoDigitNumber = qi::uint_parser<unsigned int, 10, 2, 2>;  // Radix 10, exactly 2 digits

  TimeZoneParser() : TimeZoneParser::base_type(tz_offset)
  {
    tz_utc = qi::lit('Z') >> qi::attr('+') >> qi::attr(0) >> qi::attr(0) >> qi::attr(true);

    tz_hhmm =
      (qi::char_("+") | qi::char_("-")) >>
      two_digits >>
      (two_digits | ((qi::lit(':') >> two_digits) | qi::attr(0))) >>  // sign with hours followed by optional colon and minutes
      qi::attr(true);

    tz_missing = qi::eps >> qi::attr('+') >> qi::attr(0) >> qi::attr(0) >> qi::attr(false);

    tz_offset = tz_utc | tz_hhmm | tz_missing;

#ifdef MYDEBUG
    BOOST_SPIRIT_DEBUG_NODE(two_digits);
    BOOST_SPIRIT_DEBUG_NODE(tz_offset);
#endif
  }

  qi::rule<Iterator, TimeZoneOffset()> tz_utc;
  qi::rule<Iterator, TimeZoneOffset()> tz_hhmm;
  qi::rule<Iterator, TimeZoneOffset()> tz_missing;

  qi::rule<Iterator, TimeZoneOffset()> tz_offset;
  TwoDigitNumber two_digits;
};

//########################################################
// SQL-style timestring parser
//########################################################

template <typename Iterator>
struct SQLParser : qi::grammar<Iterator, TimeStamp()>
{
  SQLParser() : SQLParser::base_type(fulltime)
  {
    using namespace qi;
    number = uint_;
    month %= uint_parser<unsigned int, 10, 1, 2>()[_pass = (_val >= 1U && _val <= 12U)];
    mday %= uint_parser<unsigned int, 10, 1, 2>()[_pass = (_val >= 1U && _val <= 31U)];
    hour %= uint_parser<unsigned int, 10, 1, 2>()[_pass = (_val >= 0U && _val <= 23U)];
    minsec %= uint_parser<unsigned int, 10, 1, 2>()[_pass = (_val >= 0U && _val <= 59U)];

    fulltime = number >> '-'                                // Year
               >> month                                     // Month
               >> '-' >> mday                               // Day
               >> qi::omit[*qi::ascii::blank]               // Any number of blanks
               >> -hour                                     // Optional hour
               >> -(':' >> minsec)                          // Optional minute
               >> -(':' >> minsec >>                        // Optional second
                    -('.' >> qi::omit[*qi::ascii::digit]))  // Skip seconds part if present
               >> qi::eoi;                                  // End-of-input

#ifdef MYDEBUG
    BOOST_SPIRIT_DEBUG_NODE(number);
    BOOST_SPIRIT_DEBUG_NODE(fulltime);
#endif
  }

  qi::rule<Iterator, unsigned int()> number;
  qi::rule<Iterator, unsigned int()> month;
  qi::rule<Iterator, unsigned int()> mday;
  qi::rule<Iterator, unsigned int()> hour;
  qi::rule<Iterator, unsigned int()> minsec;
  qi::rule<Iterator, TimeStamp()> fulltime;
};

//########################################################
// FMI-style timestring parser
// YYYYMMDDHHmm - all mandatory
//########################################################

template <typename Iterator>
struct FMIParser : qi::grammar<Iterator, TimeStamp()>
{
  using FourDigitNumber = qi::uint_parser<unsigned int, 10, 4, 4>;  // Radix 10, exactly 4 digits
  using TwoDigitNumber = qi::uint_parser<unsigned int, 10, 2, 2>;   // Radix 10, exactly 2 digits

  FMIParser() : FMIParser::base_type(fmitime)
  {
    fmitime = four_digits     // Year
              >> two_digits   // Month
              >> two_digits   // Day
              >> two_digits   // Hhour
              >> two_digits   // Minute
              >> -two_digits  // Second
              >> offset       // We support timezone-offsets, just because
              >> qi::eoi;     // End-of-input

#ifdef MYDEBUG
    BOOST_SPIRIT_DEBUG_NODE(fmitime);
#endif
  }

  FourDigitNumber four_digits;
  TwoDigitNumber two_digits;

  TimeZoneParser<Iterator> offset;

  qi::rule<Iterator, TimeStamp()> fmitime;
};

//########################################################
// Epoch parser
//########################################################

using EpochParser = qi::uint_parser<UnixTime, 10, 5, 11>;  // Radix 10, min digits 5, max digits 11

//########################################################
// Offset-style timestring parser
//########################################################

template <typename Iterator>
struct OffsetParser : qi::grammar<Iterator, TimeOffset()>
{
  OffsetParser() : OffsetParser::base_type(complete_offset)
  {
    number = qi::uint_;  // A simple integer

    optional_character = -qi::char_;  // Optional single character

    offset = (qi::char_('+') | qi::char_('-'))  // Preceeding plus or minus sign
             >> number                          // an unsigned number
             >> optional_character              // optional character for unit
             >> qi::eoi;                        // end-of-input

    zero_offset = qi::lit("0") >> qi::eoi >> qi::attr('+') >> qi::attr(0) >>
                  qi::attr('m');  // Special case when input is plain 0
    zero_unit_offset = qi::lit("0") >> qi::attr('+') >> qi::attr(0) >> optional_character >>
                       qi::eoi;  // Special case when input is plain 0 with units

    complete_offset = zero_offset | zero_unit_offset | offset;

#ifdef MYDEBUG
    BOOST_SPIRIT_DEBUG_NODE(number);
    BOOST_SPIRIT_DEBUG_NODE(optional_character);
    BOOST_SPIRIT_DEBUG_NODE(offset);
    BOOST_SPIRIT_DEBUG_NODE(zero_offset);
    BOOST_SPIRIT_DEBUG_NODE(complete_offset);
#endif
  }

  qi::rule<Iterator, unsigned int()> number;
  qi::rule<Iterator, OptionalChar()> optional_character;
  qi::rule<Iterator, TimeOffset()> zero_offset;
  qi::rule<Iterator, TimeOffset()> zero_unit_offset;
  qi::rule<Iterator, TimeOffset()> offset;
  qi::rule<Iterator, TimeOffset()> complete_offset;
};

//########################################################
// ISO-style timestring parser
//
// This parses time strings in the form of:
// YYYY(sep)MM(sep)DDTHH(sep)MMSS.sss<timezone>
//
// Separator T is mandatory if sub-daily precision is
// required
//
// Fractional seconds are parsed but ignored due to being irrelevant
//########################################################

template <typename Iterator>
struct ISOParser : qi::grammar<Iterator, TimeStamp()>
{
  ISOParser() : ISOParser::base_type(isostamp)
  {
    optional_dash = qi::omit[-qi::lit('-')];

    optional_colon = qi::omit[-qi::lit(':')];

    optional_period = qi::omit[-qi::lit('.')];

    date_time_separator = qi::omit[qi::lit('T')];

    year = (uint14 >> &qi::omit[qi::lit('-')]) | uint4;

    month = (uint12 >> &qi::omit[qi::lit('-')]) | uint2;

    mday = (uint12 >> &qi::omit[(qi::lit('T') | qi::lit('Z') | qi::eoi)]) | uint2;

    hmin = (uint12 >> &qi::omit[(qi::lit(':') | qi::lit('Z') | qi::eoi)]) | uint2;

    // Possible ':' before number is included to avoid accepting 1 digit number without preceding
    // ':'
    second =
        (qi::omit[qi::lit(':')] >> uint12 >> &qi::omit[(qi::lit('.') | qi::lit('Z') | qi::eoi)]) |
        (optional_colon >> uint2);

    isostamp_ext = year >> optional_dash >> month >> optional_dash >> mday >> date_time_separator
                        >> -hmin >> optional_colon >> -hmin >> -second >> optional_period >> -qi::omit[uint3]
                        >> tz_parser >> qi::eoi;

    // Support separately format YYYYMMDD[T]HHMM[SS] <tz>
    isostamp_basic = uint4 >> uint2 >> uint2 >> qi::omit[-date_time_separator]
                           >> uint2 >> uint2 >> -uint2 >> tz_parser >> qi::eoi;

    isostamp = isostamp_basic | isostamp_ext;

#ifdef MYDEBUG
    BOOST_SPIRIT_DEBUG_NODE(dash);
    BOOST_SPIRIT_DEBUG_NODE(colon);
    BOOST_SPIRIT_DEBUG_NODE(isostamp);
#endif
  }

  qi::uint_parser<unsigned int, 10, 4, 4> uint4;
  qi::uint_parser<unsigned int, 10, 3, 3> uint3;
  qi::uint_parser<unsigned int, 10, 2, 2> uint2;
  qi::uint_parser<unsigned int, 10, 1, 2> uint12;
  qi::uint_parser<unsigned int, 10, 1, 4> uint14;

  qi::rule<Iterator, unsigned short()> year;
  qi::rule<Iterator, unsigned short()> month;
  qi::rule<Iterator, unsigned short()> mday;
  qi::rule<Iterator, unsigned short()> hmin;
  qi::rule<Iterator, unsigned short()> second;

  qi::rule<Iterator, void()> optional_dash;
  qi::rule<Iterator, void()> optional_colon;
  qi::rule<Iterator, void()> optional_period;
  qi::rule<Iterator, void()> date_time_separator;
  TimeZoneParser<Iterator> tz_parser;

  qi::rule<Iterator, TimeStamp()> isostamp_ext;
  qi::rule<Iterator, TimeStamp()> isostamp_basic;

  qi::rule<Iterator, TimeStamp()> isostamp;
};
}  // namespace TimeParser
}  // namespace Fmi
