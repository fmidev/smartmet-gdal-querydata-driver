// ======================================================================
/*!
 * \brief Implementation of namespace NFmiInterpolation
 */
// ======================================================================
/*!
 * \namespace NFmiInterpolation
 *
 * \brief Interpolation tools
 *
 * Namespace NFmiInterpolation contains various utility functions to
 * perform interpolation. The main difficulty in handling interpolation
 * in newbase is how to handle missing values. The functions provided
 * by NFmiInterpolation handle missing values automatically.
 *
 */
// ======================================================================

#include "NFmiInterpolation.h"
#include "NFmiModMeanCalculator.h"
#include "NFmiPoint.h"
#include <macgyver/Exception.h>
#include <set>

namespace NFmiInterpolation
{
// ----------------------------------------------------------------------
/*!
 * \brief Linear interpolation in 1 dimension
 *
 * \param theX The X-coordinate
 * \param theX1 The X-coordinate of the left value
 * \param theX2 The X-coordinate of the right value
 * \param theY1 The value at X1
 * \param theY2 The value at Y2
 * \return The interpolated value (or kFloatMissing)
 */
// ----------------------------------------------------------------------

double Linear(double theX, double theX1, double theX2, double theY1, double theY2)
{
  try
  {
    // Handle special case where X1==X2
    if (theX1 == theX2)
    {
      if (theX != theX1)
        return kFloatMissing;
      if (theY1 == kFloatMissing || theY2 == kFloatMissing)
        return kFloatMissing;
      if (theY1 != theY2)
        return kFloatMissing;
      return theY1;
    }

    double factor = (theX - theX1) / (theX2 - theX1);
    return Linear(factor, theY1, theY2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Linear windvector interpolation in 1 dimension where value is constructed
 *  from wind dir (WD) and speed (WS). windVwctor = int(WS)*100 + int(WD/10).
 *
 * \param theFactor Relative coordinate offset from the left value
 * \param theLeft The left value
 * \param theRight The right value
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

double WindVector(double theFactor, double theLeftWV, double theRightWV)
{
  try
  {
    if (theLeftWV == kFloatMissing)
      return (theFactor == 1 ? theRightWV : kFloatMissing);

    if (theRightWV == kFloatMissing)
      return (theFactor == 0 ? theLeftWV : kFloatMissing);

    double leftWD = (static_cast<int>(theLeftWV) % 100) * 10.;
    auto leftWS = static_cast<double>(static_cast<int>(theLeftWV) / 100);
    double rightWD = (static_cast<int>(theRightWV) % 100) * 10.;
    auto rightWS = static_cast<double>(static_cast<int>(theRightWV) / 100);

    NFmiInterpolation::WindInterpolator windInterpolator;
    windInterpolator.operator()(leftWS, leftWD, 1 - theFactor);
    windInterpolator.operator()(rightWS, rightWD, theFactor);
    double wdInterp = windInterpolator.Direction();
    double wsInterp = windInterpolator.Speed();

    if (wdInterp != kFloatMissing && wsInterp != kFloatMissing)
      return round(wsInterp) * 100 + round(wdInterp / 10.);

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double WindVector(double theX,
                  double theY,
                  double theTopLeft,
                  double theTopRight,
                  double theBottomLeft,
                  double theBottomRight)
{
  try
  {
    double dx = theX;
    double dy = theY;

    if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
        theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      double blWD = (static_cast<int>(theBottomLeft) % 100) * 10.;
      auto blWS = static_cast<double>(static_cast<int>(theBottomLeft) / 100);
      double brWD = (static_cast<int>(theBottomRight) % 100) * 10.;
      auto brWS = static_cast<double>(static_cast<int>(theBottomRight) / 100);
      double tlWD = (static_cast<int>(theTopLeft) % 100) * 10.;
      auto tlWS = static_cast<double>(static_cast<int>(theTopLeft) / 100);
      double trWD = (static_cast<int>(theTopRight) % 100) * 10.;
      auto trWS = static_cast<double>(static_cast<int>(theTopRight) / 100);

      NFmiInterpolation::WindInterpolator windInterpolator;
      windInterpolator.operator()(blWS, blWD, (1 - dx) * (1 - dy));
      windInterpolator.operator()(brWS, brWD, dx *(1 - dy));
      windInterpolator.operator()(trWS, trWD, dx *dy);
      windInterpolator.operator()(tlWS, tlWD, (1 - dx) * dy);

      double wdInterp = windInterpolator.Direction();
      double wsInterp = windInterpolator.Speed();

      if (wdInterp != kFloatMissing && wsInterp != kFloatMissing)
        return round(wsInterp) * 100 + round(wdInterp / 10.);
      else
        return kFloatMissing;
    }

    // Grid cell edges

    if (dy == 0)
      return WindVector(dx, theBottomLeft, theBottomRight);
    if (dy == 1)
      return WindVector(dx, theTopLeft, theTopRight);
    if (dx == 0)
      return WindVector(dy, theBottomLeft, theTopLeft);
    if (dx == 1)
      return WindVector(dy, theBottomRight, theTopRight);

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Linear interpolation in 1 dimension
 *
 * \param theFactor Relative coordinate offset from the left value
 * \param theLeft The left value
 * \param theRight The right value
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

double Linear(double theFactor, double theLeft, double theRight)
{
  try
  {
    if (theLeft == kFloatMissing)
      return (theFactor == 1 ? theRight : kFloatMissing);

    if (theRight == kFloatMissing)
      return (theFactor == 0 ? theLeft : kFloatMissing);

    return (1 - theFactor) * theLeft + theFactor * theRight;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Linear interpolation in 1 dimension with modulo
 *
 * \param theX The X-coordinate
 * \param theX1 The X-coordinate of the left value
 * \param theX2 The X-coordinate of the right value
 * \param theY1 The value at X1
 * \param theY2 The value at Y2
 * \param theModulo The modulo
 * \return The interpolated value (or kFloatMissing)
 */
// ----------------------------------------------------------------------

double ModLinear(
    double theX, double theX1, double theX2, double theY1, double theY2, unsigned int theModulo)
{
  try
  {
    // Handle special case where X1==X2
    if (theX1 == theX2)
    {
      if (theX != theX1)
        return kFloatMissing;
      if (theY1 == kFloatMissing || theY2 == kFloatMissing)
        return kFloatMissing;
      if (theY1 != theY2)
        return kFloatMissing;
      return theY1;
    }

    double factor = (theX - theX1) / (theX2 - theX1);
    return ModLinear(factor, theY1, theY2, theModulo);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Linear interpolation in 1 dimension with modulo
 *
 * \param theFactor Relative coordinate offset from the left value
 * \param theLeft The left value
 * \param theRight The right value
 * \param theModulo The modulo
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

double ModLinear(double theFactor, double theLeft, double theRight, unsigned int theModulo)
{
  try
  {
    if (theLeft == kFloatMissing)
      return (theFactor == 1 ? theRight : kFloatMissing);

    if (theRight == kFloatMissing)
      return (theFactor == 0 ? theLeft : kFloatMissing);

    NFmiModMeanCalculator calculator(static_cast<float>(theModulo));
    calculator(static_cast<float>(theLeft), static_cast<float>(1 - theFactor));
    calculator(static_cast<float>(theRight), static_cast<float>(theFactor));
    return calculator();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Bilinear interpolation in 2 dimensions
 *
 * We assume all interpolation occurs in a rectilinear grid
 * and the given coordinates are relative within the grid cell.
 * The values must thus be in the range 0-1.
 *
 * \param theX The relative offset from the bottomleft X-coordinate
 * \param theY The relative offset from the bottomleft Y-coordinate
 * \param theTopLeft The top left value
 * \param theTopRight The top right value
 * \param theBottomLeft The bottom left value
 * \param theBottomRight The bottom right value
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

double BiLinear(double theX,
                double theY,
                double theTopLeft,
                double theTopRight,
                double theBottomLeft,
                double theBottomRight)
{
  try
  {
    double dx = theX;
    double dy = theY;

    if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
        theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      return ((1 - dx) * (1 - dy) * theBottomLeft + dx * (1 - dy) * theBottomRight +
              (1 - dx) * dy * theTopLeft + dx * dy * theTopRight);
    }

    // Grid cell edges

    if (dy == 0)
      return Linear(dx, theBottomLeft, theBottomRight);
    if (dy == 1)
      return Linear(dx, theTopLeft, theTopRight);
    if (dx == 0)
      return Linear(dy, theBottomLeft, theTopLeft);
    if (dx == 1)
      return Linear(dy, theBottomRight, theTopRight);

    // If only one corner is missing, interpolate within
    // the triangle formed by the three points, AND now
    // also outside it on the other triangle (little extrapolation).

    if (theTopLeft == kFloatMissing && theTopRight != kFloatMissing &&
        theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      double wsum = (dx * dy + (1 - dx) * (1 - dy) + dx * (1 - dy));
      return ((1 - dx) * (1 - dy) * theBottomLeft + dx * (1 - dy) * theBottomRight +
              dx * dy * theTopRight) /
             wsum;
    }
    else if (theTopLeft != kFloatMissing && theTopRight == kFloatMissing &&
             theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      double wsum = ((1 - dx) * dy + (1 - dx) * (1 - dy) + dx * (1 - dy));
      return ((1 - dx) * (1 - dy) * theBottomLeft + dx * (1 - dy) * theBottomRight +
              (1 - dx) * dy * theTopLeft) /
             wsum;
    }
    else if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
             theBottomLeft == kFloatMissing && theBottomRight != kFloatMissing)
    {
      double wsum = ((1 - dx) * dy + dx * dy + dx * (1 - dy));
      return (dx * (1 - dy) * theBottomRight + (1 - dx) * dy * theTopLeft + dx * dy * theTopRight) /
             wsum;
      ;
    }
    else if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
             theBottomLeft != kFloatMissing && theBottomRight == kFloatMissing)
    {
      double wsum = ((1 - dx) * (1 - dy) + (1 - dx) * dy + dx * dy);
      return ((1 - dx) * (1 - dy) * theBottomLeft + (1 - dx) * dy * theTopLeft +
              dx * dy * theTopRight) /
             wsum;
    }

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

class PointData
{
 public:
  PointData() : value_(kFloatMissing), distance_(kFloatMissing) {}
  PointData(FmiDirection corner, double value, double distance)
      : corner_(corner), value_(value), distance_(distance)
  {
  }

  FmiDirection corner_{kNoDirection};  // Minkä kulman pisteestä on kyse (lähinnä debuggaus infoa)
  double value_;                       // Määrätyn kulman arvo
  double distance_;                    // Etäisyys referenssi pisteeseen
};

bool operator<(const PointData &p1, const PointData &p2)
{
  return p1.distance_ < p2.distance_;
}
PointData CalcPointData(const NFmiPoint &referencePoint,
                        const NFmiPoint &cornerPoint,
                        FmiDirection corner,
                        double value)
{
  try
  {
    double distX = referencePoint.X() - cornerPoint.X();
    double distY = referencePoint.Y() - cornerPoint.Y();
    double distance = ::sqrt(distX * distX + distY * distY);
    return PointData(corner, value, distance);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Seeking the nearest non-missing value.
 *
 * We assume all interpolation occurs in a rectilinear grid
 * and the given coordinates are relative within the grid cell.
 * The values must thus be in the range 0-1.
 *
 * \param theX The relative offset from the bottomleft X-coordinate
 * \param theY The relative offset from the bottomleft Y-coordinate
 * \param theTopLeft The top left value
 * \param theTopRight The top right value
 * \param theBottomLeft The bottom left value
 * \param theBottomRight The bottom right value
 * \return The nearest non-missing value
 */
// ----------------------------------------------------------------------
double NearestNonMissing(double theX,
                         double theY,
                         double theTopLeft,
                         double theTopRight,
                         double theBottomLeft,
                         double theBottomRight)
{
  try
  {
    NFmiPoint referencePoint(theX, theY);
    std::multiset<PointData> sortedPointValues;
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(0, 0), kBottomLeft, theBottomLeft));
    sortedPointValues.insert(CalcPointData(referencePoint, NFmiPoint(0, 1), kTopLeft, theTopLeft));
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(1, 1), kTopRight, theTopRight));
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(1, 0), kBottomRight, theBottomRight));
    for (const auto &pointData : sortedPointValues)
    {
      if (pointData.value_ != kFloatMissing)
        return pointData.value_;
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Seeking the nearest value
 *
 * We assume all interpolation occurs in a rectilinear grid
 * and the given coordinates are relative within the grid cell.
 * The values must thus be in the range 0-1.
 *
 * \param theX The relative offset from the bottomleft X-coordinate
 * \param theY The relative offset from the bottomleft Y-coordinate
 * \param theTopLeft The top left value
 * \param theTopRight The top right value
 * \param theBottomLeft The bottom left value
 * \param theBottomRight The bottom right value
 * \return The nearest value
 */
// ----------------------------------------------------------------------
double NearestPoint(double theX,
                    double theY,
                    double theTopLeft,
                    double theTopRight,
                    double theBottomLeft,
                    double theBottomRight)
{
  try
  {
    NFmiPoint referencePoint(theX, theY);
    std::multiset<PointData> sortedPointValues;
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(0, 0), kBottomLeft, theBottomLeft));
    sortedPointValues.insert(CalcPointData(referencePoint, NFmiPoint(0, 1), kTopLeft, theTopLeft));
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(1, 1), kTopRight, theTopRight));
    sortedPointValues.insert(
        CalcPointData(referencePoint, NFmiPoint(1, 0), kBottomRight, theBottomRight));
    return sortedPointValues.begin()->value_;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Bilinear interpolation of coordinates in 2 dimensions
 *
 * We assume all interpolation occurs in a rectilinear grid
 * and the given coordinates are relative within the grid cell.
 * The values must thus be in the range 0-1.
 *
 * \param theX The relative offset from the bottomleft X-coordinate
 * \param theY The relative offset from the bottomleft Y-coordinate
 * \param theTopLeft The top left value
 * \param theTopRight The top right value
 * \param theBottomLeft The bottom left value
 * \param theBottomRight The bottom right value
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

NFmiPoint BiLinear(double theX,
                   double theY,
                   const NFmiPoint &theTopLeft,
                   const NFmiPoint &theTopRight,
                   const NFmiPoint &theBottomLeft,
                   const NFmiPoint &theBottomRight)
{
  try
  {
    const auto dx = theX;
    const auto dy = theY;
    const auto xd = 1 - dx;
    const auto yd = 1 - dy;
    const auto bottomleft = xd * yd;
    const auto bottomright = dx * yd;
    const auto topleft = xd * dy;
    const auto topright = dx * dy;

    const auto x = bottomleft * theBottomLeft.X() + bottomright * theBottomRight.X() +
                   topleft * theTopLeft.X() + topright * theTopRight.X();
    const auto y = bottomleft * theBottomLeft.Y() + bottomright * theBottomRight.Y() +
                   topleft * theTopLeft.Y() + topright * theTopRight.Y();

    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Bilinear interpolation in 2 dimensions with modulo
 *
 * We assume all interpolation occurs in a rectilinear grid
 * and the given coordinates are relative within the grid cell.
 * The values must thus be in the range 0-1.
 *
 * \param theX The relative offset from the bottomleft X-coordinate
 * \param theY The relative offset from the bottomleft Y-coordinate
 * \param theTopLeft The top left value
 * \param theTopRight The top right value
 * \param theBottomLeft The bottom left value
 * \param theBottomRight The bottom right value
 * \param theModulo The modulo
 * \return The interpolated value
 */
// ----------------------------------------------------------------------

double ModBiLinear(double theX,
                   double theY,
                   double theTopLeft,
                   double theTopRight,
                   double theBottomLeft,
                   double theBottomRight,
                   unsigned int theModulo)
{
  try
  {
    NFmiModMeanCalculator calculator(static_cast<float>(theModulo));

    double dx = theX;
    double dy = theY;

    if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
        theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      calculator(static_cast<float>(theBottomLeft), static_cast<float>((1 - dx) * (1 - dy)));
      calculator(static_cast<float>(theBottomRight), static_cast<float>(dx * (1 - dy)));
      calculator(static_cast<float>(theTopRight), static_cast<float>(dx * dy));
      calculator(static_cast<float>(theTopLeft), static_cast<float>((1 - dx) * dy));
      return calculator();
    }

    // Grid cell edges

    if (dy == 0)
      return ModLinear(dx, theBottomLeft, theBottomRight, theModulo);
    if (dy == 1)
      return ModLinear(dx, theTopLeft, theTopRight, theModulo);
    if (dx == 0)
      return ModLinear(dy, theBottomLeft, theTopLeft, theModulo);
    if (dx == 1)
      return ModLinear(dy, theBottomRight, theTopRight, theModulo);

    // If only one corner is missing, interpolate within
    // the triangle formed by the three points, but not
    // outside it.

    if (theTopLeft == kFloatMissing && theTopRight != kFloatMissing &&
        theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      if (dx < dy)
        return kFloatMissing;
      calculator(static_cast<float>(theBottomLeft), static_cast<float>((1 - dx) * (1 - dy)));
      calculator(static_cast<float>(theBottomRight), static_cast<float>(dx * (1 - dy)));
      calculator(static_cast<float>(theTopRight), static_cast<float>(dx * dy));
      return calculator();
    }
    else if (theTopLeft != kFloatMissing && theTopRight == kFloatMissing &&
             theBottomLeft != kFloatMissing && theBottomRight != kFloatMissing)
    {
      if (1 - dx < dy)
        return kFloatMissing;
      calculator(static_cast<float>(theTopLeft), static_cast<float>((1 - dx) * dy));
      calculator(static_cast<float>(theBottomLeft), static_cast<float>((1 - dx) * (1 - dy)));
      calculator(static_cast<float>(theBottomRight), static_cast<float>(dx * (1 - dy)));
      return calculator();
    }
    else if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
             theBottomLeft == kFloatMissing && theBottomRight != kFloatMissing)
    {
      if (1 - dx > dy)
        return kFloatMissing;
      calculator(static_cast<float>(theBottomRight), static_cast<float>(dx * (1 - dy)));
      calculator(static_cast<float>(theTopRight), static_cast<float>(dx * dy));
      calculator(static_cast<float>(theTopLeft), static_cast<float>((1 - dx) * dy));
      return calculator();
    }
    else if (theTopLeft != kFloatMissing && theTopRight != kFloatMissing &&
             theBottomLeft != kFloatMissing && theBottomRight == kFloatMissing)
    {
      if (dx > dy)
        return kFloatMissing;
      calculator(static_cast<float>(theBottomLeft), static_cast<float>((1 - dx) * (1 - dy)));
      calculator(static_cast<float>(theTopLeft), static_cast<float>((1 - dx) * dy));
      calculator(static_cast<float>(theTopRight), static_cast<float>(dx * dy));
      return calculator();
    }

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief 2D weighted wind interpolation
 */
// ----------------------------------------------------------------------

WindInterpolator::WindInterpolator()
    : itsCount(0),
      itsSpeedSum(0),
      itsSpeedSumX(0),
      itsSpeedSumY(0),
      itsWeightSum(0),
      itsBestDirection(0),
      itsBestWeight(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset the interpolator
 */
// ----------------------------------------------------------------------

void WindInterpolator::Reset()
{
  try
  {
    itsCount = 0;
    itsSpeedSum = 0;
    itsSpeedSumX = 0;
    itsSpeedSumY = 0;
    itsWeightSum = 0;
    itsBestDirection = 0;
    itsBestWeight = 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new wind vector to the weighted sum
 */
// ----------------------------------------------------------------------

void WindInterpolator::operator()(double theSpeed, double theDirection, double theWeight)
{
  try
  {
    if (theSpeed == kFloatMissing || theDirection == kFloatMissing || theWeight <= 0)
      return;

    if (itsCount == 0 || theWeight > itsBestWeight)
    {
      itsBestDirection = theDirection;
      itsBestWeight = theWeight;
    }

    ++itsCount;

    itsWeightSum += theWeight;

    itsSpeedSum += theWeight * theSpeed;

    // Note that wind direction is measured against the Y-axis,
    // not the X-axis. Hence sin and cos are not in the usual
    // order.

    double dir = FmiRad(theDirection);
    itsSpeedSumX += theWeight * cos(dir);
    itsSpeedSumY += theWeight * sin(dir);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mean wind speed
 *
 * We choose to return mean speed component instead of mean
 * 2D wind component since the wind is probably turning in the
 * area instead of diminishing between the grid corners like the
 * result would be if we used the 2D mean.
 */
// ----------------------------------------------------------------------

double WindInterpolator::Speed() const
{
  try
  {
    if (itsCount == 0 || itsWeightSum == 0)
      return kFloatMissing;

    return itsSpeedSum / itsWeightSum;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mean wind direction
 *
 * For direction we use 2D component analysis. If the result is a null
 * vector, we return the direction with the largest weight.
 */
// ----------------------------------------------------------------------

double WindInterpolator::Direction() const
{
  try
  {
    if (itsCount == 0 || itsWeightSum == 0)
      return kFloatMissing;

    double x = itsSpeedSumX / itsWeightSum;
    double y = itsSpeedSumY / itsWeightSum;

    // If there is almost exact cancellation, return best
    // weighted direction instead.

    if (sqrt(x * x + y * y) < 0.01)
      return itsBestDirection;

    // Otherwise use the 2D mean

    double dir = atan2(y, x);
    dir = FmiDeg(dir);
    if (dir < 0)
      dir += 360;
    return dir;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiInterpolation

// ======================================================================
