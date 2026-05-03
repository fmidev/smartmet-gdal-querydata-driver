// ======================================================================
/*!
 * \file NFmiYKJArea.h
 * \brief Interface of class NFmiYKJArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiKKJArea.h"

//! Undocumented
class NFmiYKJArea : public NFmiKKJArea
{
 public:
  virtual ~NFmiYKJArea();
  NFmiYKJArea();
  NFmiYKJArea(const NFmiYKJArea& theYKJArea);

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
              bool usePacificView = false);

  NFmiYKJArea(double theRadialRangeInMeters,
              const NFmiPoint& theCenterLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              const double theWidthInMeters,
              const double theHeightInMeters,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  NFmiYKJArea(const NFmiPoint& theBottomLeftWorldXY,  // defaults to meters
              const NFmiPoint& theTopRightWorldXY,    // defaults to meters
              const bool& IsWorldXYInMeters,          // dummy argument
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  void Init(bool fKeepWorldRect = false);
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;

  virtual NFmiArea* Clone() const;

  virtual unsigned long ClassId() const;
  virtual const char* ClassName() const;
  const std::string AreaStr() const;
  virtual const std::string WKT() const;

  NFmiYKJArea& operator=(const NFmiYKJArea& theArea);

  virtual bool operator==(const NFmiYKJArea& theArea) const;
  virtual bool operator!=(const NFmiYKJArea& theArea) const;

  using NFmiKKJArea::operator==;
  using NFmiKKJArea::operator!=;
  virtual bool operator==(const NFmiArea& theArea) const;
  virtual bool operator!=(const NFmiArea& theArea) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  std::size_t HashValue() const;

 protected:
  virtual int ZoneNumberByLongitude(const double lon) const;
  virtual int ZoneNumberByEasting(const double easting) const;

 private:
  const NFmiPoint CornerWorldXY(const NFmiPoint& latlonPoint) const;

};  // class NFmiYKJArea

//! Undocumented, should be removed
typedef NFmiYKJArea* PNFmiYKJArea;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiYKJArea::~NFmiYKJArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiYKJArea::ClassId() const
{
  return kNFmiYKJArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiYKJArea::ClassName() const
{
  return "NFmiYKJArea";
}
