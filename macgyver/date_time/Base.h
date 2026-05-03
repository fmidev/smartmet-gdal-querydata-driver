#pragma once

#include <chrono>
#include <ostream>
#include <string>

// Specify whether use std::chrono only (instead of Date library)
// in case when C++20 is being used and C++ standard library is complete enough
// to support time zone, zoned_time etc.
//
// Does not do anything if C++20 is not being used or C++ standard library
// is not complete enough.
//
// Note: not ready yet - so just disable for now
#define FMI_ENABLE_STD_CHRONO_ONLY 0

#if FMI_ENABLE_STD_CHRONO_ONLY && __cplusplus >= 202002L && defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
// Time zone, zoned_time etc is supported in libstdc++ => use it

#define FMI_CALENDAR_USES_STD_CHRONO 1
namespace date = std::chrono;

#else
// Use date library

#ifdef _WIN32
#define USE_OS_TZDB 0
#else
#define USE_OS_TZDB 1
#endif

// Time zone, zoned_time etc is NOT supported in libstdc++ or support is incomplete
//      => use date library
#include "date/date.h"
#include "date/tz.h"

#endif

namespace Fmi
{
    namespace detail
    {

        using period_t = std::micro;
        using duration_t = std::chrono::duration<int64_t, std::micro>;
        using time_point_t = std::chrono::time_point<date::local_t, duration_t>;
        using sys_time_t = std::chrono::time_point<std::chrono::system_clock, duration_t>;
        using zoned_time_t = date::zoned_time<duration_t>;
        using microsec_t = std::chrono::duration<int64_t, std::micro>;
        using millisec_t = std::chrono::duration<int64_t, std::milli>;
        using seconds_t = std::chrono::duration<int64_t, std::ratio<1, 1> >;
        using minutes_t = std::chrono::duration<int64_t, std::ratio<60, 1> >;
        using hours_t = std::chrono::duration<int64_t, std::ratio<3600, 1> >;
        using days_t = std::chrono::duration<int64_t, std::ratio<86400, 1> >;
        using day_t = date::day;
        using month_t = date::month;
        using year_t = date::year;
        using ymd_t = date::year_month_day;

        using hh_mm_ss = date::hh_mm_ss<duration_t>;

        constexpr int periods_per_sec = period_t::den;
        constexpr int periods_per_mks = std::micro::den * period_t::num / period_t::den;

        extern const time_point_t epoch_time_point;

        static_assert (periods_per_mks * period_t::den / period_t::num == std::micro::den,
          "INTERNAL ERROR");

        constexpr int count_digits(uint64_t den) { return (den == 0 ? 0 : 1 + count_digits(den / 10)); }

    }  // namespace detail

    namespace date_time
    {
        using Weekday = date::weekday;

        constexpr unsigned num_fractional_digits = detail::count_digits(detail::periods_per_sec - 1);

        class Base
        {
        public:
            enum Type
            {
                NORMAL = 0,
                POS_INFINITY = 2,
                NEG_INFINITY = -1,
                NOT_A_DATE_TIME = 1
            };

        protected:
            // This constructor is for derived classes only and is not intended to be used directly
            constexpr Base(Type type = NOT_A_DATE_TIME) noexcept
                : m_type(type)
            {
            }

        public:
            virtual ~Base() = default;

            bool is_special() const noexcept
            {
                return m_type != NORMAL;
            }

            bool is_pos_infinity() const noexcept
            {
                return m_type == POS_INFINITY;
            }

            bool is_neg_infinity() const noexcept
            {
                return m_type == NEG_INFINITY;
            }

            bool is_infinity() const noexcept
            {
                return m_type == POS_INFINITY || m_type == NEG_INFINITY;
            }

            bool is_not_a_date_time() const noexcept
            {
                return m_type == NOT_A_DATE_TIME;
            }

            inline Type type() const noexcept { return m_type; }

            virtual std::string to_simple_string() const = 0;

            virtual std::string to_iso_string() const = 0;

            virtual std::string to_iso_extended_string() const = 0;

            std::string special_time_as_string() const;

            static Type check_type(const Type& type);

        protected:
            static Type add_impl(Type a, Type b);

            static Type sub_impl(Type a, Type b);

            inline void set_type(Type type) noexcept
            {
                m_type = type;
            }

            bool operator == (const Base& other) const;
            bool operator != (const Base& other) const;
            bool operator < (const Base& other) const;
            bool operator <= (const Base& other) const;
            bool operator > (const Base& other) const;
            bool operator >= (const Base& other) const;

        private:
            void assert_supported() const;
            void assert_supported(const Base& other) const;

        private:
            Type m_type;
        };

        // FIXME: make mameber function of Base and use it in template functions below
        template <
            typename TimeType,
            typename std::enable_if<!std::is_same<TimeType, Base>::value &&
                                    std::is_base_of<Base, TimeType>::value, int>::type = 0
        >
        std::string format_time(const std::string& format, const TimeType& time)
        {
            if (time.is_special())
            {
                return time.special_time_as_string();
            }

            const auto& impl = time.get_impl();
            return date::format(format, impl);
        }

        template <
            typename TimeType,
            typename std::enable_if<!std::is_same<TimeType, Base>::value &&
                                    std::is_base_of<Base, TimeType>::value, int>::type = 0
        >
        std::string format_time(
            const std::locale& locale,
            const std::string& format,
            const TimeType& time)
        {
            if (time.is_special())
            {
                return time.special_time_as_string();
            }

            const auto& impl = time.get_impl();
            return date::format(locale, format, impl);
        }

        std::string to_simple_string(const Base& time);

        std::string to_iso_string(const Base& time);

        std::string to_iso_extended_string(const Base& time);

        std::ostream& operator << (std::ostream& os, const Base& time);
    }
}
