// ======================================================================
/*!
 * \file NFmiRotatedLatLonArea.h
 * \brief Interface of class NFmiRotatedLatLonArea
 */
// ======================================================================

#pragma once

#include "NFmiLatLonArea.h"

//! Undocumented

class NFmiRotatedLatLonArea : public NFmiLatLonArea
{
 public:
  virtual ~NFmiRotatedLatLonArea();
  NFmiRotatedLatLonArea();
  NFmiRotatedLatLonArea(const NFmiRotatedLatLonArea &theRotatedLatLonArea);
  NFmiRotatedLatLonArea(const NFmiPoint &theBottomLeftLatLon,
                        const NFmiPoint &theTopRightLatLon,
                        const NFmiPoint &theSouthernPole =
                            NFmiPoint(0., -30.),  // 25.4.02/EL: lon = 0, lat = 30 South
                        const NFmiPoint &theTopLeftXY = NFmiPoint(0., 0.),
                        const NFmiPoint &theBottomRightXY = NFmiPoint(1., 1.),
                        const bool initiallyRotated = false,
                        bool usePacificView = false);

  virtual NFmiArea *Clone() const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const;
  virtual const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const;

  virtual bool IsInside(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint ToRotLatLon(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint ToRegLatLon(const NFmiPoint &theRotLatLonPoint) const;
  const NFmiPoint &SouthernPole() const { return itsSouthernPole; }
  virtual NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const;

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;
  const std::string AreaStr() const;
  virtual const std::string WKT() const;

  using NFmiLatLonArea::operator==;
  virtual bool operator==(const NFmiRotatedLatLonArea &theArea) const;
  virtual bool operator==(const NFmiArea &theArea) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const NFmiAngle TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                           double theLatitudeEpsilon = 0.001) const;

  std::size_t HashValue() const;

  virtual void Init(bool fKeepWorldRect = false);

 private:
  NFmiPoint itsSouthernPole;

};  // class NFmiRotatedLatLonArea

//! Undocumented, should be removed
typedef NFmiRotatedLatLonArea *PNFmiRotatedLatLonArea;

//! ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiRotatedLatLonArea::~NFmiRotatedLatLonArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRotatedLatLonArea::ClassId() const
{
  return kNFmiRotatedLatLonArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiRotatedLatLonArea::ClassName() const
{
  return "NFmiRotatedLatLonArea";
}

// ======================================================================
