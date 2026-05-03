#pragma once

#include "Base.h"
#include <map>
#include <string>
#include <vector>

namespace Fmi
{
    namespace date_time
    {
        //class LocalDateTime;

        class TimeZonePtr
        {
            const date::time_zone* tz;

        public:
            /**
             * Construct an empty time zone pointer.
            */
            TimeZonePtr() noexcept;

            /**
             * Construct a time zone pointer from a time zone name.
             * 
             * @param name Time zone name
             * @exception Fmi::Exception if time zone not found
            */
            TimeZonePtr(const std::string& name);

            /**
             * Construct a time zone pointer from a embeded date::time_zone pointer
            */
            TimeZonePtr(const date::time_zone* tz) noexcept;

            /**
             * Copy constructor.
            */
            TimeZonePtr(const TimeZonePtr& src) noexcept;

            virtual ~TimeZonePtr();

            TimeZonePtr& operator = (const TimeZonePtr& src) noexcept;

            /**
             * Check if time zone pointer is set. 
            */
            inline operator bool () const noexcept { return bool(tz); }

            /**
             * Check if time zone pointer are identical
            */
            inline bool operator == (const TimeZonePtr& other) const noexcept { return tz == other.tz; }

            /**
             * Check if time zone pointer are different
            */
            inline bool operator != (const TimeZonePtr& other) const noexcept { return tz != other.tz; }

            /**
             * Get the emdeded date::time_zone pointer.
            */
            inline operator const date::time_zone * () const { return zone_ptr(); }

            /**
             * Get the emdeded date::time_zone pointer.
            */
            inline const date::time_zone * operator -> () const { return zone_ptr(); }

            /**
             * Get the emdeded date::time_zone pointer.
            */
            const date::time_zone* zone_ptr() const;

            /**
             * Get the time zone name.
            */
            std::string name() const { return tz->name(); }
            //inline std::string get_abbrev(const LocalDateTime& time) { return time.get_sys_info().abbrev; }

            bool is_utc() const;

            //LocalDateTime now() const;

            /**
             * UTC time zone pointer
            */
            static TimeZonePtr utc;

            /**
             * Get a list of all time zone names.
             * 
             * @warning date library must be compiled with identical macron
             *          USE_OS_TZDB value or otherwise this function will segfault. 
            */
            static std::vector<std::string> get_region_list();

            static std::map<std::string, TimeZonePtr> get_region_map(bool debug = false);
        };
    }
}
