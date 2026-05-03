#pragma once

#include <ctime>
#include <limits>
#include <string>
#include "Base.h"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>


namespace Fmi
{
    namespace date_time
    {
        class TimeDuration : public Base
        {
        public:
            static constexpr enum Type NOT_A_DURATION = NOT_A_DATE_TIME;

            TimeDuration() = default;
            TimeDuration(Type type) : Base(type) {}
            TimeDuration(const detail::duration_t& duration);
            TimeDuration(const TimeDuration& other) = default;
            TimeDuration(int hours, int minutes, int seconds, int microseconds = 0);
            virtual ~TimeDuration() = default;

            TimeDuration& operator=(const TimeDuration& other) = default;

            inline bool is_not_a_duration() const { return is_not_a_date_time(); }

            int64_t hours() const;
            int64_t minutes() const;
            int64_t seconds() const;
            int64_t fractional_seconds() const;

            int64_t total_days() const;
            int64_t total_hours() const;
            int64_t total_minutes() const;
            int64_t total_seconds() const;
            int64_t total_milliseconds() const;
            int64_t total_microseconds() const;
            inline int64_t total_nanoseconds() const { return 1000L * total_microseconds(); }

            constexpr int64_t ticks_per_second() const { return detail::period_t::den; }

            std::string to_simple_string() const override;
            std::string to_iso_string() const override;
            std::string to_iso_extended_string() const override;

            static TimeDuration from_tm(const std::tm& tm);

            static TimeDuration from_string(
                const std::string& str,
                bool supports_negative = true,
                unsigned max_hours = std::numeric_limits<unsigned>::max());

            static TimeDuration from_iso_string(
                const std::string& str,
                bool supports_negative = true,
                unsigned max_hours = std::numeric_limits<unsigned>::max());

            static TimeDuration from_iso_extended_string(
                const std::string& str,
                bool supports_negative = true,
                unsigned max_hours = std::numeric_limits<unsigned>::max());

            bool operator==(const TimeDuration& other) const;
            bool operator!=(const TimeDuration& other) const;
            bool operator<(const TimeDuration& other) const;
            bool operator<=(const TimeDuration& other) const;
            bool operator>(const TimeDuration& other) const;
            bool operator>=(const TimeDuration& other) const;

            TimeDuration& operator + () { return *this; }
            TimeDuration operator - () const { return TimeDuration(-m_duration);}

            TimeDuration operator+(const TimeDuration& other) const;
            TimeDuration operator-(const TimeDuration& other) const;
            TimeDuration operator*(int64_t factor) const;
            TimeDuration operator/(int64_t factor) const;

            TimeDuration& operator+=(const TimeDuration& other);
            TimeDuration& operator-=(const TimeDuration& other);
            TimeDuration& operator*=(int64_t factor);
            TimeDuration& operator/=(int64_t factor);

            TimeDuration operator++(int);
            TimeDuration& operator++();
            TimeDuration operator--(int);
            TimeDuration& operator--();

            detail::duration_t get_impl() const { return m_duration; }

            /**
             * @brief  Parse time duration from string.
             *
             * @param str  String to parse.
             * @param assume_eoi  If true, allows only whitespace after the duration.
             * @return TimeDuration  Parsed time duration.
            */
            static TimeDuration from_stream(std::istream& src, bool assume_eoi = true);

            template <typename ArchiveType> void save(ArchiveType& archive, const unsigned int version) const;

            template <typename ArchiveType> void load(ArchiveType& archive, const unsigned int version);

            BOOST_SERIALIZATION_SPLIT_MEMBER()
        private:
            void assert_special() const;

        private:
            detail::duration_t m_duration;
        };

        TimeDuration Days(int days);
        TimeDuration Hours(int hours);
        TimeDuration Minutes(int minutes);
        TimeDuration Seconds(int seconds);
        TimeDuration Milliseconds(int milliseconds);
        TimeDuration Microseconds(int microseconds);

        TimeDuration duration_from_string(const std::string& str);

        struct std::tm to_tm(const TimeDuration& td);

        template <typename ArchiveType>
        void TimeDuration::save(ArchiveType& archive, const unsigned int version) const
        {
            const auto duration_type = type();
            archive & BOOST_SERIALIZATION_NVP(duration_type);
            if (duration_type == NORMAL)
            {
                const detail::duration_t::rep cnt = m_duration.count();
                archive & BOOST_SERIALIZATION_NVP(cnt);
            }
        }

        template <typename ArchiveType>
        void TimeDuration::load(ArchiveType& archive, const unsigned int version)
        {
            Type duration_type;
            archive & BOOST_SERIALIZATION_NVP(duration_type);
            set_type(duration_type);
            if (duration_type == NORMAL)
            {
                detail::duration_t::rep cnt;
                archive & BOOST_SERIALIZATION_NVP(cnt);
                m_duration = detail::duration_t(cnt);
            }
        }
    }
}
