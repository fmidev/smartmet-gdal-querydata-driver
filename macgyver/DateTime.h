#pragma once

#include "date_time/DateTime.h"
#include "date_time/TimeDuration.h"
#include "date_time/Date.h"
#include "date_time/TimePeriod.h"

namespace Fmi
{
    using Date = date_time::Date;
    using TimeDuration = date_time::TimeDuration;
    using DateTime = date_time::DateTime;
    using DatePeriod = date_time::TimePeriod<date_time::Date>;
    using TimePeriod = date_time::TimePeriod<date_time::DateTime>;

    using date_time::Seconds;
    using date_time::Minutes;
    using date_time::Hours;
    using date_time::Days;
    using date_time::Milliseconds;
    using date_time::Microseconds;
    using date_time::Milliseconds;

    using date_time::format_time;

    namespace MicrosecClock = date_time::MicrosecClock;
    namespace SecondClock = date_time::SecondClock;
}  // namespace Fmi
