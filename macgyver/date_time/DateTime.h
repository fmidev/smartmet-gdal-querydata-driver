#pragma once

#include "Date.h"
#include "TimeDuration.h"
#include <ctime>
#include <optional>
#include <ostream>

namespace Fmi
{
    namespace date_time
    {
        class DateTime : public Base
        {
        public:
            /**
             * @brief Simple wrapper for use as index in std::map
             *
             * The problem with using directly DateTime is that comparisson
             * of one of possible values (NOT_A_DATE_TIM) is not permitted.
             * That causes difficult to debug problems when std::map index is
             * DateTime.
             */
            class Index;

            /**
             * @brief Default constructor.
             *
             * The resulting value is NOT_A_DATE_TIME
             */
            DateTime();

            /**
             * @brief Constructor for a spial values of DateTime
             *
             * Specifying DateTime::NORMAL is not permitted
            */
            DateTime(const Type& type);

            /**
             * @brief Copy constructor
            */
            DateTime(const DateTime& other);

            /**
             * @brief Constructor for a date
             *
             * The time part is set to 00:00:00
            */
            DateTime(const Date& date);

            /**
             * @brief Constructor for a date and time
             *
             * The time part is set to the specified value
             *
             * @param date Date part
             * @param time Time part
            */
            DateTime(const Date& date, const TimeDuration& time);

            /**
             * @brief Constructor for a std::chrono time point
             *
             * Intended mostly for internal use
            */
            DateTime(const detail::time_point_t& time_point);

            virtual ~DateTime();

            DateTime& operator = (const DateTime& other);

            bool operator == (const DateTime& other) const;
            bool operator != (const DateTime& other) const;
            bool operator < (const DateTime& other) const;
            bool operator <= (const DateTime& other) const;
            bool operator > (const DateTime& other) const;
            bool operator >= (const DateTime& other) const;

            DateTime& operator += (const TimeDuration& duration);
            DateTime& operator -= (const TimeDuration& duration);

            DateTime operator + (const TimeDuration& duration) const;
            DateTime operator - (const TimeDuration& duration) const;
            TimeDuration operator - (const DateTime& other) const;

            DateTime operator ++ (int);
            DateTime& operator ++ ();
            DateTime operator -- (int);
            DateTime& operator -- ();

            Date date() const;
            TimeDuration time_of_day() const;

            std::time_t as_time_t() const;
            std::tm as_tm() const;

            std::string to_simple_string() const override;
            std::string to_iso_string() const override;
            std::string to_iso_extended_string() const override;

            static DateTime from_tm(const std::tm& tm);
            static DateTime from_string(const std::string& str);
            static DateTime from_iso_string(const std::string& str);
            static DateTime from_iso_extended_string(const std::string& str);

            static std::optional<DateTime>
            try_parse_iso_string(const std::string& src, bool* have_tz = nullptr);

            static std::optional<DateTime>
            try_parse_iso_extended_string(const std::string& src, bool* have_tz = nullptr);

            static std::optional<DateTime>
            try_parse_string(const std::string& src, bool* have_tz = nullptr);

            static DateTime from_stream(std::istream& is, bool assume_eoi = true);

            detail::time_point_t get_impl() const { return m_time_point; }

            template <typename ArchiveType> void save(ArchiveType& archive, const unsigned int version) const;

            template <typename ArchiveType> void load(ArchiveType& archive, const unsigned int version);

            BOOST_SERIALIZATION_SPLIT_MEMBER()

            static const DateTime epoch;
            static const DateTime min;
            static const DateTime max;
        private:
            detail::time_point_t m_time_point;
        };

        inline std::tm to_tm(const DateTime& dt)
        {
            return dt.as_tm();
        }

        inline DateTime time_from_string(const std::string& str)
        {
            return DateTime::from_string(str);
        }

        inline DateTime time_from_iso_string(const std::string& str)
        {
            return DateTime::from_iso_string(str);
        }

        inline DateTime time_from_iso_extended_string(const std::string& str)
        {
            return DateTime::from_iso_extended_string(str);
        }

        DateTime from_time_t(std::time_t t);

        DateTime try_parse_iso(const std::string& str, bool* is_utc);
        DateTime parse_iso(const std::string& str);

        namespace MicrosecClock
        {
            DateTime universal_time();
            DateTime local_time();
        }

        namespace SecondClock
        {
            DateTime universal_time();
            DateTime local_time();
        }

        template <typename Archive>
        void DateTime::save(Archive& archive, const unsigned int version) const
        {
            Type date_type = type();
            archive & BOOST_SERIALIZATION_NVP(date_type);
            if (date_type == NORMAL)
            {
                const detail::duration_t duration = m_time_point - epoch.m_time_point;
                const int64_t value = duration.count();
                archive & BOOST_SERIALIZATION_NVP(value);
            }
        }

        template <typename Archive>
        void DateTime::load(Archive& archive, const unsigned int version)
        {
            Type date_type;
            archive & BOOST_SERIALIZATION_NVP(date_type);
            set_type(date_type);
            if (date_type == NORMAL)
            {
                int64_t value;
                archive & BOOST_SERIALIZATION_NVP(value);
                const detail::duration_t duration(value);
                m_time_point = epoch.m_time_point + duration;
            }
        }
    }
}
