// ======================================================================
/*!
 * \file NFmiGrid.h
 * \brief Interface of class NFmiGrid
 */
// ======================================================================

#pragma once

#include "NFmiArea.h"
#include "NFmiDataMatrix.h"
#include "NFmiGridBase.h"

#include <algorithm>
#include <string>

class NFmiLocation;

namespace Fmi
{
class CoordinateMatrix;
}  // namespace Fmi

// NFmiLocationCache-luokka käytetään kun halutaan optimoida paikkaan liittyviä interpolaatioita.
class NFmiLocationCache
{
 public:
  NFmiLocationCache();
  ~NFmiLocationCache();

  bool NoValue() const { return itsLocationIndex == gMissingIndex; }
  bool NoInterpolation() const { return fNoInterpolation; }
  void CalcIsInterpolationNeeded(
      int theGridSizeX, int theGridSizeY);  // tämä lasketaan vain kerran kun cachea muodostetaan
  static NFmiLocationCache MakePeekedLocation(const NFmiLocationCache &theOrigLocation,
                                              int theOffsetX,
                                              int theOffsetY,
                                              int theGridSizeX,
                                              int theGridSizeY);
  void SetToNearestGridPoint(bool fDoX, bool fDoY, int theGridSizeX, int theGridSizeY);

  NFmiPoint itsGridPoint;
  unsigned long itsLocationIndex;
  bool fNoInterpolation;  // jos tämä on totta, ei tarvitse interpoloida paikan suhteen, arvot
                          // locationIndex:istä kelpaa suoraan
};

// NFmiTimeCache-luokkaa käytetään kun halutaan optimoida aikaan liittyviä interpolaatioita.
class NFmiTimeCache
{
 public:
  NFmiTimeCache();
  ~NFmiTimeCache();

  bool NoValue() const { return itsTimeIndex1 == gMissingIndex; }
  bool NoInterpolation() const { return itsTimeIndex1 == itsTimeIndex2; }
  void CalcIsInterpolationNeeded();  // tämä lasketaan vain kerran kun cachea muodostetaan

  unsigned long itsTimeIndex1;
  unsigned long itsTimeIndex2;
  float itsOffset;
};

//! Undocumented
class NFmiGrid : public NFmiGridBase
{
 public:
  virtual ~NFmiGrid();

  NFmiGrid(const NFmiArea *theArea = 0,
           unsigned long theXNumber = 0,
           unsigned long theYNumber = 0,
           FmiDirection theStartingCorner = kBottomLeft,
           FmiInterpolationMethod theInterpolationMethod = kLinearly);
  //		   bool theBase = false);

  NFmiGrid(const NFmiGrid &theGrid, FmiDirection theDirection = kBase);

  using NFmiGridBase::Init;
  virtual bool Init(NFmiDataPool *theData);
  virtual bool Init(NFmiGrid *theGrid);
  bool Init(NFmiGrid &theGrid, FmiInterpolationMethod howToInterpolate);
  bool Init(NFmiGrid &theGrid, NFmiGrid &theSubGrid, FmiInterpolationMethod howToInterpolate);

  bool Init(const std::string &theFileName,
            unsigned long nx,
            unsigned long ny,
            unsigned long theElementSizeInBytes,
            bool isLittleEndian = true,
            unsigned long theStartOffsetInBytes = 0,
            const std::string &theDataStartsAfterString = "",
            FmiDirection theStartingCorner = kTopLeft,
            bool walkXDimFirst = true,
            double theBeforeConversionMissingValue = kFloatMissing,
            float theAfterConversionMissingValue = kFloatMissing,
            bool isSigned = true,
            float theConversionScale = 1,
            float theConversionBase = 0);

  bool NearestLatLon(double newLon,
                     double newLat,
                     double theMaxDistance = kFloatMissing * 1000.,
                     NFmiPoint *theGridPoint = 0);

  std::vector<std::pair<int, double> > NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  const NFmiPoint XYToGrid(double x, double y) const;
  const NFmiPoint XYToGrid(const NFmiPoint &theXYPoint) const;
  const NFmiPoint GridToXY(double x_, double y_) const;
  const NFmiPoint GridToXY(const NFmiPoint &theGridPoint) const;
  const NFmiPoint LatLonToGrid(double theLon, double theLat) const;
  const NFmiPoint LatLonToGrid(const NFmiPoint &theLatLon) const;
  const NFmiPoint GridToLatLon(double x_, double y_) const;
  const NFmiPoint GridToLatLon(const NFmiPoint &theGridPoint) const;
  const NFmiPoint GridToWorldXY(double x_, double y_) const;
  const NFmiPoint GridToWorldXY(const NFmiPoint &theGridPoint) const;

  bool InterpolateToLatLonPoint(double theLon, double theLat, double &theValue);
  bool InterpolateToLatLonPoint(const NFmiPoint &theLatLon, double &theValue);
  bool InterpolateToXYPoint(double x, double y, double &theValue);
  bool InterpolateToXYPoint(NFmiPoint xy, double &theValue);

  bool InterpolateToLatLonPoint(double theLon,
                                double theLat,
                                double &theValue,
                                FmiInterpolationMethod method);

  bool InterpolateToLatLonPoint(const NFmiPoint &theLatLon,
                                double &theValue,
                                FmiInterpolationMethod method);

  bool InterpolateToXYPoint(double x, double y, double &theValue, FmiInterpolationMethod method);

  bool InterpolateToXYPoint(const NFmiPoint &xy, double &theValue, FmiInterpolationMethod method);

  const NFmiPoint LatLon() const;
  const NFmiPoint LatLon(unsigned long theIndex) const;
  const NFmiPoint WorldXY(unsigned long theIndex) const;
  const NFmiPoint RelativePoint() const;
  const NFmiPoint RelativePoint(unsigned long theIndex) const;
  const NFmiPoint XY() const;
  NFmiArea *Area() const;
  bool AreGridsIdentical(const NFmiGrid &theOtherGrid) const;

  NFmiPoint WorldXY() const;  // actual metric coordinate, or latlon for point data
  const Fmi::SpatialReference &SpatialReference() const;
  Fmi::CoordinateMatrix CoordinateMatrix(bool wrap) const;

  bool CropByLatLon(NFmiPoint &theBottomLeftLatLon,
                    NFmiPoint &theTopRightLatLon,
                    bool theMinimizedCrop = true);

  void Transform(NFmiTransformList &theList);

  double Coverage(const NFmiGrid &theGrid, unsigned long theStep);

  virtual NFmiGrid *CreateNewGrid(NFmiArea *newArea,
                                  unsigned long theXNumber,
                                  unsigned long theYNumber,
                                  FmiDirection theStartingCorner,
                                  FmiInterpolationMethod theInterpolationMethod);
  void CalcLatlonCachePoints(NFmiGrid &theTargetGrid,
                             NFmiDataMatrix<NFmiLocationCache> &theLocationCache);

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;
  bool operator==(const NFmiGrid &theGrid) const;
  NFmiGrid &operator=(const NFmiGrid &theGrid);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  bool IsInside(const NFmiPoint &theLatLon) const;

  static bool IsStrechableGlobalGrid(const NFmiGrid &theGrid);

  std::size_t HashValue() const;

 protected:
  bool SwapData(FmiDirection theStartingCorner, bool walkXDimFirst);

  NFmiArea *itsArea;

 private:
};  // class NFmiGrid

//! Undocumented, should be removed
typedef NFmiGrid *PNFmiGrid;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiGrid::~NFmiGrid()
{
  delete itsArea;
}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theArea Undocumented
 * \param theXNumber Undocumented
 * \param theYNumber Undocumented
 * \param theStartingCorner Undocumented
 * \param theInterpolationMethod Undocumented
 * \param theBase Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiGrid::NFmiGrid(const NFmiArea *theArea,
                          unsigned long theXNumber,
                          unsigned long theYNumber,
                          FmiDirection theStartingCorner,
                          FmiInterpolationMethod theInterpolationMethod)
    //				   bool theBase)
    : NFmiGridBase(theXNumber, theYNumber, theStartingCorner, theInterpolationMethod)
      //				 theBase)
      ,
      itsArea(theArea ? theArea->Clone() : 0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theGrid Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::Init(NFmiGrid *theGrid)
{
  return NFmiGridBase::Init(theGrid);
}
// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::Init(NFmiDataPool *theData)
{
  return NFmiGridBase::Init(theData);
}
// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::GridToXY(const NFmiPoint &theGridPoint) const
{
  return GridToXY(theGridPoint.X(), theGridPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::XY() const
{
  return GridToXY(GridPoint());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiArea *NFmiGrid::Area() const
{
  return itsArea;
}
// ----------------------------------------------------------------------
/*!
 * \param xy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::XYToGrid(const NFmiPoint &xy) const
{
  return XYToGrid(xy.X(), xy.Y());
}

// ----------------------------------------------------------------------
/*!
 * \param theLon Undocumented
 * \param theLat Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::LatLonToGrid(double theLon, double theLat) const
{
  return XYToGrid(itsArea->ToXY(NFmiPoint(theLon, theLat)));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::LatLonToGrid(const NFmiPoint &theLatLonPoint) const
{
  return XYToGrid(itsArea->ToXY(theLatLonPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::GridToLatLon(double x_, double y_) const
{
  return itsArea->ToLatLon(GridToXY(x_, y_));
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::GridToLatLon(const NFmiPoint &theGridPoint) const
{
  return GridToLatLon(theGridPoint.X(), theGridPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::GridToWorldXY(double x_, double y_) const
{
  return itsArea->XYToWorldXY(GridToXY(x_, y_));
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::GridToWorldXY(const NFmiPoint &theGridPoint) const
{
  return GridToWorldXY(theGridPoint.X(), theGridPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiPoint NFmiGrid::WorldXY() const
{
  return GridToWorldXY(GridPoint());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::LatLon() const
{
  return GridToLatLon(GridPoint());
}
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::LatLon(unsigned long theIndex) const
{
  return GridToLatLon(GridPoint(theIndex));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return WorldXY coordinates for given location index
 *
 * \param theIndex The location index
 * \return The WorldXY coordinates for the location index
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGrid::WorldXY(unsigned long theIndex) const
{
  return GridToWorldXY(GridPoint(theIndex));
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGrid::Transform(NFmiTransformList &theList)
{
  NFmiGridBase::Transform(theList);
}
// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theMinimizedCrop Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::CropByLatLon(NFmiPoint &theBottomLeftLatLon,
                                   NFmiPoint &theTopRightLatLon,
                                   bool theMinimizedCrop)
{
  return Crop(LatLonToGrid(theBottomLeftLatLon), LatLonToGrid(theTopRightLatLon), theMinimizedCrop);
}

// ----------------------------------------------------------------------
/*!
 * \param theLon Undocumented
 * \param theLat Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToLatLonPoint(double theLon, double theLat, double &theValue)
{
  return InterpolateToGridPoint(LatLonToGrid(theLon, theLat), theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToLatLonPoint(const NFmiPoint &theLatLon, double &theValue)
{
  return InterpolateToGridPoint(LatLonToGrid(theLatLon), theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theLon Undocumented
 * \param theLat Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToLatLonPoint(double theLon,
                                               double theLat,
                                               double &theValue,
                                               FmiInterpolationMethod method)
{
  return InterpolateToGridPoint(LatLonToGrid(theLon, theLat), theValue, method);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToLatLonPoint(const NFmiPoint &theLatLon,
                                               double &theValue,
                                               FmiInterpolationMethod method)
{
  return InterpolateToGridPoint(LatLonToGrid(theLatLon), theValue, method);
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToXYPoint(double x, double y, double &theValue)
{
  return InterpolateToGridPoint(XYToGrid(x, y), theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param xy Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToXYPoint(const NFmiPoint &xy,
                                           double &theValue,
                                           FmiInterpolationMethod method)
{
  return InterpolateToGridPoint(XYToGrid(xy), theValue, method);
}

// ----------------------------------------------------------------------
/*!
 * \param xy Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToXYPoint(NFmiPoint xy, double &theValue)
{
  return InterpolateToGridPoint(XYToGrid(xy), theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGrid::InterpolateToXYPoint(double x,
                                           double y,
                                           double &theValue,
                                           FmiInterpolationMethod method)
{
  return InterpolateToXYPoint(NFmiPoint(x, y), theValue, method);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGrid::ClassId() const
{
  return kNFmiGrid;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiGrid::ClassName() const
{
  return "NFmiGrid";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiGrid
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiGrid &ob)
{
  return ob.Write(file);
}
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiGrid
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiGrid &ob)
{
  return ob.Read(file);
}

// ======================================================================
