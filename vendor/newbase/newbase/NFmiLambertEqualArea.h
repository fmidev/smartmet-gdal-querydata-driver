// ======================================================================
/*!
 * \file NFmiLambertEqualArea.h
 * \brief Interface of class NFmiLambertEqualArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumened
class NFmiLambertEqualArea : public NFmiAzimuthalArea
{
 public:
  virtual ~NFmiLambertEqualArea();

  NFmiLambertEqualArea();

  NFmiLambertEqualArea(const NFmiLambertEqualArea& theLambertEqualArea);

  NFmiLambertEqualArea(const NFmiPoint& theBottomLeftLatLon,
                       const NFmiPoint& theTopRightLatLon,
                       const double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       const double theCentralLatitude = 52.,
                       const double theTrueLatitude = 90.,
                       bool usePacificView = false);

  NFmiLambertEqualArea(double theRadialRangeInMeters,
                       const NFmiPoint& theCenterLatLon,
                       const NFmiPoint& theTopLeftXY,
                       const NFmiPoint& theBottomRightXY);

  NFmiLambertEqualArea(const NFmiPoint& theBottomLeftLatLon,
                       const double theWidthInMeters,
                       const double theHeightInMeters,
                       const double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       const double theCentralLatitude = 52.,
                       const double theTrueLatitude = 90.);

  NFmiLambertEqualArea(const double theRadialRange,
                       const double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       const double theCentralLatitude = 52.,
                       const double theTrueLatitude = 90.);

  virtual void Init(bool fKeepWorldRect = false);
  virtual NFmiArea* Clone() const;
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;
  virtual const NFmiRect WorldRect() const;

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const;

  NFmiLambertEqualArea& operator=(const NFmiLambertEqualArea& theArea);

  bool operator==(const NFmiLambertEqualArea& theArea) const;
  bool operator!=(const NFmiLambertEqualArea& theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;
  bool operator==(const NFmiArea& theArea) const;
  bool operator!=(const NFmiArea& theArea) const;

  virtual unsigned long ClassId() const;
  virtual const char* ClassName() const;
  const std::string AreaStr() const;
  virtual const std::string WKT() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  std::size_t HashValue() const;

 protected:
  virtual double K(const double delta) const;
  virtual double CalcDelta(const double xyDistance) const;
  virtual double DistanceFromPerspectivePointToCenterOfEarth() const;

 private:
};  // class NFmiLambertEqualArea

//! Undocumented, should be removed
typedef NFmiLambertEqualArea* PNFmiLambertEqualArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiLambertEqualArea::~NFmiLambertEqualArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiLambertEqualArea::WorldRect() const
{
  return itsWorldRect;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiLambertEqualArea::ClassId() const
{
  return kNFmiLambertEqualArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiLambertEqualArea::ClassName() const
{
  return "kNFmiLambertEqualArea";
}

// ======================================================================
