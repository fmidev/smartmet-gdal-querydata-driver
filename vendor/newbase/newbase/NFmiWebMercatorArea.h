// ======================================================================
/*!
 * \file NFmiWebMercatorArea.h
 * \brief Interface of class NFmiWebMercatorArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class NFmiWebMercatorArea : public NFmiArea
{
 public:
  virtual ~NFmiWebMercatorArea();
  NFmiWebMercatorArea();
  NFmiWebMercatorArea(const NFmiWebMercatorArea &theLatLonArea);
  NFmiWebMercatorArea(const NFmiPoint &theBottomLeftLatLon,
                      const NFmiPoint &theTopRightLatLon,
                      const NFmiPoint &theTopLeftXY = NFmiPoint(0., 0.),
                      const NFmiPoint &theBottomRightXY = NFmiPoint(1., 1.),
                      bool usePacificView = false);

  virtual NFmiArea *Clone() const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const;
  virtual const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const;
  virtual double XScale() const;
  virtual double YScale() const;
  virtual void Init(bool fKeepWorldRect = false);

  virtual const NFmiRect WorldRect() const;

  virtual const NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const;
  virtual const NFmiPoint WorldXYToXY(const NFmiPoint &theWorldXYPoint) const;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const;
  virtual NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const;
  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;
  const std::string AreaStr() const;
  virtual const std::string WKT() const;
  virtual bool operator==(const NFmiWebMercatorArea &theArea) const;
  virtual bool operator==(const NFmiArea &theArea) const;
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  std::size_t HashValue() const;

 protected:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;
};  // class NFmiWebMercatorArea

//! Undocumented, should be removed
typedef NFmiWebMercatorArea *PNFmiWebMercatorArea;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiWebMercatorArea::~NFmiWebMercatorArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiWebMercatorArea::WorldRect() const
{
  return itsWorldRect;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiWebMercatorArea::ClassId() const
{
  return kNFmiWebMercatorArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiWebMercatorArea::ClassName() const
{
  return "NFmiWebMercatorArea";
}

// ======================================================================
