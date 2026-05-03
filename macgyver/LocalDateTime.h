#pragma once

#include "DateTime.h"

#include "date_time/LocalDateTime.h"

namespace Fmi
{
  using LocalDateTime = date_time::LocalDateTime;
  using LocalTimePeriod = date_time::TimePeriod<date_time::LocalDateTime>;

  using TimeZonePtr = date_time::TimeZonePtr;

  using date_time::make_time;
}
