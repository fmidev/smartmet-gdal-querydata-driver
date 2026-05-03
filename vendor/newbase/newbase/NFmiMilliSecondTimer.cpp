// ======================================================================
/*!
 * \file NFmiMilliSecondTimer.cpp
 * \brief Implementation of class NFmiMilliSecondTimer
 */

#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>

#include <iomanip>

namespace
{
int calculateSignificantDigitPrecision(double value, int wantedPrecision)
{
  int usedPrecision = 0;
  if (value < 0.000000001)
    usedPrecision = 9 + wantedPrecision;
  else if (value < 0.00000001)
    usedPrecision = 8 + wantedPrecision;
  else if (value < 0.0000001)
    usedPrecision = 7 + wantedPrecision;
  else if (value < 0.000001)
    usedPrecision = 6 + wantedPrecision;
  else if (value < 0.00001)
    usedPrecision = 5 + wantedPrecision;
  else if (value < 0.0001)
    usedPrecision = 4 + wantedPrecision;
  else if (value < 0.001)
    usedPrecision = 3 + wantedPrecision;
  else if (value < 0.01)
    usedPrecision = 2 + wantedPrecision;
  else if (value < 0.1)
    usedPrecision = 1 + wantedPrecision;
  else if (value < 1)
    usedPrecision = 0 + wantedPrecision;
  else
    usedPrecision = 1;

  if (usedPrecision <= 1) usedPrecision = 1;

  return usedPrecision;
}

}  // namespace

NFmiNanoSecondTimer::NFmiNanoSecondTimer() : startTime_(std::chrono::steady_clock::now()) {}

NFmiNanoSecondTimer::NFmiNanoSecondTimer(int moveStartByMS)
    : startTime_(std::chrono::steady_clock::now())
{
  startTime_ += std::chrono::milliseconds(moveStartByMS);
}

void NFmiNanoSecondTimer::restart() { startTime_ = std::chrono::steady_clock::now(); }

double NFmiNanoSecondTimer::elapsedTimeInSeconds() const
{
  std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now();
  double elapsedSeconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime_).count();
  return elapsedSeconds;
}

std::string NFmiNanoSecondTimer::elapsedTimeInSecondsString(int precision) const
{
  double elapsedSeconds = elapsedTimeInSeconds();
  std::stringstream out;
  out << std::fixed
      << std::setprecision(::calculateSignificantDigitPrecision(elapsedSeconds, precision))
      << elapsedSeconds << " s";
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMilliSecondTimer::NFmiMilliSecondTimer()
{
  try
  {
    FirstTime();
    itsTime2 = itsTime1;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string NFmiMilliSecondTimer::EasyTimeDiffStr(int theDiffInMS, bool fIgnoreMilliSeconds)
{
  try
  {
    static const double dayInMS = 1000. * 60 * 60 * 24;
    static const double hourInMS = 1000. * 60 * 60;
    static const double minuteInMS = 1000. * 60;
    int diffInMS = theDiffInMS;
    auto days = static_cast<int>(diffInMS / dayInMS);
    if (days > 0)
      diffInMS = static_cast<int>(diffInMS - days * dayInMS);
    auto hours = static_cast<int>(diffInMS / hourInMS);
    if (hours > 0)
      diffInMS = static_cast<int>(diffInMS - hours * hourInMS);
    auto minutes = static_cast<int>(diffInMS / minuteInMS);
    if (minutes > 0)
      diffInMS = static_cast<int>(diffInMS - minutes * minuteInMS);
    auto seconds = static_cast<int>(diffInMS / 1000.);
    int msecs = diffInMS % 1000;
    std::string result;
    bool printRest = false;
    if (days > 0)
    {
      printRest = true;
      result += NFmiStringTools::Convert<int>(days) + " d ";
    }
    if (hours > 0 || printRest)
    {
      printRest = true;
      result += NFmiStringTools::Convert<int>(hours) + " h ";
    }
    if (minutes > 0 || printRest)
    {
      // printRest = true;
      result += NFmiStringTools::Convert<int>(minutes) + " m ";
    }
    //	sekunnit tulee aina
    result += NFmiStringTools::Convert<int>(seconds) + " s ";
    if (fIgnoreMilliSeconds == false)
    {
      NFmiValueString valStr(msecs, "%03d");
      result += valStr.CharPtr();
      result += " ms ";
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string NFmiMilliSecondTimer::EasyTimeDiffStr(bool fIgnoreMilliSeconds) const
{
  try
  {
    int diffInMS = TimeDiffInMSeconds();
    return NFmiMilliSecondTimer::EasyTimeDiffStr(diffInMS, fIgnoreMilliSeconds);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
