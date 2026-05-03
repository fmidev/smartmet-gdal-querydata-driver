#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cstdlib>
#include <limits>
#include <string>
#include <type_traits>

#include "ParserStructures.h"

#define SUPPORT_DATE_TIME_PARSER_GRAMMAR 0

// Adapt structs, so they behave like boost::fusion::vector's

BOOST_FUSION_ADAPT_STRUCT(
    Fmi::date_time::parser::date_members_t,
    (unsigned, year)
    (unsigned, month)
    (unsigned, mday))

BOOST_FUSION_ADAPT_STRUCT(
    Fmi::date_time::parser::seconds_members_t,
    (unsigned, seconds)
    (std::string, frac_sec))

BOOST_FUSION_ADAPT_STRUCT(
    Fmi::date_time::parser::duration_members_t,
    (int, sign)
    (unsigned, hours)
    (unsigned, minutes)
    (std::optional<Fmi::date_time::parser::seconds_members_t>, seconds))

BOOST_FUSION_ADAPT_STRUCT(
    Fmi::date_time::parser::time_zone_offset_members_t,
    (char, sign)
    (unsigned, hours)
    (unsigned, minutes))

#if SUPPORT_DATE_TIME_PARSER_GRAMMAR

BOOST_FUSION_ADAPT_STRUCT(
    Fmi::date_time::parser::date_time_members_t,
    (Fmi::date_time::parser::date_members_t, date)
    (std::optional<Fmi::date_time::parser::duration_members_t>, time)
    (std::optional<Fmi::date_time::parser::time_zone_offset_members_t>, tz_offset))

#endif

namespace Fmi
{
namespace date_time
{

namespace parser
{
    namespace qi = boost::spirit::qi;

    using r_uint12 = qi::uint_parser<unsigned, 10, 1, 2>;
    using r_uint22 = qi::uint_parser<unsigned, 10, 2, 2>;
    using r_uint14 = qi::uint_parser<unsigned, 10, 2, 4>;
    using r_uint44 = qi::uint_parser<unsigned, 10, 4, 4>;

    template <typename Iterator>
    struct Separator : public qi::grammar<Iterator, void()>
    {
        Separator(char separator)
            : Separator::base_type(m_separator)
            , m_separator(separator)
            , is_empty(separator == 0)
        {
        }

        // Do not use this with qi::eps
        Separator(qi::rule<Iterator> separator)
            : Separator::base_type(m_separator)
            , m_separator(separator)
            , is_empty(false)
        {
        }

        qi::rule<Iterator, void()> m_separator;
        const bool is_empty;
    };

    template <typename Iterator>
    class MonthShortNameParser : public qi::grammar<Iterator, unsigned()>
    {
    public:
        MonthShortNameParser()
            : MonthShortNameParser::base_type(m_month)
        {
            using namespace qi;
            m_month = (no_case[lit("Jan")] >> attr(1)) |
                        (no_case[lit("Feb")] >> attr(2)) |
                        (no_case[lit("Mar")] >> attr(3)) |
                        (no_case[lit("Apr")] >> attr(4)) |
                        (no_case[lit("May")] >> attr(5)) |
                        (no_case[lit("Jun")] >> attr(6)) |
                        (no_case[lit("Jul")] >> attr(7)) |
                        (no_case[lit("Aug")] >> attr(8)) |
                        (no_case[lit("Sep")] >> attr(9)) |
                        (no_case[lit("Oct")] >> attr(10)) |
                        (no_case[lit("Nov")] >> attr(11)) |
                        (no_case[lit("Dec")] >> attr(12));
        }

    private:
        qi::rule<Iterator, unsigned()> m_month;
    };

    template <typename Iterator, typename DateSeparatorType>
    struct DateParser : public qi::grammar<Iterator, date_members_t()>
    {
        DateParser(const DateSeparatorType& separator, bool numerical_month)
            : DateParser::base_type(m_date)
            , m_sep(separator)
        {
            using namespace qi;
            if (m_sep.is_empty)
            {
                m_year %= r_uint44()[_pass = (_1 >= 0 && _1 <= 9999)];
                m_mday %= r_uint22()[_pass = (_1 >= 1 && _1 <= 31)];
                if (numerical_month)
                {
                    m_month %= r_uint22()[_pass = (_1 >= 1 && _1 <= 12)];
                }
                else
                {
                    m_month%= m_month_abbrev;
                }
                m_date %= lexeme[m_year >> m_month >> m_mday];
            }
            else
            {
                m_year %= r_uint14()[_pass = (_1 >= 0 && _1 <= 9999)];
                m_mday %= r_uint12()[_pass = (_1 >= 1 && _1 <= 31)];
                if (numerical_month)
                {
                    m_month %= r_uint12()[_pass = (_1 >= 1 && _1 <= 12)];
                }
                else
                {
                    m_month %= m_month_abbrev;
                }
                m_date %= lexeme[
                    m_year >>
                    omit[m_sep] >>
                    m_month >>
                    omit[m_sep] >>
                    m_mday];
            }
        }

    private:
        Separator<Iterator> m_sep;
        MonthShortNameParser<Iterator> m_month_abbrev;
        qi::rule<Iterator, unsigned()> m_year;
        qi::rule<Iterator, unsigned()> m_month;
        qi::rule<Iterator, unsigned()> m_mday;
        qi::rule<Iterator, date_members_t()> m_date;
    };

    /**
     *  Parses ISO 8601 date format (YYYYMMDD)
    */
    template <typename Iterator>
    struct IsoDateParser : public DateParser<Iterator, char>
    {
        IsoDateParser()
            : DateParser<Iterator, char>(0, true)
        {
        }
    };

    /**
     *  Parses ISO 8601 extended date format (YYYY-MM-DD)
    */
    template <typename Iterator>
    struct IsoExtendedDateParser : public DateParser<Iterator, char>
    {
        IsoExtendedDateParser()
            : DateParser<Iterator, char>('-', true)
        {
        }
    };

    template <typename Iterator>
    struct GenericDateParser : public qi::grammar<Iterator, date_members_t()>
    {
        GenericDateParser()
            : GenericDateParser::base_type(m_date)
            , m_iso_date()
            , m_iso_extended_date()
            , m_yyyy_mmm_dd(':', false)
            , m_date(m_iso_extended_date | m_yyyy_mmm_dd | m_iso_date)
        {
        }

    private:
        IsoDateParser<Iterator> m_iso_date;
        IsoExtendedDateParser<Iterator> m_iso_extended_date;
        DateParser <Iterator, char> m_yyyy_mmm_dd;
        qi::rule<Iterator, date_members_t()> m_date;
    };

    template <typename Iterator>
    struct SecondsParser : public qi::grammar<Iterator, seconds_members_t()>
    {
        SecondsParser(bool two_digits_required)
            : SecondsParser::base_type(m_seconds)
        {
            using namespace qi;
            if (two_digits_required) {
                m_full_seconds %= r_uint22()[_pass = (_1 >= 0 && _1 <= 59)];
            } else {
                m_full_seconds %= r_uint12()[_pass = (_1 >= 0 && _1 <= 59)];
            }
            m_frac_seconds %= lexeme[omit['.'] >> *digit];
            m_seconds %= m_full_seconds
                >> (m_frac_seconds | (eps >> attr(std::string{})));
        }
    private:
        qi::rule<Iterator, unsigned()> m_full_seconds;
        qi::rule<Iterator, std::string()> m_frac_seconds;
        qi::rule<Iterator, seconds_members_t()> m_seconds;
    };

    template <typename Iterator, typename DurationSeparatorType>
    struct DurationParser : public qi::grammar<Iterator, duration_members_t()>
    {
        using iterator = std::string::const_iterator;

        /**
         * @brief Construct a new Duration Parser object
         *
         * @param separator separator character between hours, minutes and seconds when not
         *                  0 and absent when 0
        */
        DurationParser(
            char separator = ':',
            bool supports_negative = true,
            const unsigned max_hours = std::numeric_limits<unsigned>::max())

            : DurationParser::base_type(m_duration)
            , m_sep(separator)
            , m_seconds(m_sep.is_empty)
        {
            using namespace qi;

            if (supports_negative)
            {
                m_sign %= ('-' >> attr(-1))
                          | (('+' | eps) >> attr(1));
            }
            else
            {
                m_sign %= attr(1);
            }

            if (m_sep.is_empty)
            {
                m_hours %= r_uint22()[_pass = (_1 >= 0 && _1 <= max_hours)];
                m_minutes %= r_uint22()[_pass = (_1 >= 0 && _1 <= 59)];
                m_duration %= lexeme[m_sign >> m_hours >> m_minutes >> -m_seconds];
            }
            else
            {
                m_hours %= r_uint12()[_pass = (_1 >= 0 && _1 <= max_hours)];
                m_minutes %= r_uint12()[_pass = (_1 >= 0 && _1 <= 59)];
                // Seconds are optional
                m_duration %= lexeme[
                    m_sign
                    >> m_hours
                    >> omit[m_sep]
                    >> m_minutes
                    >> -(omit[m_sep] >> m_seconds)
                ];
            }
        }

    private:
        qi::rule<Iterator, int()> m_sign;
        Separator<Iterator> m_sep;
        qi::rule<Iterator, unsigned()> m_hours;
        qi::rule<Iterator, unsigned()> m_minutes;
        SecondsParser<Iterator> m_seconds;
        qi::rule<Iterator, duration_members_t()> m_duration;
    };

    template <typename Iterator>
    struct GenericDurationParser : public qi::grammar<Iterator, duration_members_t()>
    {
        GenericDurationParser(
            bool supports_negative = true,
            const unsigned max_hours = std::numeric_limits<unsigned>::max())

            : GenericDurationParser::base_type(m_duration)
            , m_iso_duration(0, supports_negative, max_hours)
            , m_iso_extended_duration(':', supports_negative, max_hours)
            , m_duration(m_iso_extended_duration | m_iso_duration)
        {
        }

    private:
        DurationParser<Iterator, char> m_iso_duration;
        DurationParser<Iterator, char> m_iso_extended_duration;
        qi::rule<Iterator, duration_members_t()> m_duration;
    };

    template <typename Iterator>
    struct TimeZoneOffsetParser : public qi::grammar<Iterator, time_zone_offset_members_t()>
    {
        TimeZoneOffsetParser()
            : TimeZoneOffsetParser::base_type(m_offset)
        {
            using namespace qi;
            m_hours %= r_uint22() [_pass = (_1 >= 0 && _1 <= 14)];
            m_opt_sep %= -omit[char(':')];
            m_minutes %= r_uint22() [_pass = (_1 >= 0 && _1 <= 59)];
            m_sign %= (char_('+') | char_('-'));

            m_utc %= char_('Z') >> attr(0) >> attr(0);
            m_given_offset %= m_sign >> m_hours >> m_opt_sep >> m_minutes;

            m_offset %= lexeme[ m_utc | m_given_offset];
        }

    private:
        qi::rule<Iterator, char()> m_sign;
        qi::rule<Iterator, unsigned()> m_hours;
        qi::rule<Iterator, void()> m_opt_sep;
        qi::rule<Iterator, unsigned()> m_minutes;
        qi::rule<Iterator, time_zone_offset_members_t()> m_utc;
        qi::rule<Iterator, time_zone_offset_members_t()> m_given_offset;
        qi::rule<Iterator, time_zone_offset_members_t()> m_offset;
    };

// The following does not seem to work correctlty (tz_offset is never stored
// enen vhen specofied in the input string.
#if 0
    template <typename Iterator>
    struct IsoDateTimeParser : public qi::grammar<Iterator, date_time_members_t()>
    {
        IsoDateTimeParser()
            : IsoDateTimeParser::base_type(m_date_time)
            , m_date(0, true)
            , m_duration(0, 24)
            , m_offset()
        {
            using namespace qi;
            m_date_time %=
                lexeme[m_date >> -(omit[char_('T')] >> m_duration >> -m_offset)];
        }

    private:
        DateParser<Iterator, char> m_date;
        DurationParser<Iterator, char> m_duration;
        TimeZoneOffsetParser<Iterator> m_offset;
        qi::rule<Iterator, date_time_members_t()> m_date_time;
    };

    template <typename Iterator>
    struct IsoExtendedDateTimeParser : public qi::grammar<Iterator, date_time_members_t()>
    {
        IsoExtendedDateTimeParser()
            : IsoExtendedDateTimeParser::base_type(m_date_time)
            , m_date('-', true)
            , m_duration(':', 24)
            , m_offset()
        {
            using namespace qi;
            m_date_time %=
                lexeme[m_date >> -(omit[char_('T')] >> m_duration >> -m_offset)];
        }

    private:
        DateParser<Iterator, char> m_date;
        DurationParser<Iterator, char> m_duration;
        TimeZoneOffsetParser<Iterator> m_offset;
        qi::rule<Iterator, date_time_members_t()> m_date_time;
    };

    template <typename Iterator>
    struct GenericDateTimeParser : public qi::grammar<Iterator, date_time_members_t()>
    {
        GenericDateTimeParser()
            : GenericDateTimeParser::base_type(m_date_time)
            , m_sep_1(+qi::space)
            , m_sep_2('T')
            , m_date_1('-', true)    // ISO extended date
            , m_date_2('-', false)   // YYYY-MMM-DD
            , m_date_3(0, true)      // ISO date
            , m_duration_1(':')
            , m_duration_2(0)
            , m_offset()
            , m_date_time_1(m_date_2 >> -(m_sep_1 >> m_duration_1 >> -m_offset))
            , m_date_time_2(m_date_1 >> -((m_sep_1 | m_sep_2) >> m_duration_1 >>-m_offset))
            , m_date_time_3(m_date_3 >> -(m_sep_2 >> m_duration_2 >> -m_offset))
            , m_date_time(qi::lexeme[m_date_time_1 | m_date_time_2 | m_date_time_3])
        {
        }

    private:

        qi::rule<Iterator, void()> m_sep_1;
        qi::rule<Iterator, void()> m_sep_2;
        DateParser<Iterator, char> m_date_1;
        DateParser<Iterator, char> m_date_2;
        DateParser<Iterator, char> m_date_3;
        DurationParser<Iterator, char> m_duration_1;
        DurationParser<Iterator, char> m_duration_2;
        TimeZoneOffsetParser<Iterator> m_offset;
        qi::rule<Iterator, date_time_members_t()> m_date_time_1;
        qi::rule<Iterator, date_time_members_t()> m_date_time_2;
        qi::rule<Iterator, date_time_members_t()> m_date_time_3;
        qi::rule<Iterator, date_time_members_t()> m_date_time;
    };
#endif // SUPPORT_DATE_TIME_PARSER_GRAMMAR

}  // namespace parser

}  // namespace date_time
}  // namespace Fmi
