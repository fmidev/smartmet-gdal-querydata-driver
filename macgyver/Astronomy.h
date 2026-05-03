// ======================================================================
/*!
 * \brief Astronomical calculations
 *
 * Solar calculations
 *
 * Based on NOAA JavaScript at
 *       <http://www.srrb.noaa.gov/highlights/sunrise/azel.html>
 *       <http://www.srrb.noaa.gov/highlights/sunrise/sunrise.html>
 *
 * Reference:
 *       Solar Calculation Details
 *       <http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html>
 *
 * Transformation to C++ by Ilmatieteen Laitos, 2008.
 *
 * License:
 *       UNKNOWN (not stated in JavaScript)
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"

namespace Fmi
{
namespace Astronomy
{
/*
 * Position of Sun at a given time and place on Earth
 *
 * 'lon': Positive east of GM [-180,180]
 * 'lat': [-90,90]; truncated to +-89.8 near the poles
 *
 * To give times in UTC, create a 'local_date_time' with UTC timezone.
 */

struct solar_position_t
{
  const double azimuth;      // solar azimuth
  const double declination;  // solar declination (degrees)
  const double elevation;    // solar elevation

  solar_position_t(double a, double d, double e) : azimuth(a), declination(d), elevation(e) {}
  bool dark() const;
};

/*
 * Sunrise, noon and sunset times
 *
 * 'lon': Positive east of GM [-180,180]
 * 'lat': [-90,90]; truncated to +-89.8 near the poles
 *
 * Near the poles, sun may never (within a day) rise or set. In such a case,
 * rise and set are set to the TWO NEAREST EVENTS of the noon (one prior,
 * one after). 'sunset' may precede 'sunrise', which means we're in all night
 * season.
 */

struct solar_time_t
{
  const Fmi::LocalDateTime sunrise;
  const Fmi::LocalDateTime sunset;
  const Fmi::LocalDateTime noon;  // even if below the horizon

  bool sunrise_today() const { return sunrise.local_time().date() == noon.local_time().date(); }
  bool sunset_today() const { return sunset.local_time().date() == noon.local_time().date(); }
  bool polar_day() const { return sunset.local_time().date() > sunrise.local_time().date(); }
  bool polar_night() const { return sunset < sunrise; }
  solar_time_t(const Fmi::LocalDateTime& sr,
               const Fmi::LocalDateTime& ss,
               const Fmi::LocalDateTime& noon_)
      : sunrise(sr), sunset(ss), noon(noon_)
  {
  }

  TimeDuration daylength() const;
};

enum SetAndRiseOccurence
{
  FIRST_RISE,
  SECOND_RISE,
  FIRST_SET,
  SECOND_SET
};

struct lunar_time_t
{
  Fmi::LocalDateTime moonrise;
  Fmi::LocalDateTime moonset;
  Fmi::LocalDateTime moonrise2;
  Fmi::LocalDateTime moonset2;
  bool rise_today = false;
  bool set_today = false;
  bool rise2_today = false;
  bool set2_today = false;
  bool above_hz_24h = false;

  lunar_time_t(const Fmi::LocalDateTime& mr,
               const Fmi::LocalDateTime& ms,
               const Fmi::LocalDateTime& mr2,
               const Fmi::LocalDateTime& ms2,
               bool rise,
               bool set,
               bool rise2,
               bool set2,
               bool above24h)
      : moonrise(mr),
        moonset(ms),
        moonrise2(mr2),
        moonset2(ms2),
        rise_today(rise),
        set_today(set),
        rise2_today(rise2),
        set2_today(set2),
        above_hz_24h(above24h)
  {
  }

  lunar_time_t() = default;

  const Fmi::LocalDateTime& risesettime(SetAndRiseOccurence occ) const;

  bool moonrise_today() const { return rise_today; }
  bool moonset_today() const { return set_today; }
  bool moonrise2_today() const { return rise2_today; }
  bool moonset2_today() const { return set2_today; }
  bool above_horizont_24h() const { return above_hz_24h; }
  std::string as_string(SetAndRiseOccurence occ) const;
  std::string as_string_long(SetAndRiseOccurence occ) const;  // date included
};

std::ostream& operator<<(std::ostream&, const lunar_time_t& lt);

// Actual functions

solar_position_t solar_position(const Fmi::LocalDateTime& ldt, double lon, double lat);

solar_position_t solar_position(const DateTime& utc, double lon, double lat);

solar_time_t solar_time(const Fmi::Date& ldate, double lon, double lat);

solar_time_t solar_time(const Fmi::LocalDateTime& ldt, double lon, double lat);

double moonphase(const DateTime& utc);

double lunar_phase(const DateTime& utc);
lunar_time_t lunar_time(const Fmi::LocalDateTime& ldt,
                        double lon,
                        double lat,
                        bool allow_missing_dates = false);
std::string moon_rise(const lunar_time_t& lunar_time_t);
std::string moon_set(const lunar_time_t& lunar_time_t);
std::string moon_riseset(const lunar_time_t& lunar_time_t);

}  // namespace Astronomy
}  // namespace Fmi

// ======================================================================
