// ======================================================================
/*!
 * \file NFmiCalculatedAreaMask.cpp
 * \brief Implementation of class NFmiCalculatedAreaMask
 */
// ======================================================================
/*!
 * \class NFmiCalculatedAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*
 * \class NFmiLatLonAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiElevationAngleAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiJulianDayAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiLocalHourAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculatedAreaMask.h"
#include "NFmiArea.h"
#include "NFmiDataIdent.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"
#include "NFmiLocation.h"
#include "NFmiPoint.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::~NFmiCalculatedAreaMask()
{
  try
  {
    delete itsDataIdent;
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

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask() : NFmiAreaMaskImpl(), itsDataIdent(nullptr) {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask(const NFmiDataIdent& theParam,
                                               const NFmiCalculationCondition& theOperation)
    : NFmiAreaMaskImpl(theOperation, kCalculated, NFmiInfoData::kCalculatedValue, kNoValue),
      itsDataIdent(new NFmiDataIdent(theParam))
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask(const NFmiCalculatedAreaMask& theMask)
    : NFmiAreaMaskImpl(theMask),
      itsDataIdent(theMask.itsDataIdent ? new NFmiDataIdent(*theMask.itsDataIdent) : nullptr)
{
}

NFmiAreaMask* NFmiCalculatedAreaMask::Clone() const
{
  try
  {
    return new NFmiCalculatedAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask& NFmiCalculatedAreaMask::operator=(const NFmiCalculatedAreaMask& theMask)
{
  try
  {
    if (this != &theMask)
    {
      NFmiAreaMaskImpl::operator=(theMask);
      itsDataIdent = theMask.itsDataIdent ? new NFmiDataIdent(*theMask.itsDataIdent) : nullptr;
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCalculatedAreaMask::IsWantedParam(const NFmiDataIdent& theParam,
                                           const NFmiLevel* /* theLevel */) const
{
  try
  {
    if (itsDataIdent && (*itsDataIdent == theParam))
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

const NFmiDataIdent* NFmiCalculatedAreaMask::DataIdent() const
{
  try
  {
    return itsDataIdent;
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

const NFmiParam* NFmiCalculatedAreaMask::Param() const
{
  try
  {
    if (itsDataIdent)
      return itsDataIdent->GetParam();

    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask() : NFmiCalculatedAreaMask() {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask(const NFmiDataIdent& theParam,
                                       const NFmiCalculationCondition& theOperation)
    : NFmiCalculatedAreaMask(theParam, theOperation)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask(const NFmiLatLonAreaMask& theMask) = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::~NFmiLatLonAreaMask() = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiLatLonAreaMask::Clone() const
{
  try
  {
    return new NFmiLatLonAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLatLonAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  try
  {
    if (itsDataIdent->GetParamIdent() == kFmiLatitude)
      return theLatLon.Y();
    else
      return theLatLon.X();
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

const NFmiString NFmiLatLonAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiLatLonAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask() : NFmiLatLonAreaMask(), itsTime() {}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask(const NFmiElevationAngleAreaMask& theMask) =
    default;


// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask(const NFmiDataIdent& theParam,
                                                       const NFmiCalculationCondition& theOperation)
    : NFmiLatLonAreaMask(theParam, theOperation), itsTime()
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::~NFmiElevationAngleAreaMask() = default;

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiElevationAngleAreaMask::Time(const NFmiMetTime& theTime)
{
  try
  {
    itsTime = theTime;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiElevationAngleAreaMask::Clone() const
{
  try
  {
    return new NFmiElevationAngleAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiElevationAngleAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  try
  {
    if (fUseTimeInterpolationAlways)
      itsTime = theCalculationParams.itsTime;  // tämä on pakko tehdä kun käytetään esim.
                                               // SumT-tyyppisiä SmartTool-funktioita ja niiden
                                               // argumentteina on annettu tietyt argumentit jotka
                                               // ovat ajasta riippuvia.
    return CalcValueFromLocation(theCalculationParams.UsedLatlon());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiElevationAngleAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  try
  {
    NFmiLocation location(theLatLon);
    return location.ElevationAngle(itsTime);
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

const NFmiString NFmiElevationAngleAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiElevationAngleAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask() : NFmiElevationAngleAreaMask(), itsJulianDay(0) {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask(const NFmiDataIdent& theParam,
                                             const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsJulianDay(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask(const NFmiJulianDayAreaMask& theMask) = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::~NFmiJulianDayAreaMask() = default;

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiJulianDayAreaMask::Time(const NFmiMetTime& theTime)
{
  try
  {
    NFmiElevationAngleAreaMask::Time(theTime);
    itsJulianDay = itsTime.GetJulianDay();
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiJulianDayAreaMask::Clone() const
{
  try
  {
    return new NFmiJulianDayAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiJulianDayAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    return itsJulianDay;  // tämä on jo laskettu Time-metodissa
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

const NFmiString NFmiJulianDayAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiJulianDayAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask() : NFmiElevationAngleAreaMask() {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask(const NFmiDataIdent& theParam,
                                             const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask(const NFmiLocalHourAreaMask& theMask) = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::~NFmiLocalHourAreaMask() = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiLocalHourAreaMask::Clone() const
{
  try
  {
    return new NFmiLocalHourAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLocalHourAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  try
  {
    NFmiTime locTime(itsTime.LocalTime(NFmiLocation(theLatLon)));
    return locTime.GetHour();
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

const NFmiString NFmiLocalHourAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiLocalHourAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

NFmiUtcHourAreaMask::~NFmiUtcHourAreaMask() = default;

NFmiUtcHourAreaMask::NFmiUtcHourAreaMask() : NFmiElevationAngleAreaMask() {}

NFmiUtcHourAreaMask::NFmiUtcHourAreaMask(const NFmiUtcHourAreaMask& theMask) = default;

NFmiUtcHourAreaMask::NFmiUtcHourAreaMask(const NFmiDataIdent& theParam,
                                         const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}

NFmiAreaMask* NFmiUtcHourAreaMask::Clone() const
{
  try
  {
    return new NFmiUtcHourAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiUtcHourAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    return itsTime.GetHour();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiString NFmiUtcHourAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiUtcHourAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

NFmiMinuteAreaMask::~NFmiMinuteAreaMask() = default;

NFmiMinuteAreaMask::NFmiMinuteAreaMask() : NFmiElevationAngleAreaMask() {}

NFmiMinuteAreaMask::NFmiMinuteAreaMask(const NFmiMinuteAreaMask& theMask) = default;

NFmiMinuteAreaMask::NFmiMinuteAreaMask(const NFmiDataIdent& theParam,
                                       const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}

NFmiAreaMask* NFmiMinuteAreaMask::Clone() const
{
  try
  {
    return new NFmiMinuteAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiMinuteAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    return itsTime.GetMin();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiString NFmiMinuteAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiMinuteAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(
    const std::shared_ptr<NFmiFastQueryInfo>& theInfo)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo Undocumented
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(
    const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
    const NFmiDataIdent& theParam,
    const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(const NFmiForecastHourAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask), itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask& NFmiForecastHourAreaMask::operator=(
    const NFmiForecastHourAreaMask& theMask)
{
  try
  {
    if (this != &theMask)
    {
      NFmiElevationAngleAreaMask::operator=(theMask);
      itsInfo = NFmiAreaMask::DoShallowCopy(theMask.itsInfo);
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::~NFmiForecastHourAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiForecastHourAreaMask::Clone() const
{
  try
  {
    return new NFmiForecastHourAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiForecastHourAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    return itsTime.DifferenceInHours(itsInfo->TimeDescriptor().FirstTime());
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

const NFmiString NFmiForecastHourAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiForecastHourAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiTimeStepAreaMask::~NFmiTimeStepAreaMask() = default;

NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo)
{
}

NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const NFmiTimeStepAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask), itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo))
{
}

NFmiTimeStepAreaMask& NFmiTimeStepAreaMask::operator=(const NFmiTimeStepAreaMask& theMask)
{
  try
  {
    if (this != &theMask)
    {
      NFmiElevationAngleAreaMask::operator=(theMask);
      itsInfo = NFmiAreaMask::DoShallowCopy(theMask.itsInfo);
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           const NFmiDataIdent& theParam,
                                           const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo)
{
}

NFmiAreaMask* NFmiTimeStepAreaMask::Clone() const
{
  try
  {
    return new NFmiTimeStepAreaMask(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTimeStepAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    // palauttaa datan currentin ajan aikaresoluution tunneissa
    return (itsInfo->TimeResolution() / 60.);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiString NFmiTimeStepAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiTimeStepAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiTimeStepAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                   bool fUseTimeInterpolationAlways)
{
  try
  {
    if (fUseTimeInterpolationAlways)
      itsInfo->Time(theCalculationParams
                        .itsTime);  // asetetaan ensin aika kohdalleen ja sitten kysytään arvoa
    else
      itsInfo->TimeIndex(
          theCalculationParams
              .itsTimeIndex);  // asetetaan ensin aikaindeksi kohdalleen ja sitten kysytään arvoa
    return CalcValueFromLocation(
        theCalculationParams.UsedLatlon());  // ollaan jo oikeassa ajassa, ei aikainterpolointia
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiGridSizeAreaMask    *************************
// **********************************************************

NFmiGridSizeAreaMask::~NFmiGridSizeAreaMask() = default;

NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           bool calcXValue)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo), fCalcXValue(calcXValue)
{
}

NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           const NFmiDataIdent& theParam,
                                           const NFmiCalculationCondition& theOperation,
                                           bool calcXValue)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo), fCalcXValue(calcXValue)
{
}

NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const NFmiGridSizeAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask),
      itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo)),
      fCalcXValue(theMask.fCalcXValue)
{
}

NFmiAreaMask* NFmiGridSizeAreaMask::Clone() const
{
  return new NFmiGridSizeAreaMask(*this);
}
double NFmiGridSizeAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                   bool /* fUseTimeInterpolationAlways */)
{
  try
  {
    return CalcValueFromLocation(theCalculationParams.UsedLatlon());  // ajalla ei ole väliä
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiGridSizeAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  try
  {
    if (itsInfo->IsGrid())
    {
      if (fCalcXValue)
        return itsInfo->Area()->WorldXYWidth() / (itsInfo->Grid()->XNumber() - 1);
      else
        return itsInfo->Area()->WorldXYHeight() / (itsInfo->Grid()->YNumber() - 1);
    }
    return kFloatMissing;

    /*
            // Tämä saattaisi olla maantieteellisesti oikea lasku tapa, mutta teen simppelin
       jakolaskun
            if(itsInfo->IsGrid() && itsInfo->Location(theLatLon))
            {
                    NFmiPoint gridPoint1(itsInfo->Grid()->GridPoint());
                    NFmiPoint gridPoint2(gridPoint1);
                    if(fCalcXValue)
                            gridPoint2.X((itsInfo->Grid()->XNumber() < gridPoint2.X()) ?
       gridPoint2.X()+1 : gridPoint2.X()-1);
                    else
                            gridPoint2.Y((itsInfo->Grid()->YNumber() < gridPoint2.Y()) ?
       gridPoint2.Y()+1 : gridPoint2.Y()-1);
                    NFmiPoint latlon1(itsInfo->Grid()->GridToLatLon(gridPoint1));
                    NFmiPoint latlon2(itsInfo->Grid()->GridToLatLon(gridPoint2));
                    NFmiLocation loc1(latlon1);
                    return loc1.Distance(latlon2);
            }
            return kFloatMissing;
    */
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiString NFmiGridSizeAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiTimeStepAreaMask::MakeSubMaskString");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// **********************************************************
// *****    NFmiGridSizeAreaMask    *************************
// **********************************************************
