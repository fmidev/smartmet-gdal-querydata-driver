// ======================================================================
/*!
 * \file NFmiOrthographicArea.h
 * \brief Interface of class NFmiOrthographicArea
 */
// ======================================================================

#pragma once

#define MunFmiMax(x, y) ((x) > (y) ? (x) : (y))
#define MunFmiMin(x, y) ((x) < (y) ? (x) : (y))

#include "NFmiAzimuthalArea.h"

class NFmiOrthographicArea : public NFmiAzimuthalArea
{
 public:
  virtual ~NFmiOrthographicArea();

  NFmiOrthographicArea();

  NFmiOrthographicArea(const NFmiOrthographicArea &thePerspectiveArea);

  NFmiOrthographicArea(const NFmiPoint &theCentralLatLonPoint,
                       const double theRadialRange = kRearth,
                       const double theAzimuthAngle = 0.,
                       const NFmiPoint &theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint &theBottomRightXY = NFmiPoint(1.f, 1.f),
                       bool usePacificView = false);

  /*
          HUOM! NFmiOrthographicArea -PROJEKTIOSSA NURKKAPISTEIDEN MUUNNOS
          EI VÄLTTÄMÄTTÄ AINA OLE BIJEKTIO:
          ESIM. JOILLEKIN SUORAKULMAISEN KOORDINAATISTON XY-PISTEILLE EI LÖYDY
          VASTAAVIA MAANTIETEELLISIÄ LONLAT -PISTEITÄ (PISTEET OVAT "KARTTAPALLON" ULKOPUOLELLA)
  */
  NFmiOrthographicArea(const NFmiPoint &theBottomLeftLatLon,
                       const NFmiPoint &theTopRightLatLon,
                       const double theAzimuthAngle = 0.0,
                       const NFmiPoint &theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint &theBottomRightXY = NFmiPoint(1.f, 1.f),
                       bool usePacificView = false);

  virtual void Init(bool fKeepWorldRect = false);

  using NFmiAzimuthalArea::LatLonToWorldXY;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXY) const;

  void ZoomFactor(double theZoomFactor);
  double ZoomFactor() const;

  virtual double GlobeRadius() const;
  virtual void GlobeRadius(double &theGlobeRadius);

  virtual double AzimuthAngle() const;
  virtual void AzimuthAngle(double &theAzimuthAngle);

  virtual NFmiArea *Clone() const;
  virtual NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const;

  NFmiOrthographicArea &operator=(const NFmiOrthographicArea &theArea);
  bool operator==(const NFmiOrthographicArea &theArea) const;
  bool operator!=(const NFmiOrthographicArea &theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;
  bool operator==(const NFmiArea &theArea) const;
  bool operator!=(const NFmiArea &theArea) const;

  virtual const char *ClassName() const;
  const std::string AreaStr() const;
  virtual const std::string WKT() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  std::size_t HashValue() const;

 protected:
  virtual double K(const double /* delta */) const { return 1.; };
  virtual double CalcDelta(const double /* xyDistance */) const { return 0.; };
  virtual double DistanceFromPerspectivePointToCenterOfEarth() const { return 0.; };
  double itsAzimuthAngle;
  double itsLat0;
  double itsLon0;
  double itsSinLat0;
  double itsCosLat0;
  double itsVisibilityTerm;
  double itsZoomFactor;
  double itsGlobeRadius;
  NFmiPoint itsCurrentLatlonPoint;

 private:
};  // class NFmiOrthographicArea

//! Undocumented, should be removed
typedef NFmiOrthographicArea *PNFmiOrthographicArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiOrthographicArea::~NFmiOrthographicArea() {}
// ----------------------------------------------------------------------

inline double NFmiOrthographicArea::GlobeRadius() const
{
  return itsGlobeRadius;
}
// ----------------------------------------------------------------------

inline const char *NFmiOrthographicArea::ClassName() const
{
  return "kNFmiOrthographicArea";
}

// ======================================================================
