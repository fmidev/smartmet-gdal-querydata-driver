// ======================================================================
/*!
 * \file NFmiMilliSecondTimer.h
 * \brief Interface of class NFmiMilliSecondTimer
 */
// ======================================================================
/*!
 * \class NFmiMilliSecondTimer
 *
 * Määrittely luokalle, jolla pystyy mittaamaan aikaa ainakin teoreettisesti
 * millisekunnin tarkkuudella. Todellisuudessa tarkkuus kai rajoittuu
 * muistaakseni 18 Hz:n tarkkuuteen eli n. 0.0555... sekunnin tarkkuuteen.
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiStringTools.h"
#include <macgyver/DateTime.h>
#include <chrono>

class NFmiNanoSecondTimer
{
  std::chrono::time_point<std::chrono::steady_clock> startTime_;

 public:
  NFmiNanoSecondTimer();
  NFmiNanoSecondTimer(int moveStartByMS);

  void restart();
  double elapsedTimeInSeconds() const;
  std::string elapsedTimeInSecondsString(int precision = 3) const;
};

//! Luokka koodin nopeusmittauksia varten

class NFmiMilliSecondTimer
{
 public:
  NFmiMilliSecondTimer();

  void StartTimer();
  void StopTimer();
  int TimeDiffInMSeconds() const;
  int CurrentTimeDiffInMSeconds()
      const;  // alkuajan ja nykyhetken erotus milli sekunneissa (eli kelloa ei pysäytetä)
  std::string EasyTimeDiffStr(bool fIgnoreMilliSeconds = false) const;
  static std::string EasyTimeDiffStr(int theDiffInMS, bool fIgnoreMilliSeconds = false);

  // Deprecated:
  void FirstTime();
  void SecondTime();

 private:
  Fmi::DateTime itsTime1;  //!< The start time
  Fmi::DateTime itsTime2;  //!< The end time

};  // class NFmiMilliSecondTimer

// ----------------------------------------------------------------------
/*!
 * Starts the timer
 *
 * \deprecated
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::FirstTime()
{
  itsTime1 = Fmi::MicrosecClock::universal_time();
}
// ----------------------------------------------------------------------
/*!
 * Ends the timer
 *
 * \deprecated
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::SecondTime()
{
  itsTime2 = Fmi::MicrosecClock::universal_time();
}
// ----------------------------------------------------------------------
/*!
 * Starts the timer
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::StartTimer()
{
  FirstTime();
}
// ----------------------------------------------------------------------
/*!
 * Ends the timer
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::StopTimer()
{
  SecondTime();
}
// ----------------------------------------------------------------------
/*!
 * Returns the measured time in milliseconds
 *
 * \return The measured time in milliseconds
 * \todo Make the method const
 */
// ----------------------------------------------------------------------

inline int NFmiMilliSecondTimer::TimeDiffInMSeconds() const
{
  return (itsTime2 - itsTime1).total_milliseconds();
}

inline int NFmiMilliSecondTimer::CurrentTimeDiffInMSeconds() const
{
  return (Fmi::MicrosecClock::universal_time() - itsTime1).total_milliseconds();
}

// ======================================================================
