// ======================================================================
/*!
 * \file NFmiTimeBag.cpp
 * \brief Implementation of class NFmiTimeBag
 */
// ======================================================================

#include "NFmiTimeBag.h"
#include <macgyver/Exception.h>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

using namespace std;

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiTimeBag::GetSize() const
{
  try
  {
    if (itsResolution.IsDate())
    {
      if (itsResolution.Year())
        return (itsLastTime.GetYear() - itsFirstTime.GetYear()) / itsResolution.Year() + 1;
      // Esim (1997 - 1927) / 5 = 4

      if (itsResolution.Month())
      {
        return (itsLastTime.GetYear() - itsFirstTime.GetYear()) * 12 +
                           ((itsLastTime.GetMonth() - itsFirstTime.GetMonth())) <
                       0
                   ? (12 - (itsLastTime.GetMonth() - itsFirstTime.GetMonth())) /
                             itsResolution.Month() +
                         1
                   : (itsLastTime.GetMonth() - itsFirstTime.GetMonth()) / itsResolution.Month() + 1;
      }
      // Esim (1997 - 1995) * 12 + (10 - 2 < 0) ? 12-(10-2) : 10-2
      //
      //		last   first         l    f

      if (itsResolution.Day())
        return itsLastTime.DifferenceInDays(itsFirstTime) / itsResolution.Day() + 1;
    }

    if (!itsResolution)
      return 1;  // 28.5.1998/Vili, oli 0 mutta muutin 1, oletuskoko on aina 1
    // throw Fmi::Exception(BCP,"NFmiTimeBag::GetSize() itsResolution = 0 ");

    if (itsFirstTime <= itsLastTime)
      return (itsLastTime.DifferenceInMinutes(itsFirstTime) / itsResolution) + 1;

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeBag::IsEmpty() const
{
  try
  {
    return NFmiSize::GetSize() == 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBag This-otus ja theBag yhdistetään halutulla tavalla.
 * \param theStartTimeFunction Mistä otetaan alkuaika, jos 0, pienempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \param theEndTimeFunction Mistä otetaan loppuaika, jos 0, suurempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \return Palautetaan yhdistetty timebagi
 *
 * \todo Should be a const method, but NFmiSortable does not allow it
 */
// ----------------------------------------------------------------------

const NFmiTimeBag NFmiTimeBag::Combine(const NFmiTimeBag &theBag,
                                       int theStartTimeFunction,
                                       int theEndTimeFunction)
{
  try
  {
    NFmiMetTime startTime;
    if (theStartTimeFunction == 0)
      startTime = itsFirstTime < theBag.itsFirstTime ? itsFirstTime : theBag.itsFirstTime;
    else if (theStartTimeFunction == 1)
      startTime = itsFirstTime;
    else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
      startTime = theBag.itsFirstTime;

    NFmiMetTime endTime;
    if (theEndTimeFunction == 0)
      endTime = itsLastTime > theBag.itsLastTime ? itsLastTime : theBag.itsLastTime;
    else if (theEndTimeFunction == 1)
      endTime = itsLastTime;
    else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
      endTime = theBag.itsLastTime;

    return NFmiTimeBag(startTime,
                       endTime,
                       itsResolution < theBag.itsResolution ? itsResolution : theBag.itsResolution);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::Next()
{
  try
  {
    itsCurrentTime.NextMetTime(itsResolution);
    itsIndex++;
    return itsCurrentTime <= itsLastTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::Previous()
{
  try
  {
    itsCurrentTime.PreviousMetTime(itsResolution);
    itsIndex--;
    return itsCurrentTime >= itsFirstTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param directionToIter Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeBag::Reset(FmiDirection directionToIter)
{
  try
  {
    if (directionToIter == kForward)
    {
      itsCurrentTime = itsFirstTime;
      itsCurrentTime.PreviousMetTime(itsResolution);
      itsIndex = -1;
    }
    else
    {
      itsCurrentTime = itsLastTime;
      itsCurrentTime.NextMetTime(itsResolution);
      itsIndex = GetSize();
    }

    if (itsResolution != 0)
      itsSize = GetSize();
    else
      itsSize = 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::SetCurrent(const NFmiMetTime &theTime)
{
  try
  {
    auto resolutionInMinutes = static_cast<long>(itsResolution);
    assert(resolutionInMinutes);
    int diffFromFirstTimeInMinutes = theTime.DifferenceInMinutes(itsFirstTime);
    if (diffFromFirstTimeInMinutes >= 0 && (diffFromFirstTimeInMinutes % resolutionInMinutes == 0))
    {
      if (diffFromFirstTimeInMinutes <
          resolutionInMinutes *
              static_cast<long>(itsSize))  // tarkistetaan vielä, ettei mene yli lastTimen
      {
        itsCurrentTime = itsFirstTime;
        if (diffFromFirstTimeInMinutes)  // pientä optimointia, pitäisi tehdä NFmiTime-luokan
                                         // ChangeBy???-metodeissa!!!!
          itsCurrentTime.ChangeByMinutes(diffFromFirstTimeInMinutes);

        itsIndex = diffFromFirstTimeInMinutes / resolutionInMinutes;
        return true;
      }
    }
    itsIndex = -1;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee this timebagista theBag2:n leikkauksen siten, että rajat (firstTime
 * ja lastTime) otetaan joko this-bagistä tai theBag2:sta, riippuen siitä
 * onko rajoittavan bagin ajat this-bagin sisällä vai ulkona. Tulos
 * timebagi sijoitetaan refBag:iin ja siihen saa joko this:in tai
 * theBag2:en aikaresoluution.
 * \param theBag2 Leikkausta rajoittava timebagi
 * \param refBag Tulos-timebagi sijoitetaan tähän.
 * \param resolOfBag2 Jos true, otetaan aikaresoluutio theBag2:sta, muuten this:istä.
 * \return Palauttaa false, jos tulostimebagiä ei voi tehdä, en ole varma voiko mennä siihen.
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::CalcIntersection(const NFmiTimeBag &theBag2,
                                   NFmiTimeBag &refBag,
                                   bool resolOfBag2)
{
  try
  {
    NFmiMetTime begTime(itsFirstTime > theBag2.itsFirstTime ? itsFirstTime : theBag2.itsFirstTime);
    NFmiMetTime endTime(itsLastTime < theBag2.itsLastTime ? itsLastTime : theBag2.itsLastTime);

    if (begTime > endTime)
      return false;

    if (resolOfBag2)
      refBag = NFmiTimeBag(begTime, endTime, theBag2.Resolution());
    else
      refBag = NFmiTimeBag(begTime, endTime, itsResolution);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee this timebagista leikkauksen siten, että rajat otetaan this-bagistä.
 * Metodi etsii annettujen rajojen sisältä/rajalta timebagistä aikoja
 * ja muodostaa uuden timebagin joka palautetaan ulos.
 * Jos rajat ovat ohi kokonaan timebagistä ja vielä samaan suuntaan,
 * palautetaan tyhjä timebagi. Samoin käy jos rajojen sisään ei sovi yhtään
 * timebagin aikaa.
 * \param theStartLimit Leikkausta rajoittava alkuaika.
 * \param theEndLimit Leikkausta rajoittava loppuaika.
 * \return Palauttaa tulos timebagin.
 */
// ----------------------------------------------------------------------

const NFmiTimeBag NFmiTimeBag::GetIntersection(const NFmiMetTime &theStartLimit,
                                               const NFmiMetTime &theEndLimit)
{
  try
  {
    if (FindNearestTime(theStartLimit, kForward))
    {
      NFmiMetTime startTime(CurrentTime());
      if (FindNearestTime(theEndLimit, kBackward))
      {
        NFmiMetTime endTime(CurrentTime());
        return NFmiTimeBag(startTime, endTime, Resolution());
      }
    }
    return NFmiTimeBag();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::SetTime(unsigned long theIndex)
{
  try
  {
    if (theIndex < GetSize())
    {
      SetCurrentIndex(theIndex);  // NFmiSize asetetaan tällä
      itsCurrentTime = itsFirstTime;
      if (theIndex > 0 && itsResolution.IsDate())
      {
        if (itsResolution.Year())
          itsCurrentTime.SetYear(
              static_cast<short>(itsCurrentTime.GetYear() + (theIndex * itsResolution.Year())));
        if (itsResolution.Month())
          throw Fmi::Exception(BCP, "NFmiTimeBag::SetTime: Kuukausi resoluutiossa ei toimi vielä.");
        //	itsCurrentTime.ChangeByMonth(theIndex * itsResolution.Month()); // TEE
        // ChangeByMonth-metodi!!!!
        if (itsResolution.Day())
          itsCurrentTime.ChangeByDays(theIndex * itsResolution.Day());

        return true;
      }
      else
      {
        itsCurrentTime.ChangeByMinutes(theIndex * itsResolution);
        return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::IsInside(const NFmiMetTime &theTime) const
{
  try
  {
    if (theTime >= itsFirstTime && theTime <= itsLastTime)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param minutes Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeBag::MoveByMinutes(long minutes)
{
  try
  {
    itsFirstTime.ChangeByMinutes(minutes);
    itsLastTime.ChangeByMinutes(minutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * This method seeks the nearest time of this timebag to theTime
 * given as a parameter considering also theDirection and
 * theTimeRangeInMinutes. The method is moved here from the class
 * NFmiTimeDescriptor where it has been made by Lasse. The new
 * parameter theTimeRangeInMinutes has the default value kLongMissing.
 * In addition theDirection has also the defaul value kCenter, when
 * seeks the nearest time step of this timebag. If theTime was outside
 * of this timebag but inside theTimeRangeInMinutes taking into
 * consideration theDirection, this timebag sets to the first or to
 * the last time respectively and the returning is true.
 *
 * \param theTime Undocumented
 * \param theDirection Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::FindNearestTime(const NFmiMetTime &theTime,
                                  FmiDirection theDirection,
                                  unsigned long theTimeRangeInMinutes)
{
  try
  {
    bool noTimeRange = (theTimeRangeInMinutes == kUnsignedLongMissing);
    Reset();
    if (IsInside(theTime))
    {
      while (Next())
      {
        if (CurrentTime() == theTime)
          return true;
        if (CurrentTime() > theTime)
        {
          if (theDirection == kForward)
            return (noTimeRange || CurrentTime().DifferenceInMinutes(theTime) <=
                                       static_cast<long>(theTimeRangeInMinutes));
          if (theDirection == kBackward)
          {
            Previous();
            return (noTimeRange || theTime.DifferenceInMinutes(CurrentTime()) <=
                                       static_cast<long>(theTimeRangeInMinutes));
          }
          // finds the nearest time to theTime (e.g. theDirection == kCenter)
          long deltaT2 = CurrentTime().DifferenceInMinutes(theTime);
          Previous();
          long deltaT1 = theTime.DifferenceInMinutes(CurrentTime());
          if (deltaT1 <= deltaT2)
            return (noTimeRange || deltaT1 <= static_cast<long>(theTimeRangeInMinutes));
          Next();
          return (noTimeRange || deltaT2 <= static_cast<long>(theTimeRangeInMinutes));
        }
      }
    }

    // theTime wasn't inside of this timebag

    bool timeIsLeftFromTimeBag = (theTime < FirstTime());
    Reset();
    if (theDirection == kBackward)
    {
      if (!timeIsLeftFromTimeBag)
      {
        Reset(kBackward);
        Previous();
      }
      else
        return false;
    }
    else if (theDirection == kForward)
    {
      if (timeIsLeftFromTimeBag)
        Next();
      else
        return false;
    }
    else  // if(theDirection == kCenter) // seeks the nearest time to theTime
    {
      if (!timeIsLeftFromTimeBag)
      {
        Reset(kBackward);
        Previous();
      }
      else       // if(timeIsLeftFromTimeBag)
        Next();  // the first time of this timebag
    }
    if (noTimeRange || labs(theTime.DifferenceInMinutes(CurrentTime())) <=
                           static_cast<long>(theTimeRangeInMinutes))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBag Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimeBag &NFmiTimeBag::operator=(const NFmiTimeBag &theBag)
{
  try
  {
    NFmiSize::operator=(*(static_cast<const NFmiSize *>(&theBag)));

    itsFirstTime = theBag.itsFirstTime;
    itsCurrentTime = theBag.itsCurrentTime;
    itsLastTime = theBag.itsLastTime;
    itsResolution = theBag.itsResolution;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeBag Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeBag::operator==(const NFmiTimeBag &theTimeBag) const
{
  try
  {
    if (itsFirstTime == theTimeBag.itsFirstTime && itsLastTime == theTimeBag.itsLastTime &&
        itsResolution == theTimeBag.itsResolution)
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream &NFmiTimeBag::Write(std::ostream &file) const
{
  try
  {
    file << itsFirstTime.GetYear() << " " << itsFirstTime.GetMonth() << " " << itsFirstTime.GetDay()
         << " " << itsFirstTime.GetHour() << " " << itsFirstTime.GetMin() << " "
         << itsFirstTime.GetSec() << std::endl;

    file << itsLastTime.GetYear() << " " << itsLastTime.GetMonth() << " " << itsLastTime.GetDay()
         << " " << itsLastTime.GetHour() << " " << itsLastTime.GetMin() << " "
         << itsLastTime.GetSec() << std::endl;

    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
      file << itsResolution;
    else
      file << static_cast<long>(itsResolution) << std::endl;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::istream &NFmiTimeBag::Read(std::istream &file)
{
  try
  {
    short year, month, day, hour, min, sec;

    file >> year >> month >> day >> hour >> min >> sec;
    itsFirstTime = NFmiMetTime(year, month, day, hour, min, sec, 1);

    file >> year >> month >> day >> hour >> min >> sec;
    itsLastTime = NFmiMetTime(year, month, day, hour, min, sec, 1);

    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file >> itsResolution;
    }
    else
    {
      long theResolution;
      file >> theResolution;
      itsResolution = theResolution;
    }

    // Safety checks against bad resolutions since they may cause segmentation faults
    // in other places where the resolution is assumed to be correct.

    long mins = itsResolution;
    long periodlength = itsLastTime.DifferenceInMinutes(itsFirstTime);

    if (periodlength != 0 && periodlength % mins != 0)
      throw Fmi::Exception(BCP, "Invalid time resolution in NFmiTimeBag");

    Reset();
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Karsii halutut ylimääräiset ajat pois
 * \param theMaxTimeCount Kertoo kuinka monta aikaa saa olla maksimissaan.
 * \param fFromEnd kertoo, mistä päästä karsitaan ylimääräiset pois.
 */
// ----------------------------------------------------------------------

void NFmiTimeBag::PruneTimes(int theMaxTimeCount, bool fFromEnd)
{
  try
  {
    if (static_cast<int>(GetSize()) > theMaxTimeCount)
    {
      if (fFromEnd)
      {
        NFmiMetTime tmpTime(itsFirstTime);
        tmpTime.ChangeByMinutes(itsResolution * (theMaxTimeCount - 1));
        itsLastTime = tmpTime;
      }
      else
      {
        NFmiMetTime tmpTime(itsLastTime);
        tmpTime.ChangeByMinutes(-itsResolution * (theMaxTimeCount - 1));
        itsFirstTime = tmpTime;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Muuttaa timebagiä niin että annettu aika laitetaan firstTimeksi,
 * ja lastTimeksi lasketaan uusi arvo, niin että erotus ensimmäisen ja
 * viimeisen ajan välillä pysyy samana. Näin timebagin rakenne ja resoluutio säilyvät.
 * Lisäksi currentTime:ksi asetetaan theTime arvo eli 1. aika.
 * \param theTime on timebagin uusi firstTime.
 */
// ----------------------------------------------------------------------

void NFmiTimeBag::SetNewStartTime(const NFmiMetTime &theTime)
{
  try
  {
    long moveInMinutes = theTime.DifferenceInMinutes(itsFirstTime);
    MoveByMinutes(moveInMinutes);
    SetCurrent(theTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
