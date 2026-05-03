// ======================================================================
/*!
 * \file NFmiAzimuthalArea.h
 * \brief Interface of class NFmiAzimuthalArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class NFmiAzimuthalArea : public NFmiArea
{
 public:
  virtual ~NFmiAzimuthalArea();

  NFmiAzimuthalArea();

  NFmiAzimuthalArea(const NFmiAzimuthalArea& theAzimuthalArea);

  NFmiAzimuthalArea(double theRadialRangeInMeters,
                    const NFmiPoint& theCenterLatLon,
                    const NFmiPoint& theTopLeftXY,
                    const NFmiPoint& theBottomRightXY,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const double theRadialRange,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  virtual void Init(bool fKeepWorldRect = false);

  virtual const NFmiRect WorldRect() const;

  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const;

  virtual const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint ToLatLon(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint WorldXYToXY(const NFmiPoint& theWorldXYPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint ToXY(double theAzimuth, double theRadius) const;

  double CentralLongitude() const;
  double Orientation() const;

  double CentralLatitude() const;
  double TrueLatitude() const;
  double XScaleFactor() const;
  double YScaleFactor() const;

  virtual const NFmiPoint RadialXYPoint(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint CurrentCenter() const;

  NFmiAzimuthalArea& operator=(const NFmiAzimuthalArea& theArea);

  virtual bool operator==(const NFmiAzimuthalArea& theArea) const;
  virtual bool operator!=(const NFmiAzimuthalArea& theArea) const;

  virtual bool operator==(const NFmiArea& theArea) const;
  virtual bool operator!=(const NFmiArea& theArea) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual const char* ClassName() const;
  const std::string AreaStr() const = 0;

  std::size_t HashValue() const;

 protected:
  NFmiPoint itsTopRightLatLon;
  NFmiPoint itsBottomLeftLatLon;

  NFmiPoint itsBottomLeftWorldXY;

  virtual double K(const double delta) const = 0;
  virtual double CalcDelta(const double xyDistance) const = 0;
  virtual double DistanceFromPerspectivePointToCenterOfEarth() const = 0;

  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;
  double itsRadialRange;
  NFmiLongitude itsCentralLongitude;
  NFmiLatitude itsCentralLatitude;
  NFmiLatitude itsTrueLatitude;
  double itsTrueLatScaleFactor;

 private:
};  // class NFmiAzimuthalArea

//! Undocumented, should be removed
typedef NFmiAzimuthalArea* PNFmiAzimuthalArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiAzimuthalArea::~NFmiAzimuthalArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiAzimuthalArea::WorldRect() const
{
  return itsWorldRect;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLongitude() const
{
  return itsCentralLongitude.Value();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::Orientation() const
{
  return itsCentralLongitude.Value();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLatitude() const
{
  return itsCentralLatitude.Value();
}
// ----------------------------------------------------------------------
/*!
 * \return Projection true latitude
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::TrueLatitude() const
{
  return itsTrueLatitude.Value();
}
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::XScaleFactor() const
{
  return itsXScaleFactor;
}
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::YScaleFactor() const
{
  return itsYScaleFactor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiAzimuthalArea::ClassName() const
{
  return "NFmiAzimuthalArea";
}

// ======================================================================
