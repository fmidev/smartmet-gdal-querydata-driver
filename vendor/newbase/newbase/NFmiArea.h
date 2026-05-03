// ======================================================================
/*!
 * \file NFmiArea.h
 * \brief Interface of class NFmiArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiRect.h"
#include "NFmiSaveBaseFactory.h"
#include <gis/SpatialReference.h>
#include <string>

// Note: We could forward declare Fmi::SpatialReference too, but then all code using this or
// NFmiAreaTools would have to include Fmi::SpatialReference.h to enable implicit conversion of
// strings such as "WGS84" into actual spatial references. Having the include here is a smaller
// nuisance.

namespace Fmi
{
class CoordinateMatrix;
}

struct PacificPointFixerData
{
  PacificPointFixerData() : itsBottomLeftLatlon(), itsTopRightLatlon(), fIsPacific(false) {}
  PacificPointFixerData(const NFmiPoint &theBottomLeftLatlon,
                        const NFmiPoint &theTopRightLatlon,
                        bool isPacific)
      : itsBottomLeftLatlon(theBottomLeftLatlon),
        itsTopRightLatlon(theTopRightLatlon),
        fIsPacific(isPacific)
  {
  }

  NFmiPoint itsBottomLeftLatlon;
  NFmiPoint itsTopRightLatlon;
  bool fIsPacific;
};

//! Undocumented
class NFmiArea
{
 public:
  virtual ~NFmiArea();
  NFmiArea();
  NFmiArea(const NFmiArea &theArea);
  NFmiArea(const NFmiPoint &theTopLeftCorner,
           const NFmiPoint &theBottomRightCorner,
           bool usePacificView = false);
  NFmiArea(double theLeft,
           double theTop,
           double theRight,
           double theBottom,
           bool usePacificView = false);

  NFmiArea &operator=(const NFmiArea &theArea);

  void SetGridSize(std::size_t theWidth, std::size_t theHeight);

  bool IsInside(const NFmiPoint &theLatLonPoint) const;
  bool IsInside(const NFmiArea &theArea) const;

  const NFmiPoint TopLeftLatLon() const;
  const NFmiPoint TopRightLatLon() const;
  const NFmiPoint BottomLeftLatLon() const;
  const NFmiPoint BottomRightLatLon() const;
  const NFmiPoint CenterLatLon() const;

  const NFmiPoint TopLeft() const;
  const NFmiPoint BottomRight() const;
  const NFmiPoint TopRight() const;
  const NFmiPoint BottomLeft() const;

  void Place(const NFmiPoint &newPlace);
  void Size(const NFmiPoint &newSize);

  double Top() const;
  double Bottom() const;
  double Left() const;
  double Right() const;
  double Height() const;
  double Width() const;
  const NFmiRect &XYArea() const;

  virtual NFmiArea *Clone() const;
  virtual NFmiArea *CreateNewArea(const NFmiRect &theRect) const;

  virtual void Init(bool fKeepWorldRect = false);
  virtual void SetXYArea(const NFmiRect &newArea);
  virtual const NFmiRect XYArea(const NFmiArea *theArea) const;
  virtual const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const = 0;
  virtual const NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint WorldXYToXY(const NFmiPoint &theWorldXYPoint) const = 0;
  virtual const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const = 0;
  virtual const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const = 0;

  virtual NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const = 0;
  virtual NFmiArea *CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const;
  //  virtual NFmiArea * CreateNewArea(const NFmiRect & theRect);
  virtual NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect);

  // Added to enable enlarging areas
  NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect, bool fMustBeInside);

  virtual NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                                  FmiDirection theFixedPoint,
                                  bool fShrinkArea);

  virtual const NFmiRect WorldRect() const = 0;

  virtual const NFmiPoint WorldXYPlace() const;
  virtual const NFmiPoint WorldXYSize() const;
  virtual double WorldXYWidth() const;
  virtual double WorldXYHeight() const;
  virtual double WorldXYAspectRatio() const;

  virtual const NFmiAngle TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                           double theLatitudeEpsilon = 0.001) const;

  Fmi::CoordinateMatrix CoordinateMatrix(std::size_t nx, std::size_t ny, bool wrap) const;

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;
  virtual const std::string AreaStr() const = 0;
  virtual const std::string WKT() const = 0;
  std::string SimpleWKT() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual bool operator==(const NFmiArea &theArea) const;
  virtual bool operator!=(const NFmiArea &theArea) const;
  bool PacificView() const { return fPacificView; }
  void PacificView(bool newValue) { fPacificView = newValue; }
  void CheckForPacificView();

  static PacificPointFixerData PacificPointFixer(const NFmiPoint &theBottomLeftLatlon,
                                                 const NFmiPoint &theTopRightLatlon);
  static bool IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon);
  static bool IsPacificLongitude(double theLongitude);
  NFmiArea *DoPossiblePacificFix() const;
  NFmiArea *DoForcePacificFix() const;

  std::size_t HashValue() const;

  // Temporary fix until the above method is fixed to be virtual
  std::size_t HashValueKludge() const;

  const Fmi::SpatialReference &SpatialReference() const;
  std::string ProjStr() const;

  static NFmiArea *CreateFromBBox(const Fmi::SpatialReference &theSR,
                                  const NFmiPoint &theBottomLeftWorldXY,
                                  const NFmiPoint &theTopRightWorldXY);

  static NFmiArea *CreateFromCenter(const Fmi::SpatialReference &theSR,
                                    const Fmi::SpatialReference &theCenterSR,
                                    const NFmiPoint &theCenterLatLon,
                                    double theWidthInMeters,
                                    double theHeightInMeters);

  static NFmiArea *CreateFromCorners(const Fmi::SpatialReference &theSR,
                                     const Fmi::SpatialReference &theBBoxSR,
                                     const NFmiPoint &theBottomLeftLatLon,
                                     const NFmiPoint &theTopRightLatLon);

  static NFmiArea *CreateFromReverseCorners(const Fmi::SpatialReference &theSR,
                                            const Fmi::SpatialReference &theBBoxSR,
                                            const NFmiPoint &theTopLeftLatLon,
                                            const NFmiPoint &theBottomRightLatLon);

  static NFmiArea *CreateFromCornerAndSize(const Fmi::SpatialReference &theSR,
                                           const Fmi::SpatialReference &theCornerSR,
                                           const NFmiPoint &theBottomLeftLatLon,
                                           double theWidthInMeters,
                                           double theHeightInMeters);

 protected:
  int Sign(double theValue) const;
  double FixLongitude(double theLongitude) const;

  std::shared_ptr<Fmi::SpatialReference> itsSpatialReference;
  std::string itsProjStr;

 private:
  NFmiRect itsXYRectArea;
  bool fPacificView;

};  // class NFmiArea

//! Undocumented, should be removed

typedef NFmiArea *PNFmiArea;

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiArea
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiArea &ob)
{
  return ob.Write(file);
}
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiArea
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiArea &ob)
{
  return ob.Read(file);
}

// ======================================================================
