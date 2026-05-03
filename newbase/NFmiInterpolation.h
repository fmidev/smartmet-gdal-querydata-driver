// ======================================================================
/*!
 * \brief Interface of namespace NFmiInterpolation
 */
// ======================================================================

#pragma once

class NFmiPoint;

namespace NFmiInterpolation
{
// 1D linear interpolation

double Linear(double theX, double theX1, double theX2, double theY1, double theY2);

double Linear(double theFactor, double theLeft, double theRight);

double WindVector(double theFactor, double theLeftWV, double theRightWV);
// 1D linear interpolation with modulo

double ModLinear(double theX,
                 double theX1,
                 double theX2,
                 double theY1,
                 double theY2,
                 unsigned int theModulo = 360);

double ModLinear(double theFactor, double theLeft, double theRight, unsigned int theModulo = 360);

// 2D linear interpolation inside a rectangle

double BiLinear(double theX,
                double theY,
                double theTopLeft,
                double theTopRight,
                double theBottomLeft,
                double theBottomRight);

double WindVector(double theX,
                  double theY,
                  double theTopLeft,
                  double theTopRight,
                  double theBottomLeft,
                  double theBottomRight);

double NearestNonMissing(double theX,
                         double theY,
                         double theTopLeft,
                         double theTopRight,
                         double theBottomLeft,
                         double theBottomRight);

double NearestPoint(double theX,
                    double theY,
                    double theTopLeft,
                    double theTopRight,
                    double theBottomLeft,
                    double theBottomRight);

// 2D linear interpolation of a coordinate inside a rectangle

NFmiPoint BiLinear(double theX,
                   double theY,
                   const NFmiPoint& theTopLeft,
                   const NFmiPoint& theTopRight,
                   const NFmiPoint& theBottomLeft,
                   const NFmiPoint& theBottomRight);

// 2D linear interpolation with modulo inside a rectangle

double ModBiLinear(double theX,
                   double theY,
                   double theTopLeft,
                   double theTopRight,
                   double theBottomLeft,
                   double theBottomRight,
                   unsigned int theModulo = 360);

// 2D wind interpolation

class WindInterpolator
{
 public:
  WindInterpolator();
  void operator()(double theSpeed, double theDirection, double theWeight = 1.0);
  void Reset();
  double Speed() const;
  double Direction() const;

 private:
  double itsCount;
  double itsSpeedSum;
  double itsSpeedSumX;
  double itsSpeedSumY;
  double itsWeightSum;

  double itsBestDirection;
  double itsBestWeight;
};

}  // namespace NFmiInterpolation

// ======================================================================
