// ======================================================================
/*!
 * \file NFmiStereographicArea.h
 * \brief Interface of class NFmiStereographicArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumened
class NFmiStereographicArea : public NFmiAzimuthalArea
{
 public:
  virtual ~NFmiStereographicArea();

  NFmiStereographicArea();

  NFmiStereographicArea(const NFmiStereographicArea& theStereographicArea);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        const NFmiPoint& theTopRightLatLon,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.,
                        bool usePacificView = false);

  NFmiStereographicArea(double theRadialRangeInMeters,
                        const NFmiPoint& theCenterLatLon,
                        const NFmiPoint& theTopLeftXY,
                        const NFmiPoint& theBottomRightXY);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        const double theWidthInMeters,
                        const double theHeightInMeters,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.);

  NFmiStereographicArea(const double theRadialRange,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.);

  virtual void Init(bool fKeepWorldRect = false);
  virtual NFmiArea* Clone() const;
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;
  virtual const NFmiRect WorldRect() const;

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const;

  NFmiStereographicArea& operator=(const NFmiStereographicArea& theArea);

  bool operator==(const NFmiStereographicArea& theArea) const;
  bool operator!=(const NFmiStereographicArea& theArea) const;

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
};  // class NFmiStereographicArea

//! Undocumented, should be removed
typedef NFmiStereographicArea* PNFmiStereographicArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiStereographicArea::~NFmiStereographicArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiStereographicArea::WorldRect() const
{
  return itsWorldRect;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiStereographicArea::ClassId() const
{
  return kNFmiStereographicArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiStereographicArea::ClassName() const
{
  return "kNFmiStereographicArea";
}

// ======================================================================
