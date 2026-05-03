#pragma once

#include <ctime>
#include "Base.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/split_member.hpp>

namespace Fmi
{
    namespace literals
    {
        using namespace date::literals;
    }

    namespace date_time
    {
        enum Month
        {
            Jan = 1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
        };

        struct YMD
        {
            int year;
            unsigned month;
            unsigned day;
        };

        class Date : public Base
        {
        public:
            static constexpr enum Type NOT_A_DATE = NOT_A_DATE_TIME;

            Date();
            Date(const Type& type);
            Date(const date::local_days& date);
            Date(const Date& other);
            Date(int year, unsigned month, unsigned day);
            virtual ~Date();

            Date& operator=(const Date& other);

            static const Date epoch;

            inline bool is_not_a_date() const { return is_not_a_date_time(); }

            int year() const;
            unsigned month() const;
            unsigned day() const;
            YMD year_month_day() const;
            Weekday day_of_week() const;
            int day_of_year() const;
            Date end_of_month() const;
            long modjulian_day() const;
            long julian_day() const;

            /**
             * @return ISO 8601 week number
             */
            int week_number() const;

            std::tm as_tm() const;

            std::string to_simple_string() const override;
            std::string to_iso_string() const override;
            std::string to_iso_extended_string() const override;

            static Date from_time_t(std::time_t time);
            static Date from_tm(const std::tm& tm);
            static Date from_string(const std::string& str);
            static Date from_iso_string(const std::string& str);
            static Date from_iso_extended_string(const std::string& str);

            bool operator==(const Date& other) const;
            bool operator!=(const Date& other) const;
            bool operator<(const Date& other) const;
            bool operator<=(const Date& other) const;
            bool operator>(const Date& other) const;
            bool operator>=(const Date& other) const;

            Date operator+(int num_days) const;
            Date operator-(int num_days) const;
            int operator-(const Date& other) const;

            Date& operator+=(int num_days);
            Date& operator-=(int num_days);

            Date operator ++(int);
            Date& operator++();
            Date operator --(int);
            Date& operator--();

            const date::local_days& get_impl() const { return date; };

            template <typename ArchiveType> void save(ArchiveType& archive, const unsigned int version) const;

            template <typename ArchiveType> void load(ArchiveType& archive, const unsigned int version);

            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            date::local_days date;
        };

        Date date_from_string(const std::string& str);

        inline std::tm to_tm(const Date& date) { return date.as_tm(); }

        struct tm to_tm(const Date& date);
        Date from_tm(const struct tm& tm);

        template <typename ArchiveType>
        void Date::save(ArchiveType& archive, const unsigned int version) const
        {
            const auto date_type = type();
            archive & BOOST_SERIALIZATION_NVP(date_type);
            if (date_type == NORMAL)
            {
                const date::local_days::rep days = date.time_since_epoch().count();
                archive & days;
            }
        }

        template <typename ArchiveType>
        void Date::load(ArchiveType& archive, const unsigned int version)
        {
            Type date_type;
            archive & BOOST_SERIALIZATION_NVP(date_type);
            set_type(date_type);
            if (date_type == NORMAL)
            {
                date::local_days::rep days;
                archive & BOOST_SERIALIZATION_NVP(days);
                date = date::local_days(date::days(days));
            }
        }
    }
}
