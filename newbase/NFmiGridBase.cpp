// ======================================================================
/*!
 * \file NFmiGridBase.cpp
 * \brief Implementation of class NFmiGridBase
 */
// ======================================================================
/*!
 * \class NFmiGridBase
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiGridBase.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <cstring>
#include <iostream>
// memcpy()

using namespace std;

const double kFmiEps = 0.0001;

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 */
// ----------------------------------------------------------------------

NFmiGridBase::NFmiGridBase(const NFmiGridBase &theGridBase)
    : itsData(theGridBase.itsData ? new NFmiDataPool(*(theGridBase.itsData)) : nullptr),
      itsFloatData(nullptr),
      itsStartingCorner(theGridBase.itsStartingCorner),
      itsInterpolationMethod(theGridBase.itsInterpolationMethod),
      itsXNumber(theGridBase.itsXNumber),
      itsYNumber(theGridBase.itsYNumber),
      itsCurrentX(theGridBase.itsCurrentX),
      itsCurrentY(theGridBase.itsCurrentY)
      //  , itsBase(theGridBase.itsBase)
      ,
      fWaitX(theGridBase.fWaitX),
      fWaitY(theGridBase.fWaitY),
      itsFirstX(theGridBase.itsFirstX),
      itsFirstY(theGridBase.itsFirstY),
      itsLastX(theGridBase.itsLastX),
      itsLastY(theGridBase.itsLastY),
      itsPreviousX(theGridBase.itsPreviousX),
      itsPreviousY(theGridBase.itsPreviousY),
      itsTopLeftValue(theGridBase.itsTopLeftValue),
      itsTopRightValue(theGridBase.itsTopRightValue),
      itsBottomLeftValue(theGridBase.itsBottomLeftValue),
      itsBottomRightValue(theGridBase.itsBottomRightValue)
{
  try
  {
    // itsFloatData array of floats will NOT be copied

    if (itsData)
      itsData->InitMissingValues(*theGridBase.DataPool());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Init(NFmiDataPool *theData)
{
  try
  {
    if (Size() != theData->Number())
      return false;

    if (itsData)
      delete itsData;

    itsData = new NFmiDataPool(*theData);

    itsData->First();

    if (!Swap())
      return false;

    itsData->InitMissingValues(*theData);

    return ResetCrop();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Muuttaa parametrina annetun gridin halutessa tiheämpään/väljempään
 * hilaan lineaarisesti interpoloiden, alueiden oletetaan olevan samoja.
 *
 * \param theData Lähtödata, josta initialisoidaan uusi hila (=this olio).
 * \return True jos ei ongelmia, muuten false.
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Init(NFmiGridBase *theData)
{
  try
  {
    theData->Reset();

    float theXScaleFactor =
        (static_cast<float>(theData->XNumber() - 1)) / (static_cast<float>(XNumber() - 1));
    float theYScaleFactor =
        (static_cast<float>(theData->YNumber() - 1)) / (static_cast<float>(YNumber() - 1));

    if (itsData)
      delete itsData;

    itsData = new NFmiDataPool();

    if (!itsData->InitMissingValues(*(theData->DataPool())))
      return false;

    if ((theXScaleFactor == 1.0) && (theYScaleFactor == 1.0))
    {
      // Gridit saman kokoisia hiloja --> voidaan kopioida suoraan interpoloimatta

      if (!itsData->Init(Size(), theData->DataPool()->Data()))
        return false;

      return true;
    }

    if (!itsData->Init(Size()))
      return false;

    if ((theData->itsInterpolationMethod == kLinearlyFast))
    {
      // HUOM! - ei ota huomioon "croppausta", ts. toimii aina KOKO gridille, ei pelkästään sen
      // osagridille.
      //		 - interpoloitavan gridin datatyypiksi oletetaan ainoastaan float

      int x = 0;
      int y = 0;
      double dx = 0.;
      double dy = 0.;
      long index = 0;
      long numberOfGridItems = Size();
      int xRes = XNumber();
      int yRes = YNumber();
      double value = 0.;

      // Create a array of float data just to avoid "item-by-item" type casting

      if (itsFloatData)
        delete[] itsFloatData;

      itsFloatData = new float[numberOfGridItems];

      do
      {
        value = 0;
        dx = static_cast<double>(x) * theXScaleFactor;
        dy = static_cast<double>(y) * theYScaleFactor;

        if (!theData->FastBiVariateInterpolation(dx, dy, value))
          return false;

        itsFloatData[index] = static_cast<float>(value);

        x++;
        if (x == xRes)
        {
          x = 0;
          y++;
        }
      } while ((y < yRes) && (++index < numberOfGridItems));

      // Update the data pool with interpolated data array
      if (!itsData->Init(numberOfGridItems, itsFloatData))
        return false;
    }
    else
    {
      ResetCrop();
      while (Next())
      {
        //		  double x_ = (itsCurrentX - itsBase) * theXScaleFactor + theData->Base();
        double x_ = itsCurrentX * theXScaleFactor;
        //		  double y_ = (itsCurrentY - itsBase) * theYScaleFactor + theData->Base();
        double y_ = itsCurrentY * theYScaleFactor;

        double value = 0;
        theData->InterpolateToGridPoint(
            (theData->itsFirstX + x_), (theData->itsFirstY + y_), value);  // Salla 180898

        FloatValue(value);
      }
    }

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
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::First()
{
  try
  {
    Setf(false);
    return GridPoint(itsFirstX, itsFirstY);
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

bool NFmiGridBase::Reset()
{
  try
  {
    First();
    Setf(true);
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
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Last()
{
  try
  {
    Setf(false);
    return GridPoint(itsLastX, itsLastY);
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

bool NFmiGridBase::ResetLast()
{
  try
  {
    Last();
    Setf(true);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::GridPoint(double x, double y)
{
  try
  {
    if (!NearestGridPoint(x, y))
      return false;

    itsCurrentX = static_cast<unsigned long>(x);
    itsCurrentY = static_cast<unsigned long>(y);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Next(unsigned long numberOfSteps)
{
  try
  {
    if (!(fWaitX && fWaitY))
    {
      //	  if(Index() - itsBase + numberOfSteps > (Size()-1))
      if (Index() + numberOfSteps > (Size() - 1))
        return false;
      long index = Index();
      CalcX(index + numberOfSteps);
      CalcY(index + numberOfSteps);
      return true;
    }
    Setf(false);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Previous(unsigned long numberOfSteps)
{
  try
  {
    if (!(fWaitX && fWaitY))
    {
      //	  if(static_cast<int>(Index() - itsBase - numberOfSteps) < 0)
      if (static_cast<int>(Index() - numberOfSteps) < 0)
        return false;

      long index = Index();
      CalcX(index - numberOfSteps);
      CalcY(index - numberOfSteps);
      return true;
    }
    Setf(false);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Left(unsigned long numberOfSteps)
{
  try
  {
    if (!fWaitX)
    {
      if (!IsInsideX(-static_cast<signed long>(numberOfSteps)))
        return false;

      GridPoint(itsCurrentX - numberOfSteps, itsCurrentY);
      return true;
    }
    fWaitX = false;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Right(unsigned long numberOfSteps)
{
  try
  {
    if (!fWaitX)
    {
      if (!IsInsideX(numberOfSteps))
        return false;

      GridPoint(itsCurrentX + numberOfSteps, itsCurrentY);
      return true;
    }
    fWaitX = false;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Up(unsigned long numberOfSteps)
{
  try
  {
    if (!fWaitY)
    {
      if (!IsInsideY(numberOfSteps))
        return false;

      GridPoint(itsCurrentX, itsCurrentY + numberOfSteps);
      return true;
    }
    fWaitY = false;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Down(unsigned long numberOfSteps)
{
  try
  {
    if (!fWaitY)
    {
      if (!IsInsideY(-static_cast<signed long>(numberOfSteps)))
        return false;

      GridPoint(itsCurrentX, itsCurrentY - numberOfSteps);
      return true;
    }
    fWaitY = false;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 */
// ----------------------------------------------------------------------

void NFmiGridBase::CalcX(unsigned long theIndex)
{
  try
  {
    //  long theX = (theIndex-itsBase) % XNumber() + itsBase;
    long theX = theIndex % XNumber();
    GridPoint(CalcFirstX() + theX, itsCurrentY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 */
// ----------------------------------------------------------------------

void NFmiGridBase::CalcY(unsigned long theIndex)
{
  try
  {
    //  long theY = (theIndex-itsBase) / XNumber() + itsBase;
    long theY = theIndex / XNumber();
    GridPoint(itsCurrentX, CalcFirstY() + theY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGridBase::GridPoint(unsigned long theLocationIndex) const
{
  try
  {
    // HUOM!!! En tiedä varmana, miten croppaus pitäisi ottaa huomioon tässä laskussa!!! Marko
    //  long theX = CalcFirstX() + ((theLocationIndex-itsBase) % XNumber() + itsBase);
    long theX = CalcFirstX() + (theLocationIndex % XNumber());
    //  long theY = CalcFirstY() + ((theLocationIndex-itsBase) / XNumber() + itsBase);
    long theY = CalcFirstY() + (theLocationIndex / XNumber());
    return NFmiPoint(theX, theY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeft Undocumented
 * \param theTopRight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::MaximizedCrop(const NFmiPoint &theBottomLeft, const NFmiPoint &theTopRight)
{
  try
  {
    // Truncate the crop window size into the nearest greater integer window size

    itsFirstX = std::max(0, static_cast<int>(trunc(theBottomLeft.X())));
    itsFirstY = std::max(0, static_cast<int>(trunc(theBottomLeft.Y())));

    itsLastX = std::min(OriginalXNumber(), static_cast<unsigned long>(trunc(theTopRight.X() + 1)));
    itsLastY = std::min(OriginalYNumber(), static_cast<unsigned long>(trunc(theTopRight.Y() + 1)));

    Reset();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeft Undocumented
 * \param theTopRight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::MinimizedCrop(const NFmiPoint &theBottomLeft, const NFmiPoint &theTopRight)
{
  try
  {
    if (theBottomLeft.X() == 0. && theBottomLeft.Y() == 0.)
    {
      itsFirstX = 0;
      itsFirstY = 0;
    }
    else
    {
      // Truncate the crop window size into the nearest smaller integer window size
      itsFirstX = std::max(0l, static_cast<long>(trunc(theBottomLeft.X() + 1)));
      itsFirstY = std::max(0l, static_cast<long>(trunc(theBottomLeft.Y() + 1)));
    }

    itsLastX = static_cast<unsigned long>(trunc(theTopRight.X()));
    itsLastY = static_cast<unsigned long>(trunc(theTopRight.Y()));

    Reset();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeft Undocumented
 * \param theTopRight Undocumented
 * \param theMinimizedCrop Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Crop(const NFmiPoint &theBottomLeft,
                        const NFmiPoint &theTopRight,
                        bool theMinimizedCrop)
{
  try
  {
    if (theMinimizedCrop)
      return MinimizedCrop(theBottomLeft, theTopRight);

    return MaximizedCrop(theBottomLeft, theTopRight);
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

bool NFmiGridBase::ResetCrop()
{
  try
  {
    //  const NFmiPoint bl(itsBase,itsBase);
    const NFmiPoint bl(0, 0);
    //  const NFmiPoint tr(itsXNumber - !itsBase,itsYNumber - !itsBase);
    const NFmiPoint tr(itsLastX, itsLastY);
    return Crop(bl, tr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param data Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::FloatValue(double data) const
{
  try
  {
    if (!itsData)
      return false;

    return itsData->FloatValue(DataIndex(), static_cast<float>(data));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param dx Undocumented
 * \param dy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGridBase::FloatValue(long dx, long dy) const
{
  try
  {
    //  if(itsCurrentX + dx < static_cast<unsigned long>(itsBase) || itsCurrentY + dy <
    //  static_cast<unsigned long>(itsBase))
    if (static_cast<long>(itsCurrentX) + dx < 0 || static_cast<long>(itsCurrentY) + dy < 0)
      return kFloatMissing;

    if (!IsInsideGrid(itsCurrentX + dx, itsCurrentY + dy))
      return kFloatMissing;

    if (!itsData)
      return kFloatMissing;

    return static_cast<float>(itsData->IndexFloatValue(
        static_cast<unsigned long>(DataIndex(itsCurrentX + dx, itsCurrentY + dy))));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param gridPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGridBase::FloatValue(const NFmiPoint &gridPoint)
{
  try
  {
    if (!IsInsideGrid(static_cast<unsigned long>(gridPoint.X()),
                      static_cast<unsigned long>(gridPoint.Y())))
      return kFloatMissing;

    return itsData->IndexFloatValue(static_cast<unsigned long>(DataIndex(
        static_cast<unsigned long>(gridPoint.X()), static_cast<unsigned long>(gridPoint.Y()))));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param data Undocumented
 * \param gridPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::FloatValue(double data, const NFmiPoint &gridPoint)
{
  try
  {
    if (!IsInsideGrid(static_cast<unsigned long>(gridPoint.X()),
                      static_cast<unsigned long>(gridPoint.Y())))
      return false;

    if (!itsData)
      return false;

    return itsData->FloatValue(DataIndex(static_cast<unsigned long>(gridPoint.X()),
                                         static_cast<unsigned long>(gridPoint.Y())),
                               static_cast<float>(data));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param point Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::IsInsideOriginalGrid(double xGrid, double yGrid) const
{
  try
  {
    if (!(xGrid + kFmiEps >= 0 && xGrid - kFmiEps <= itsXNumber - 1 && yGrid + kFmiEps >= 0 &&
          yGrid - kFmiEps <= itsYNumber - 1))
      return false;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::NearestGridPoint(double &x_, double &y_) const
{
  try
  {
    if (!IsInsideOriginalGrid(x_, y_))
      return false;

    x_ = ::round(x_);
    y_ = ::round(y_);

    if (!IsInsideGrid(static_cast<unsigned long>(x_), static_cast<unsigned long>(y_)))
      return false;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::InterpolateToGridPoint(double x_,
                                          double y_,
                                          double &theValue,
                                          FmiInterpolationMethod method)
{
  try
  {
    // Testaa alueen, onko sisällä
    if (x_ > itsXNumber - 1 || y_ > itsYNumber - 1)
    {
      theValue = kFloatMissing;
      return false;
    }

    switch (method)
    {
      case kLinearly:
      {
        if (!BiLinearInterpolation(x_, y_, theValue))
        {
          theValue = kFloatMissing;
          return false;
        }
        return true;
      }

      case kNearestPoint:
      {
        if (!NearestPointInterpolation(x_, y_, theValue))
        {
          theValue = kFloatMissing;
          return false;
        }
        return true;
      }

      default:
        theValue = kFloatMissing;
        return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::NearestPointInterpolation(double x_, double y_, double &theValue)
{
  try
  {
    if (NearestGridPoint(x_, y_))
    {
      theValue = FloatValue(NFmiPoint(x_, y_));
      if (!IsMissingValue(theValue))  // 19.4.99/EL
        return true;
    }
    theValue = kFloatMissing;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::BiLinearInterpolation(double x, double y, double &theValue)
{
  try
  {
    double X, Y;

    double dx = modf(x, &X);
    double dy = modf(y, &Y);

    if ((dx != 0.) && (dx < kFmiEps))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < kFmiEps))
    {
      dy = 0;
    }

    if (dx > (1 - kFmiEps))
    {
      dx = 0;
      X++;
    }
    if (dy > (1 - kFmiEps))
    {
      dy = 0;
      Y++;
    }

    double topLeftValue;
    double topRightValue;
    double bottomLeftValue;
    double bottomRightValue;

    if ((X == itsPreviousX) && (Y == itsPreviousY))
    {
      // Still sitting within the same grid cell -->
      // previous cell corner values can be used

      topLeftValue = itsTopLeftValue;
      topRightValue = itsTopRightValue;
      bottomLeftValue = itsBottomLeftValue;
      bottomRightValue = itsBottomRightValue;
    }
    else
    {
      if (!GridPoint(X, Y))
      {
        theValue = static_cast<double>(kFloatMissing);

        itsPreviousX = kMinDouble;
        itsPreviousY = kMinDouble;

        return false;
      }

      topLeftValue = FloatValue(0, 1);
      topRightValue = FloatValue(1, 1);
      bottomLeftValue = FloatValue(0, 0);
      bottomRightValue = FloatValue(1, 0);

      itsTopLeftValue = topLeftValue;
      itsTopRightValue = topRightValue;
      itsBottomLeftValue = bottomLeftValue;
      itsBottomRightValue = bottomRightValue;

      itsPreviousX = X;
      itsPreviousY = Y;
    }

    if ((topLeftValue == topRightValue) && (topRightValue == bottomLeftValue) &&
        (bottomLeftValue == bottomRightValue))
    {
      theValue = topLeftValue;
      return true;  // Each of four corner values are the same - nothing to interpolate!
    }

    if ((!IsMissingValue(topLeftValue)) && (!IsMissingValue(topRightValue)) &&
        (!IsMissingValue(bottomLeftValue)) && (!IsMissingValue(bottomRightValue)))
    {
      double interpolatedTopValue = dx * topRightValue + (1. - dx) * topLeftValue;
      double interpolatedBottomValue = dx * bottomRightValue + (1. - dx) * bottomLeftValue;

      theValue = dy * interpolatedTopValue + (1. - dy) * interpolatedBottomValue;
      return true;
    }

    // Missing value(s) found. Use nearest point interpolation method instead
    NearestPointInterpolation(x, y, theValue);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::FastBiVariateInterpolation(double x, double y, double &theValue)
{
  try
  {
    double X, Y;

    double dx = modf(x, &X);
    double dy = modf(y, &Y);

    auto x0 = static_cast<int>(X);
    auto y0 = static_cast<int>(Y);

    if ((dx != 0.) && (dx < kFmiEps))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < kFmiEps))
    {
      dy = 0;
    }

    if (dx > (1 - kFmiEps))
    {
      dx = 0;
      X++;
    }
    if (dy > (1 - kFmiEps))
    {
      dy = 0;
      Y++;
    }

    double topLeftValue;
    double topRightValue;
    double bottomLeftValue;
    double bottomRightValue;

    // Copy the datapool data into a temporary array of floats just to avoid "item-by-item" type
    // casting when interpolating

    long numberOfFloats = XNumber() * YNumber();

    if (itsFloatData == nullptr)
    {
      // In fact, this initialization is a "once in a life time" operation and should be in
      // constructor or Init(), instead.
      // However, this might be waste of memory space since "fast" interpolation may not necessarily
      // be used at all.
      itsFloatData = new float[numberOfFloats];

      memcpy(reinterpret_cast<unsigned char *>(itsFloatData),
             reinterpret_cast<const unsigned char *>(itsData->Data()),
             numberOfFloats * sizeof(float));
    }

    if ((X == itsPreviousX) && (Y == itsPreviousY))
    {
      // Still sitting within the same grid cell -->
      // previous cell corner values can be used

      topLeftValue = itsTopLeftValue;
      topRightValue = itsTopRightValue;
      bottomLeftValue = itsBottomLeftValue;
      bottomRightValue = itsBottomRightValue;
    }
    else
    {
      int x1 = x0;
      int y1 = y0;

      if (x0 + 1 < static_cast<int>(XNumber()))
        x1++;

      if (y0 + 1 < static_cast<int>(YNumber()))
        y1++;

      topLeftValue = DataValue(x0, y1);
      topRightValue = DataValue(x1, y1);
      bottomLeftValue = DataValue(x0, y0);
      bottomRightValue = DataValue(x1, y0);

      itsTopLeftValue = topLeftValue;
      itsTopRightValue = topRightValue;
      itsBottomLeftValue = bottomLeftValue;
      itsBottomRightValue = bottomRightValue;

      itsPreviousX = X;
      itsPreviousY = Y;
    }

    if ((topLeftValue == topRightValue) && (topRightValue == bottomLeftValue) &&
        (bottomLeftValue == bottomRightValue))
    {
      theValue = topLeftValue;
      return true;  // Each of four corner values are the same - nothing to interpolate!
    }

    if ((!IsMissingValue(topLeftValue)) && (!IsMissingValue(topRightValue)) &&
        (!IsMissingValue(bottomLeftValue)) && (!IsMissingValue(bottomRightValue)))
    {
      // Bi-linear interpolation
      double interpolatedTopValue = dx * topRightValue + (1. - dx) * topLeftValue;
      double interpolatedBottomValue = dx * bottomRightValue + (1. - dx) * bottomLeftValue;
      theValue = dy * interpolatedTopValue + (1. - dy) * interpolatedBottomValue;

      return true;
    }

    // At least one missing value was encountered.
    // Use simple nearest neighbor method (returning either a missing value or a usual "non-missing"
    // value)

    int xi = x0;
    int yi = y0;

    if (dx >= 0.5)
    {
      if (xi + 1 < static_cast<int>(XNumber()))
        xi++;
    }

    if (dy >= 0.5)
    {
      if (yi + 1 < static_cast<int>(YNumber()))
        yi++;
    }

    theValue = DataValue(xi, yi);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGridBase::DataValue(int x, int y)
{
  try
  {
    return static_cast<double>(itsFloatData[y * XNumber() + x]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Kill the dog
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Swap()
{
  try
  {
    if (itsStartingCorner == kBottomLeft)
      return true;

    NFmiDataPool datapool(*itsData);

    if (datapool.Data() == nullptr)
      return false;

    if (itsData)
      delete itsData;

    itsData = new NFmiDataPool();

    if (!itsData->Init(Size()))
      return false;

    ResetCrop();
    First();

    switch (itsStartingCorner)
    {
      case kBottomLeft:
        return true;
      case kBottomRight:
      {
        GridPoint(itsLastX, itsFirstY);
        do
        {
          GridPoint(itsLastX, itsCurrentY);
          do
          {
            FloatValue(static_cast<float>(datapool.FloatValue()));
            datapool.Next();
          } while (Left());
        } while (Up());
        break;
      }
      case kTopLeft:
      {
        GridPoint(itsFirstX, itsLastY);
        do
        {
          GridPoint(itsFirstX, itsCurrentY);
          do
          {
            FloatValue(static_cast<float>(datapool.FloatValue()));
            datapool.Next();
          } while (Right());
        } while (Down());
        break;
      }
      case kTopRight:
      {
        itsData->First();
        GridPoint(itsLastX, itsLastY);
        do
        {
          GridPoint(itsLastX, itsCurrentY);
          do
          {
            FloatValue(static_cast<float>(datapool.FloatValue()));
            datapool.Next();
          } while (Left());
        } while (Down());
        break;
      }
      default:
      {
        Origo(kBottomLeft);
        return false;
      }
    }

    Origo(kBottomLeft);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numOfSteps Undocumented
 */
// ----------------------------------------------------------------------

void NFmiGridBase::MeanFilter(unsigned long numOfSteps)
{
  try
  {
    FirstY();
    do
    {
      FirstX();
      do
      {
        double sum = 0;
        double num = 0;
        unsigned long dx;
        unsigned long dy;
        for (dy = 0; dy < numOfSteps; dy++)
        {
          for (dx = 0; dx < numOfSteps; dx++)
          {
            double tmp = FloatValue(dx, dy);
            if (tmp != kFloatMissing)
            {
              sum += tmp;
              num++;
            }
          }
        }
        if (!num)
          continue;

        sum = sum / num;
        for (dy = 0; dy < numOfSteps; dy++)
        {
          for (dx = 0; dx < numOfSteps; dx++)
          {
            itsData->FloatValue(DataIndex(itsCurrentX + dx, itsCurrentY + dy), float(sum));
          }
        }
      } while (Right(numOfSteps));
    } while (Up(numOfSteps));
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

void NFmiGridBase::Center()
{
  try
  {
    GridPoint(FirstGridPoint().X() + (LastGridPoint().X() - FirstGridPoint().X()) / 2.,
              FirstGridPoint().Y() + (LastGridPoint().Y() - FirstGridPoint().Y()) / 2.);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::ReadBinaryData(const char *theFileName)
{
  try
  {
    // In binary mode, reads 'numberOfItems' data items of type float from the input stream
    // 'theFileName'
    // NOTE: Reads the data items for the whole grid, not just for the cropped part of it.

    ResetCrop();
    unsigned long numberOfItems = XNumber() * YNumber();

    if (!itsData)
      itsData = new NFmiDataPool();

    return itsData->ReadBinaryData(numberOfItems, theFileName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::ReadTextData(const char *theFileName)
{
  try
  {
    // In text mode, reads 'numberOfItems' data items of type 'float' from the input stream
    // 'theFileName'
    // NOTE: Reads the data items for the whole grid, not just for the cropped part of it.

    ResetCrop();
    unsigned long numberOfItems = XNumber() * YNumber();

    if (!itsData)
      itsData = new NFmiDataPool();

    return itsData->ReadTextData(numberOfItems, theFileName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::WriteBinaryData(const char *theFileName)
{
  try
  {
    // In binary mode, writes 'numberOfItems' data items into the output stream 'theFileName'
    // NOTE: Writes out the data items for the whole grid, not just for the cropped part of it.

    ResetCrop();
    unsigned long numberOfItems = XNumber() * YNumber();

    if (itsData)
      return itsData->WriteBinaryData(numberOfItems, theFileName);
    else
      return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::WriteTextData(const char *theFileName)
{
  try
  {
    // In text mode, writes 'numberOfItems' data items into the input stream 'theFileName'.
    // NOTE: Writes out the data items for the whole grid, not just for the cropped part of it.

    ResetCrop();
    unsigned long numberOfItems = XNumber() * YNumber();

    if (itsData)
      return itsData->WriteBinaryData(numberOfItems, theFileName);
    else
      return false;
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
 * \return The output stream to written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGridBase::Write(std::ostream &file) const
{
  try
  {
    file << static_cast<unsigned long>(itsInterpolationMethod) << std::endl;
    file << static_cast<unsigned long>(itsStartingCorner) << std::endl;
    file << itsXNumber << " ";
    file << itsYNumber << std::endl;

    if (itsData)
      file << *itsData;
    else
    {
      NFmiDataPool pool;  // Salla teki pidemmän kaavan kautta välttääkseen varoituksen..
      file << pool;
    }

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

std::istream &NFmiGridBase::Read(std::istream &file)
{
  try
  {
    unsigned long interpolationMethod;
    unsigned long startingCorner;

    file >> interpolationMethod;
    itsInterpolationMethod = FmiInterpolationMethod(interpolationMethod);

    file >> startingCorner;
    itsStartingCorner = FmiDirection(startingCorner);
    file >> itsXNumber;
    file >> itsYNumber;

    itsFirstX = 0;
    itsLastX = itsXNumber - 1;
    itsFirstY = 0;
    itsLastY = itsYNumber - 1;

    if (itsData)
      delete itsData;

    itsData = new NFmiDataPool();
    file >> *itsData;

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

// asettaa paikan annetun indeksin mukaisesti hilassa

bool NFmiGridBase::Index(unsigned long theIndex)
{
  try
  {
    if (theIndex >= Size())
      return false;

    itsCurrentX = theIndex % itsXNumber;
    itsCurrentY = theIndex / itsXNumber;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXNumber Undocumented
 * \param theYNumber Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Init(unsigned long theXNumber, unsigned long theYNumber)
{
  try
  {
    itsFloatData = nullptr;
    itsStartingCorner = kBottomLeft;
    itsInterpolationMethod = kLinearly;
    itsXNumber = theXNumber;
    itsYNumber = theYNumber;
    itsCurrentX = 0;
    itsCurrentY = 0;
    //  itsBase = false;
    fWaitX = true;
    fWaitY = true;
    itsFirstX = 0;
    itsFirstY = 0;
    itsLastX = theXNumber - 1;
    itsLastY = theYNumber - 1;
    itsPreviousX = kMinDouble;
    itsPreviousY = kMinDouble;
    itsTopLeftValue = kMaxDouble;
    itsTopRightValue = kMaxDouble;
    itsBottomLeftValue = kMaxDouble;
    itsBottomRightValue = kMaxDouble;

    if (itsData)
      delete itsData;

    itsData = new NFmiDataPool();
    return itsData->Init(theXNumber * theYNumber) == true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Tein tämän Swap-funktion , alkuperäinen on outo siinä mielessä, että
 * sillä on joku startincorner olemassa. Jos se ei ole bottomleft, tehdään
 * swappiä, mutta startingcorner jää kuitenkin sellaiseksi kuin se oli.
 * Nyt haluan kertoa gridille, että koska sinä olet väärin päin,
 * muuta datasi seuraavasti, mutta lopuksi startingcorner pitää olla
 * bottomleft, mihin muotoon lopuksi data jää.
 *
 * \param theCurrentDirection Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGridBase::Swap(FmiDirection theCurrentDirection)
{
  try
  {
    FmiDirection oldDirection =
        itsStartingCorner;  // itsStartingCorner pitäisi pitää sisällään bottomleft:in
    itsStartingCorner = theCurrentDirection;
    bool status = Swap() == true;
    itsStartingCorner =
        oldDirection;  // asetetaan bottomleft taas kulmaksi, koska data on muutettu sen mukaiseksi.
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * operator=
 */
// ----------------------------------------------------------------------

NFmiGridBase &NFmiGridBase::operator=(const NFmiGridBase &theBase)
{
  try
  {
    theBase.itsData ? itsData = new NFmiDataPool(*(theBase.itsData)) : itsData = nullptr;
    itsFloatData = nullptr;
    itsStartingCorner = theBase.itsStartingCorner;
    itsInterpolationMethod = theBase.itsInterpolationMethod;
    itsXNumber = theBase.itsXNumber;
    itsYNumber = theBase.itsYNumber;
    itsCurrentX = theBase.itsCurrentX;
    itsCurrentY = theBase.itsCurrentY;
    //  itsBase = theBase.itsBase;
    fWaitX = theBase.fWaitX;
    fWaitY = theBase.fWaitY;
    itsFirstX = theBase.itsFirstX;
    itsFirstY = theBase.itsFirstY;
    itsLastX = theBase.itsLastX;
    itsLastY = theBase.itsLastY;
    itsPreviousX = theBase.itsPreviousX;
    itsPreviousY = theBase.itsPreviousY;
    itsTopLeftValue = theBase.itsTopLeftValue;
    itsTopRightValue = theBase.itsTopRightValue;
    itsBottomLeftValue = theBase.itsBottomLeftValue;
    itsBottomRightValue = theBase.itsBottomRightValue;

    // itsFloatData array of floats will NOT be copied

    if (itsData)
      itsData->InitMissingValues(*theBase.DataPool());

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// gridbasen yhtäläisyydestä ei oikeastaan muuta dataosiot kiinnosta kuin hilan x- ja y-koot.
bool NFmiGridBase::operator==(const NFmiGridBase &theGrid) const
{
  try
  {
    if (this == &theGrid)
      return true;

    return (itsXNumber == theGrid.itsXNumber && itsYNumber == theGrid.itsYNumber);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value for the grid definition
 */
// ----------------------------------------------------------------------

std::size_t NFmiGridBase::HashValue() const
{
  try
  {
    std::size_t hash = 0;

    Fmi::hash_combine(hash, Fmi::hash_value(itsStartingCorner));
    Fmi::hash_combine(hash, Fmi::hash_value(itsInterpolationMethod));
    Fmi::hash_combine(hash, Fmi::hash_value(itsXNumber));
    Fmi::hash_combine(hash, Fmi::hash_value(itsYNumber));
    Fmi::hash_combine(hash, Fmi::hash_value(itsFirstX));
    Fmi::hash_combine(hash, Fmi::hash_value(itsFirstY));
    Fmi::hash_combine(hash, Fmi::hash_value(itsLastX));
    Fmi::hash_combine(hash, Fmi::hash_value(itsLastY));

    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
