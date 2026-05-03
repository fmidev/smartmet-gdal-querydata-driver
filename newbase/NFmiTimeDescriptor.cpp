// ======================================================================
/*!
 * \file NFmiTimeDescriptor.cpp
 * \brief Implementation of class NFmiTimeDescriptor
 */
// ======================================================================
/*!
 * \class NFmiTimeDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiTimeDescriptor.h"
#include "NFmiMetTime.h"
#include "NFmiTimeBag.h"
#include "NFmiTimeList.h"

#include "NFmiVersion.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::~NFmiTimeDescriptor()
{
  try
  {
    Destroy();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor()
    : itsOriginTimeBag(nullptr),
      itsValidTimeBag(nullptr),
      itsTimeList(nullptr),
      itsTimeBagIdent(true),
      itsIsLocalTime(kUTC),
      itsIsInterpolation(false),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(nullptr)
{
  try
  {
    NFmiMetTime timeNow;
    itsOriginTimeBag = new NFmiTimeBag(timeNow, timeNow, 60);
    itsValidTimeBag = new NFmiTimeBag(timeNow, timeNow, 60);
    itsActivity = new bool[Size()];
    for (int i = 0; i < static_cast<int>(Size()); i++)
      itsActivity[i] = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOriginTime Undocumented
 * \param theValidTimeBag Undocumented
 * \param theIsLocalTime Undocumented
 * \param theIsInterpolation Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                                       const NFmiTimeBag &theValidTimeBag,
                                       FmiTimeLocalzation theIsLocalTime,
                                       bool theIsInterpolation)
    : itsOriginTimeBag(new NFmiTimeBag(theOriginTime, theOriginTime, 0)),
      itsValidTimeBag(new NFmiTimeBag(theValidTimeBag)),
      itsTimeList(nullptr),
      itsTimeBagIdent(true),
      itsIsLocalTime(theIsLocalTime),
      itsIsInterpolation(theIsInterpolation),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(
          new bool[itsValidTimeBag ? itsValidTimeBag->GetSize() : itsTimeList->NumberOfItems()])
{
  try
  {
    for (int i = 0; i < static_cast<int>(itsValidTimeBag ? itsValidTimeBag->GetSize()
                                                         : itsTimeList->NumberOfItems());
         i++)
      itsActivity[i] = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOriginTime Undocumented
 * \param theTimeList Undocumented
 * \param theIsLocalTime Undocumented
 * \param theIsInterpolation Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                                       const NFmiTimeList &theTimeList,
                                       FmiTimeLocalzation theIsLocalTime,
                                       bool theIsInterpolation)
    : itsOriginTimeBag(new NFmiTimeBag(theOriginTime, theOriginTime, 0)),
      itsValidTimeBag(nullptr),
      itsTimeList(new NFmiTimeList(theTimeList)),
      itsTimeBagIdent(true),
      itsIsLocalTime(theIsLocalTime),
      itsIsInterpolation(theIsInterpolation),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(nullptr)
{
  try
  {
    if (itsValidTimeBag)
    {
      itsActivity =
          new bool[itsValidTimeBag ? itsValidTimeBag->GetSize() : itsTimeList->NumberOfItems()];
      for (int i = 0; i < static_cast<int>(itsValidTimeBag ? itsValidTimeBag->GetSize()
                                                           : itsTimeList->NumberOfItems());
           i++)
        itsActivity[i] = true;
    }
    else if (itsTimeList)
    {
      itsActivity = new bool[itsTimeList->NumberOfItems()];
      for (int i = 0; i < static_cast<int>(itsTimeList->NumberOfItems()); i++)
        itsActivity[i] = true;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOriginTimeBag Undocumented
 * \param theValidTime Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag,
                                       const NFmiMetTime &theValidTime)
    : itsOriginTimeBag(new NFmiTimeBag(theOriginTimeBag)),
      itsValidTimeBag(new NFmiTimeBag(theValidTime, theValidTime, 0)),
      itsTimeList(nullptr),
      itsTimeBagIdent(false),
      itsIsLocalTime(kUTC),
      itsIsInterpolation(false),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(new bool[itsOriginTimeBag->GetSize()])
{
  try
  {
    for (int i = 0; i < static_cast<int>(itsOriginTimeBag->GetSize()); i++)
      itsActivity[i] = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theOriginTimeBag Undocumented
 * \param theForecastPeriod Undocumented (in hours)
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiTimeBag &theOriginTimeBag,
                                       unsigned long theForecastPeriod)
    : itsOriginTimeBag(new NFmiTimeBag(theOriginTimeBag)),
      itsValidTimeBag(nullptr),
      itsTimeList(nullptr),
      itsTimeBagIdent(false),
      itsIsLocalTime(kUTC),
      itsIsInterpolation(false),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(new bool[itsOriginTimeBag->GetSize()])
{
  try
  {
    NFmiMetTime firstTime = itsOriginTimeBag->FirstTime();
    firstTime.NextMetTime(static_cast<short>(theForecastPeriod * 60));
    itsValidTimeBag = new NFmiTimeBag(firstTime, firstTime, 0);
    for (int i = 0; i < static_cast<int>(itsOriginTimeBag->GetSize()); i++)
      itsActivity[i] = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOriginTime Undocumented
 * \param theForecastPeriodMin Undocumented
 * \param theForecastPeriodMax Undocumented
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiMetTime &theOriginTime,
                                       unsigned long theForecastPeriodMin,
                                       unsigned long theForecastPeriodMax)
    : itsOriginTimeBag(new NFmiTimeBag(theOriginTime, theOriginTime, 0)),
      itsValidTimeBag(nullptr),
      itsTimeList(nullptr),
      itsTimeBagIdent(true),
      itsIsLocalTime(kUTC),
      itsIsInterpolation(false),
      itsIsOriginLastest(false),
      itsLocalTimeStep(0),
      itsActivity(nullptr)
{
  try
  {
    NFmiMetTime theStartTime(theOriginTime);
    theStartTime.ChangeByHours(static_cast<short>(theForecastPeriodMin));
    theStartTime.SetTimeStep(30);
    NFmiMetTime theEndTime(theStartTime);
    theEndTime.ChangeByHours(static_cast<short>(theForecastPeriodMax));

    itsValidTimeBag = new NFmiTimeBag(theStartTime, theEndTime, 30);

    itsActivity =
        new bool[itsValidTimeBag ? itsValidTimeBag->GetSize() : itsTimeList->NumberOfItems()];
    for (int i = 0; i < static_cast<int>(itsValidTimeBag ? itsValidTimeBag->GetSize()
                                                         : itsTimeList->NumberOfItems());
         i++)
      itsActivity[i] = true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theTimeDescriptor The object being copied
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor::NFmiTimeDescriptor(const NFmiTimeDescriptor &theTimeDescriptor)
    : NFmiDataDescriptor(),
      itsOriginTimeBag(theTimeDescriptor.itsOriginTimeBag
                           ? new NFmiTimeBag(*(theTimeDescriptor.itsOriginTimeBag))
                           : nullptr),
      itsValidTimeBag(theTimeDescriptor.itsValidTimeBag
                          ? new NFmiTimeBag(*(theTimeDescriptor.itsValidTimeBag))
                          : nullptr),
      itsTimeList(theTimeDescriptor.itsTimeList ? new NFmiTimeList(*(theTimeDescriptor.itsTimeList))
                                                : nullptr),
      itsTimeBagIdent(theTimeDescriptor.itsTimeBagIdent),
      itsIsLocalTime(theTimeDescriptor.itsIsLocalTime),
      itsIsInterpolation(theTimeDescriptor.itsIsInterpolation),
      itsIsOriginLastest(theTimeDescriptor.itsIsOriginLastest),
      itsLocalTimeStep(theTimeDescriptor.itsLocalTimeStep),
      itsActivity(nullptr)
{
  try
  {
    unsigned long theSize = 0;
    if (itsValidTimeBag != nullptr)
      theSize = itsValidTimeBag->GetSize();
    else if (itsTimeList != nullptr)
      theSize = itsTimeList->NumberOfItems();

    itsActivity = new bool[theSize];
    for (int i = 0; i < static_cast<int>(theSize); i++)
      itsActivity[i] = theTimeDescriptor.itsActivity[i];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::Destroy()
{
  try
  {
    if (itsValidTimeBag)
    {
      delete itsValidTimeBag;
      itsValidTimeBag = nullptr;
    }
    if (itsTimeList)
    {
      delete itsTimeList;
      itsTimeList = nullptr;
    }
    if (itsOriginTimeBag)
    {
      delete itsOriginTimeBag;
      itsOriginTimeBag = nullptr;
    }
    if (itsActivity)
    {
      delete[] itsActivity;
      itsActivity = nullptr;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::ExtendTimeBag()
{
  try
  {
    NFmiMetTime theFirstTime(itsValidTimeBag->FirstTime());
    NFmiMetTime theLastTime(itsValidTimeBag->LastTime());
    unsigned long theResolution = !itsLocalTimeStep
                                      ? static_cast<unsigned long>(itsValidTimeBag->Resolution())
                                      : itsLocalTimeStep;
    ;

    theFirstTime.ChangeByHours(-12);
    theFirstTime.SetTimeStep(
        static_cast<short>(theResolution));  // Siirtää ajan lähimmäksi UTC-lokeroksi
    theLastTime.ChangeByHours(12);
    theLastTime.SetTimeStep(static_cast<short>(theResolution));

    delete itsValidTimeBag;
    itsValidTimeBag = new NFmiTimeBag(theFirstTime, theLastTime, theResolution);

    delete[] itsActivity;
    itsActivity = new bool[itsValidTimeBag->GetSize()];

    int i;
    // All
    for (i = 0; i < static_cast<int>(itsValidTimeBag->GetSize()); i++)
      itsActivity[i] = true;

    // Start
    for (i = 0; i < static_cast<int>((12 * 60) / theResolution); i++)
      itsActivity[i] = false;

    // End
    for (i = itsValidTimeBag->GetSize() - (12 * 60) / theResolution;
         i < static_cast<int>(itsValidTimeBag->GetSize());
         i++)
      itsActivity[i] = false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::ReduseTimeBag()
{
  try
  {
    NFmiMetTime theFirstTime(itsValidTimeBag->FirstTime());
    NFmiMetTime theLastTime(itsValidTimeBag->LastTime());
    unsigned long theResolution = itsValidTimeBag->Resolution();

    theFirstTime.ChangeByHours(+12);
    theLastTime.ChangeByHours(-12);

    delete itsValidTimeBag;
    itsValidTimeBag = new NFmiTimeBag(theFirstTime, theLastTime, theResolution);

    delete[] itsActivity;
    itsActivity = new bool[itsValidTimeBag->GetSize()];

    int i;
    // All
    for (i = 0; i < static_cast<int>(itsValidTimeBag->GetSize()); i++)
      itsActivity[i] = true;

    // Start
    for (i = 0; i < static_cast<int>((12 * 60) / theResolution); i++)
      itsActivity[i] = false;

    // End
    for (i = itsValidTimeBag->GetSize() - (12 * 60) / theResolution;
         i < static_cast<int>(itsValidTimeBag->GetSize());
         i++)
      itsActivity[i] = false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::Reset()
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
      {
        if (itsTimeList)
          itsTimeList->Reset();
        else
          itsValidTimeBag->Reset();
      }
    }
    else
    {
      if (IsOriginTime())
        itsOriginTimeBag->Reset();
    }
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

bool NFmiTimeDescriptor::Next()
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return itsTimeList ? itsTimeList->Next() : itsValidTimeBag->Next();

      return false;
    }

    if (IsOriginTime())
      return (itsOriginTimeBag->Next());

    return false;
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

bool NFmiTimeDescriptor::Previous()
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return itsTimeList ? itsTimeList->Previous()
                           : itsValidTimeBag->Previous();  // Huom TimeList->false
      return false;
    }

    if (IsOriginTime())
      return (itsOriginTimeBag->Previous());

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

bool NFmiTimeDescriptor::Time(const NFmiMetTime &theTime)
{
  try
  {
    if (IsValidTime())
    {
      if (itsValidTimeBag)
        return itsValidTimeBag->SetCurrent(theTime);

      if (itsTimeList)
        return itsTimeList->Find(theTime);
    }
    else
    {
      if (itsOriginTimeBag)
        return itsOriginTimeBag->SetCurrent(theTime);
    }
    return false;  // assert:in paikka, ei saisi mennä tähän ????
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theDirection Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeDescriptor::TimeToNearestStep(const NFmiMetTime &theTime,
                                           FmiDirection theDirection,
                                           unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (itsTimeBagIdent)
      return (itsTimeList
                  ? itsTimeList->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes)
                  : itsValidTimeBag->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes));

    return (itsOriginTimeBag->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes));
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

const NFmiMetTime &NFmiTimeDescriptor::Time() const
{
  try
  {
    if (itsTimeBagIdent)
      return (itsTimeList ? *itsTimeList->Current() : itsValidTimeBag->CurrentTime());

    return (itsOriginTimeBag->CurrentTime());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime &NFmiTimeDescriptor::FirstTime() const
{
  try
  {
    if (itsTimeBagIdent)
      return (itsTimeList ? itsTimeList->FirstTime() : itsValidTimeBag->FirstTime());

    return (itsOriginTimeBag->FirstTime());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime &NFmiTimeDescriptor::LastTime() const
{
  try
  {
    if (itsTimeBagIdent)
      return (itsTimeList ? itsTimeList->LastTime() : itsValidTimeBag->LastTime());

    return (itsOriginTimeBag->LastTime());
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

const NFmiMetTime &NFmiTimeDescriptor::OriginTime() const
{
  try
  {
    if (!itsTimeBagIdent)
      return (itsOriginTimeBag->CurrentTime());

    return (itsOriginTimeBag->FirstTime());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiTimeDescriptor::OriginTime(const NFmiMetTime &newTime)
{
  try
  {
    if (!itsTimeBagIdent)
    {
      // en tiedä miten tämä pitäisi oikeasti hoitaa, tätä 'origintimebag' featurea ei ole oikeasti
      // käytetty.
      // siirrän kuitenkin koko origintimebagia niin että se alkaa annetusta origintimesta
      NFmiMetTime lastTime(newTime);
      lastTime.ChangeByMinutes(
          itsOriginTimeBag->LastTime().DifferenceInMinutes(itsOriginTimeBag->FirstTime()));
      *itsOriginTimeBag = NFmiTimeBag(newTime, lastTime, itsOriginTimeBag->Resolution());
    }
    else
    {
      if (itsOriginTimeBag)
        *itsOriginTimeBag = NFmiTimeBag(newTime, newTime, newTime.GetTimeStep());
      else
        itsOriginTimeBag = new NFmiTimeBag(newTime, newTime, newTime.GetTimeStep());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 *
 * \bug Using a global static is risky in here, perhaps the return
 *      value should be a value instead of a reference instead?
 *      What does the last else-branch do anyway?
 */
// ----------------------------------------------------------------------

const NFmiMetTime &NFmiTimeDescriptor::ValidTime() const
{
  try
  {
    if (itsTimeBagIdent)
      return (itsTimeList ? *itsTimeList->Current() : itsValidTimeBag->CurrentTime());

    if (itsTimeList)
    {
      itsTimeList->Reset();
      itsTimeList->Next();
      return *itsTimeList->Current();
    }

    // Origintimebag systeemejä ei tietääkseni käytetä missään, joten tätä ei ole testattu.
    // TÄMÄ ei ole thread safetya koodia!!!!
    static NFmiMetTime dummyTime;
    dummyTime = itsOriginTimeBag->CurrentTime();
    dummyTime.NextMetTime(static_cast<short int>(
        itsValidTimeBag->FirstTime().DifferenceInMinutes(itsOriginTimeBag->FirstTime())));
    return dummyTime;
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

unsigned long NFmiTimeDescriptor::Index() const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return (itsTimeList ? itsTimeList->Index() : itsValidTimeBag->CurrentIndex());

      return static_cast<unsigned long>(-1);
    }

    if (IsOriginTime())
      return (itsOriginTimeBag->CurrentIndex());

    return static_cast<unsigned long>(-1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \®eturn Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiTimeDescriptor::Size() const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
      {
        if (itsTimeList != nullptr)
          return itsTimeList->NumberOfItems();

        if (itsValidTimeBag != nullptr)
          return itsValidTimeBag->GetSize();
      }
      return static_cast<unsigned long>(0);
    }

    if (itsOriginTimeBag && IsOriginTime())
      return (itsOriginTimeBag->GetSize());

    return static_cast<unsigned long>(0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeDescriptor::IsEmpty() const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
      {
        if (itsTimeList)
          return itsTimeList->NumberOfItems() == 0;

        return itsValidTimeBag->IsEmpty();
      }
    }
    else
    {
      if (itsOriginTimeBag && IsOriginTime())
        return itsOriginTimeBag->IsEmpty();
    }
    return true;  // Jos ei kerran ole mitään aikarakennetta, niin on tämä kait sitten tyhjä
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

unsigned long NFmiTimeDescriptor::SizeActive() const
{
  try
  {
    if (itsTimeBagIdent && IsValidTime())
    {
      unsigned long num = 0;
      for (int i = 0; i < static_cast<int>(itsValidTimeBag->GetSize()); i++)
      {
        if (itsActivity[i])
          num++;
      }
      return num;
    }

    return static_cast<unsigned long>(0);
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

NFmiTimeBag NFmiTimeDescriptor::GetActivePeriod()
{
  try
  {
    NFmiTime saveTime(Time());

    NFmiTime firstTime, lastTime;

    if (FirstActive())
    {
      firstTime = Time();
      if (LastActive())
      {
        lastTime = Time();
        NFmiTimeBag bag(firstTime, lastTime, ActiveResolution());
        Time(saveTime);
        return bag;
      }
    }
    NFmiTimeBag bag;
    Time(saveTime);
    return bag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param thePeriod Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeDescriptor::SetActivePeriod(bool theActivityState, const NFmiTimeBag &thePeriod)
{
  try
  {
    bool tempBoolean = false;

    NFmiTime saveTime = Time();

    for (int i = 0; i < static_cast<int>(itsValidTimeBag->GetSize()); i++)
      itsActivity[i] = false;

    NFmiTimeBag tempBag(thePeriod);
    tempBag.Reset();

    while (tempBag.Next())
    {
      if (Time(tempBag.CurrentTime()))
      {
        SetActivity(theActivityState);
        tempBoolean = true;
      }
    }

    Time(saveTime);
    return tempBoolean;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeDescriptor::SetActivity(bool theActivityState)
{
  try
  {
    bool temp = itsActivity[Index()];
    itsActivity[Index()] = theActivityState;
    return temp;
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

bool NFmiTimeDescriptor::FirstActive()
{
  try
  {
    Reset();
    return NextActive();
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

bool NFmiTimeDescriptor::LastActive()
{
  try
  {
    TFmiTime lastActive(Time());
    Reset();
    bool tempBoolean = false;
    while (Next())
    {
      if (IsActive())
      {
        tempBoolean = true;
        lastActive = Time();
      }
    }
    Time(lastActive);
    return tempBoolean;
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

bool NFmiTimeDescriptor::PreviousActive()
{
  try
  {
    while (Previous())
      if (IsActive())
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeDescriptor::NextActive()
{
  try
  {
    while (Next())
      if (IsActive())
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTimePerioid NFmiTimeDescriptor::ActiveResolution()
{
  try
  {
    if (itsValidTimeBag)
    {
      NFmiTime saveTime = Time();
      NFmiTime startTime, endTime;

      if (FirstActive())
      {
        startTime = Time();
        if (NextActive())
        {
          endTime = Time();
        }
      }
      Time(saveTime);
      NFmiTimePerioid period(endTime.DifferenceInMinutes(startTime));
      return period;
    }

    return 0;
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

const NFmiTimePerioid NFmiTimeDescriptor::Resolution() const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return itsTimeList ? NFmiTimePerioid(itsTimeList->CurrentResolution())
                           : itsValidTimeBag->Resolution();
      return 0;
    }

    if (IsOriginTime())
      return (itsOriginTimeBag->Resolution());

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theTimeDescriptor The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor &NFmiTimeDescriptor::operator=(const NFmiTimeDescriptor &theTimeDescriptor)
{
  try
  {
    Destroy();

    itsOriginTimeBag = theTimeDescriptor.itsOriginTimeBag
                           ? new NFmiTimeBag(*theTimeDescriptor.itsOriginTimeBag)
                           : nullptr;
    itsValidTimeBag = theTimeDescriptor.itsValidTimeBag
                          ? new NFmiTimeBag(*theTimeDescriptor.itsValidTimeBag)
                          : nullptr;
    itsTimeList =
        theTimeDescriptor.itsTimeList ? new NFmiTimeList(*theTimeDescriptor.itsTimeList) : nullptr;
    itsTimeBagIdent = theTimeDescriptor.itsTimeBagIdent;

    itsActivity = new bool[theTimeDescriptor.Size()];
    for (int i = 0; i < static_cast<int>(theTimeDescriptor.Size()); i++)
      itsActivity[i] = theTimeDescriptor.itsActivity[i];

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theTimeDescriptor The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiTimeDescriptor::operator==(const NFmiTimeDescriptor &theTimeDescriptor) const
{
  try
  {
    bool retVal = true;
    if (this->itsTimeBagIdent && theTimeDescriptor.itsTimeBagIdent)
    {
      if (!(this->itsTimeBagIdent == theTimeDescriptor.itsTimeBagIdent))
        return false;
    }
    if (this->itsIsInterpolation && theTimeDescriptor.itsIsInterpolation)
    {
      if (!(this->itsIsInterpolation == theTimeDescriptor.itsIsInterpolation))
        return false;
    }
    if (this->itsIsOriginLastest && theTimeDescriptor.itsIsOriginLastest)
    {
      if (!(this->itsIsOriginLastest == theTimeDescriptor.itsIsOriginLastest))
        return false;
    }
    if (this->itsLocalTimeStep && theTimeDescriptor.itsLocalTimeStep)
    {
      if (!(this->itsLocalTimeStep == theTimeDescriptor.itsLocalTimeStep))
        return false;
    }
    if (this->itsIsLocalTime && theTimeDescriptor.itsIsLocalTime)
    {
      if (!(this->itsIsLocalTime == theTimeDescriptor.itsIsLocalTime))
        return false;
    }
    if (this->itsValidTimeBag && theTimeDescriptor.itsValidTimeBag)
    {
      if (!(*(this->itsOriginTimeBag) == *(theTimeDescriptor.itsOriginTimeBag) &&
            *(this->itsValidTimeBag) == *(theTimeDescriptor.itsValidTimeBag)))
        return false;
    }
    else if (this->itsTimeList && theTimeDescriptor.itsTimeList)
    {
      if (!(*(this->itsTimeList) == *(theTimeDescriptor.itsTimeList)))
        return false;
    }
    return retVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theCombine This-otus ja theCombine yhdistetään halutulla tavalla.
 * \param theStartTimeFunction Mistä otetaan alkuaika, jos 0, pienempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \param theEndTimeFunction Mistä otetaan loppuaika, jos 0, suurempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \return Palautetaan yhdistetty timedescriptor.
 */
// ----------------------------------------------------------------------

NFmiTimeDescriptor NFmiTimeDescriptor::Combine(const NFmiTimeDescriptor &theCombine,
                                               int theStartTimeFunction,
                                               int theEndTimeFunction) const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (itsValidTimeBag && theCombine.ValidTimeBag())
      {
        return NFmiTimeDescriptor(
            itsOriginTimeBag->FirstTime(),
            itsValidTimeBag->Combine(
                *(theCombine).itsValidTimeBag, theStartTimeFunction, theEndTimeFunction),
            itsIsLocalTime,
            itsIsInterpolation);
      }
      else if (itsTimeList && theCombine.itsTimeList)  // yhdistetään aikalistat
      {
        NFmiTimeList timeList = itsTimeList->Combine(
            *(theCombine.itsTimeList), theStartTimeFunction, theEndTimeFunction);
        return NFmiTimeDescriptor(
            itsOriginTimeBag->FirstTime(), timeList, itsIsLocalTime, itsIsInterpolation);
      }
      else             // timebagi ja timelist sekaisin, ei voi yhdistää
        return *this;  // hätä ratkaisu, palautetaan this:in kopio kun ei voi yhdistää
    }

    return NFmiTimeDescriptor(
        itsOriginTimeBag->Combine(
            *(theCombine).itsOriginTimeBag, theStartTimeFunction, theEndTimeFunction),
        itsValidTimeBag->FirstTime());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

// Aktiivisuustila:
// Toistaiseksi lukemisen yhteydessä varataan dynaamisen muisti sitä varten ja alustetaan
// vakioilla 'true'; edelleen aktiivisuustilaa ei kirjoiteta Write()-metodissa

std::ostream &NFmiTimeDescriptor::Write(std::ostream &file) const
{
  try
  {
    // Jos aikaa kasitellää listatyyppisenä, kirjoitetaan aikalista
    // Muussa tapauksessa kirjoitetaan TimeBag

    if (itsTimeList)
    {
      file << "1 " << ClassName() << std::endl;  // 1 NFmiTimeDescriptor
      file << *itsTimeList;
    }
    else
    {
      file << "0 " << ClassName() << std::endl;  // 0 NFmiTimeDescriptor
      file << *itsValidTimeBag;
    }

    file << *itsOriginTimeBag;

    file << static_cast<unsigned long>(itsIsLocalTime) << " "  // Onko paikallista aikaa
         << itsIsInterpolation << " "                          // Interpoloidaanko
         << itsIsOriginLastest << " "  // Haetaanko viimeisimmän tuotteen perusteella
         << itsTimeBagIdent << " "     // Hakutapa
         << itsLocalTimeStep << " "    // Varalla
         << 0 << " "                   // Varalla
         << 0 << std::endl;            // Varalla

    if (itsValidTimeBag)
    {
      for (int i = 0; i < static_cast<int>(itsValidTimeBag->GetSize()); i++)
        file << itsActivity[i] << " ";
    }
    else if (itsTimeList)
    {
      for (int i = 0; i < static_cast<int>(itsTimeList->NumberOfItems()); i++)
        file << itsActivity[i] << " ";
    }

    file << std::endl;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiTimeDescriptor::Read(std::istream &file)
{
  try
  {
    unsigned short theTimeListIdent;
    char dirty[30];

    Destroy();

    itsOriginTimeBag = new NFmiTimeBag;

    file >> theTimeListIdent >> dirty;  // 0->TimeBag,	1->TimeList

    if (theTimeListIdent)  // 1
    {
      itsTimeList = new NFmiTimeList;
      file >> *itsTimeList;
    }
    else  // 0
    {
      itsValidTimeBag = new NFmiTimeBag;
      file >> *itsValidTimeBag;
    }

    file >> *itsOriginTimeBag;

    unsigned long theIsLocalTime;
    unsigned long dummy;

    file >> theIsLocalTime >> itsIsInterpolation >> itsIsOriginLastest >> itsTimeBagIdent >>
        itsLocalTimeStep >> dummy >> dummy;

    itsIsLocalTime = FmiTimeLocalzation(theIsLocalTime);

    if (itsTimeBagIdent == true)
    {
      {
        auto theSize = static_cast<int>(itsValidTimeBag ? itsValidTimeBag->GetSize()
                                                        : itsTimeList->NumberOfItems());
        itsActivity = new bool[theSize];
        // We trust everything to be at least version 6 by now
        for (int i = 0; i < theSize; i++)
          if (DefaultFmiInfoVersion >= 3)
            file >> itsActivity[i];
          else
            itsActivity[i] = true;
      }
    }
    else
    {
      if (itsOriginTimeBag != nullptr)
      {
        itsActivity = new bool[static_cast<int>(itsOriginTimeBag->GetSize())];
        // We trust everything to be at least version 6 by now
        for (int i = 0; i < static_cast<int>(itsOriginTimeBag->GetSize()); i++)
          if (DefaultFmiInfoVersion >= 3)
            file >> itsActivity[i];
          else
            itsActivity[i] = true;
      }
      else
        itsActivity = nullptr;
    }

    if (itsValidTimeBag != nullptr)
      itsValidTimeBag->Reset();
    else if (itsTimeList != nullptr)
      itsTimeList->Reset();

    if (itsOriginTimeBag != nullptr)
      itsOriginTimeBag->Reset();

    return file;
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

bool NFmiTimeDescriptor::Time(const unsigned long &theIndex)
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
      {
        if (itsTimeList)
          return itsTimeList->Index(theIndex);

        return itsValidTimeBag->SetTime(theIndex);
      }
    }
    else
    {
      if (IsOriginTime())
        itsOriginTimeBag->SetTime(theIndex);
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
 * \param theLongitude Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::SetLocalTimes(const float theLongitude)
{
  try
  {
    if (TimeLocalzation() == kUTC)
    {
      itsIsLocalTime = kLongitude;

      NFmiTimeBag *theLocalTimeBag =
          new NFmiTimeBag(itsValidTimeBag->FirstTime().LocalTime(theLongitude),
                          itsValidTimeBag->LastTime().LocalTime(theLongitude),
                          itsValidTimeBag->Resolution());

      delete itsValidTimeBag;

      itsValidTimeBag = theLocalTimeBag;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

// ----------------------------------------------------------------------
/*!
 * Tekee sellaisen timedescriptorin, jossa on ajat halutulta väliltä
 * reunat mukaan lukien, ja joka palautetaan ulos.
 * Jos rajat ovat ohi kokonaan timedescriptorista ja vielä samaan suuntaan,
 * palautetaan tyhjä timedescriptor. Samoin käy jos rajojen sisään ei sovi yhtään
 * timedescriptor aikaa.
 * \param theStartLimit Leikkausta rajoittava alkuaika.
 * \param theEndLimit Leikkausta rajoittava loppuaika.
 * \return Palauttaa tulos timedescriptorin.
 */
// ----------------------------------------------------------------------
NFmiTimeDescriptor NFmiTimeDescriptor::GetIntersection(const NFmiMetTime &theStartLimit,
                                                       const NFmiMetTime &theEndLimit) const
{
  try
  {
    NFmiTimeDescriptor timedesc(*this);
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
      {
        if (itsTimeList)
        {
          *(timedesc.ValidTimeList()) = itsTimeList->GetIntersection(theStartLimit, theEndLimit);
        }
        else
        {
          *(timedesc.ValidTimeBag()) = itsValidTimeBag->GetIntersection(theStartLimit, theEndLimit);
        }
      }
    }
    else
    {
      if (IsOriginTime())
      {
        *(timedesc.OriginTimeBag()) = itsOriginTimeBag->GetIntersection(theStartLimit, theEndLimit);
      }
    }
    return timedesc;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeDescriptor::IsInside(const NFmiMetTime &theTime) const
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return itsTimeList ? itsTimeList->IsInside(theTime) : itsValidTimeBag->IsInside(theTime);

      return false;
    }

    if (IsOriginTime())
      return itsOriginTimeBag->IsInside(theTime);

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeDescriptor::FindNearestTime(const NFmiMetTime &theTime,
                                         FmiDirection theDirection,
                                         unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        return itsTimeList
                   ? itsTimeList->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes)
                   : itsValidTimeBag->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes);

      return false;
    }

    if (IsOriginTime())
      return itsOriginTimeBag->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes);

    return false;
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
void NFmiTimeDescriptor::PruneTimes(int theMaxTimeCount, bool fFromEnd)
{
  try
  {
    if (itsTimeBagIdent)
    {
      if (IsValidTime())
        itsTimeList ? itsTimeList->PruneTimes(theMaxTimeCount, fFromEnd)
                    : itsValidTimeBag->PruneTimes(theMaxTimeCount, fFromEnd);
    }
    else
    {
      if (IsOriginTime())
        itsOriginTimeBag->PruneTimes(theMaxTimeCount, fFromEnd);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Muuttaa timedescriptoria niin että annettu aika laitetaan firstTimeksi,
 * ja lastTimeksi lasketaan uusi arvo, niin että erotus ensimmäisen ja
 * viimeisen ajan välillä pysyy samana.
 * Timebagin tai timelistin rakenne ja resoluutio säilyvät.
 * Lisäksi currentTime:ksi asetetaan theTime arvo eli 1. aika.
 * Myös originTimeksi laitetaan theTime.
 * \param theTime on timeDescriptorin uusi firstTime.
 */
// ----------------------------------------------------------------------

void NFmiTimeDescriptor::SetNewStartTime(const NFmiMetTime &theTime)
{
  try
  {
    if (ValidTimeBag())
    {
      NFmiTimeBag newTimeBag = *(ValidTimeBag());
      newTimeBag.SetNewStartTime(theTime);
      *this = NFmiTimeDescriptor(theTime, newTimeBag);
    }
    else if (ValidTimeList())
    {
      NFmiTimeList newTimeList = *ValidTimeList();
      newTimeList.SetNewStartTime(theTime);
      *this = NFmiTimeDescriptor(theTime, newTimeList);
    }
    Time(theTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
