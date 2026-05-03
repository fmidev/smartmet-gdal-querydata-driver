#pragma once

#include "NFmiArea.h"

//! Undocumened
class NFmiLambertConformalConicArea : public NFmiArea
{
 public:
  virtual ~NFmiLambertConformalConicArea() = default;
  NFmiLambertConformalConicArea() = default;
  NFmiLambertConformalConicArea(const NFmiLambertConformalConicArea& theLambertConformalConicArea) =
      default;

  NFmiLambertConformalConicArea(const NFmiPoint& theBottomLeftLatLon,
                                const NFmiPoint& theTopRightLatLon,
                                double theCentralLongitude,
                                double theCentralLatitude,
                                double theTrueLatitude1,
                                double theTrueLatitude2,
                                double theRadius = kRearth,
                                bool usePacificView = false,
                                const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                                const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const;
  virtual const NFmiPoint WorldXYToXY(const NFmiPoint& theWorldXYPoint) const;

  virtual void Init(bool fKeepWorldRect = false);
  virtual NFmiArea* Clone() const;
  virtual NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                            const NFmiPoint& theTopRightLatLon,
                            bool allowPacificFix = true) const;

  virtual const NFmiRect WorldRect() const { return itsWorldRect; }

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const;

  NFmiLambertConformalConicArea& operator=(const NFmiLambertConformalConicArea& theArea) = default;

  bool operator==(const NFmiLambertConformalConicArea& theArea) const;
  bool operator!=(const NFmiLambertConformalConicArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const;
  bool operator!=(const NFmiArea& theArea) const;

  virtual unsigned long ClassId() const { return kNFmiLambertConformalConicArea; }

  virtual const char* ClassName() const { return "kNFmiLambertConformalConicArea"; }
  const std::string AreaStr() const;
  virtual const std::string WKT() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  std::size_t HashValue() const;

 private:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsCentralLongitude;
  double itsCentralLatitude;
  double itsTrueLatitude1;
  double itsTrueLatitude2;
  double itsRadius = kRearth;
  NFmiRect itsWorldRect;

  // Unique derived coefficients per projection:
  double itsXScaleFactor = 0;
  double itsYScaleFactor = 0;
  double itsN = 0;
  double itsF = 0;
  double itsRho0 = 0;

};  // class NFmiLambertConformalConicArea
