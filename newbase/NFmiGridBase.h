// ======================================================================
/*!
 * \file NFmiGridBase.h
 * \brief Interface of class NFmiGridBase
 */
// ======================================================================

#pragma once

#include "NFmiDataPool.h"
#include "NFmiPoint.h"

//! Undocumented
class NFmiGridBase
{
 public:
  virtual ~NFmiGridBase();

  NFmiGridBase(const NFmiGridBase& theGridBase);

  NFmiGridBase(unsigned long theXNumber = 0,
               unsigned long theYNumber = 0,
               FmiDirection theStartingCorner = kBottomLeft,
               FmiInterpolationMethod theInterpolationMethod = kLinearly);
  //			   bool theBase = false) ;

  virtual bool Init(NFmiDataPool* theData);
  virtual bool Init(NFmiGridBase* theData);

  unsigned long OriginalXNumber() const;
  unsigned long OriginalYNumber() const;
  unsigned long XNumber() const;
  unsigned long YNumber() const;

  virtual unsigned long OriginalSize() const;
  virtual unsigned long Size() const;

  virtual NFmiDataPool* DataPool() const;

  virtual bool GridPoint(double x, double y);
  virtual const NFmiPoint GridPoint() const;
  unsigned long CurrentX() const { return itsCurrentX; }
  unsigned long CurrentY() const { return itsCurrentY; }
  bool First();
  bool Last();
  bool Reset();
  bool ResetLast();
  void Center();

  const NFmiPoint FirstGridPoint() const;
  const NFmiPoint LastGridPoint() const;

  void FirstX();
  void FirstY();
  void LastX();
  void LastY();

  void ResetX();
  void ResetY();
  void ResetLastX();
  void ResetLastY();

  void CenterX();
  void CenterY();

  unsigned long CalcFirstX() const;
  unsigned long CalcFirstY() const;

  bool IsInsideX(signed long numOfSteps) const;
  bool IsInsideY(signed long numOfSteps) const;
  bool IsInsideGrid(unsigned long XPointNum, unsigned long YPointNum) const;
  bool IsInsideGrid(const NFmiPoint&) const;
  bool IsInsideOriginalGrid(double xGrid, double yGrid) const;

  //  bool Base() const;

  virtual bool Next(unsigned long numberOfSteps = 1);
  virtual bool Previous(unsigned long numberOfSteps = 1);

  virtual bool Left(unsigned long numberOfSteps = 1);
  virtual bool Right(unsigned long numberOfSteps = 1);
  virtual bool Up(unsigned long numberOfSteps = 1);
  virtual bool Down(unsigned long numberOfSteps = 1);

  virtual bool NextRow();
  virtual const float* Row(long numberOfRowSteps = 0);

  double FloatValue(long dx = 0, long dy = 0) const;
  bool FloatValue(double data) const;

  double FloatValue(const NFmiPoint& gridPoint);
  bool FloatValue(double data, const NFmiPoint& gridPoint);

  void Origo(FmiDirection theStartingCorner);
  FmiDirection Origo() const;

  bool Crop(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight, bool theMinimize = true);
  bool ResetCrop();
  bool IsCropped() const;

  void Transform(NFmiTransformList& theList);
  void Clamp(double theClampMinValue, double theClampMaxValue);
  void Normalize();
  void Normalize(double theInputMinValue,
                 double theInputMaxValue,
                 double theOutputMinValue = 0.,
                 double theOutputMaxValue = 1.);

  virtual void ResetMissingValues();
  virtual void AddMissingValue(double theMissingValue);
  virtual bool IsMissingValue(double theMissingValue);

  FmiInterpolationMethod InterpolationMethod() const;
  void InterpolationMethod(FmiInterpolationMethod theInterpolationMethod);

  bool BiLinearInterpolation(double x, double y, double& theValue);
  bool FastBiVariateInterpolation(double x, double y, double& theValue);
  double DataValue(int x, int y);
  bool NearestPointInterpolation(double x, double y, double& theValue);

  bool InterpolateToGridPoint(double x, double y, double& theValue);
  bool InterpolateToGridPoint(const NFmiPoint& xy, double& theValue);

  bool InterpolateToGridPoint(double x, double y, double& theValue, FmiInterpolationMethod method);
  bool InterpolateToGridPoint(const NFmiPoint& xy, double& theValue, FmiInterpolationMethod method);

  void MeanFilter(unsigned long numOfSteps);

  double DeltaXValue(long numberOfXSteps = 1) const;
  double DeltaYValue(long numberOfYSteps = 1) const;
  double MinValue();
  double MaxValue();

  bool ReadBinaryData(const char* theFileName);
  bool ReadTextData(const char* theFileName);

  bool WriteBinaryData(const char* theFileName);
  bool WriteTextData(const char* theFileName);

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual const char* ClassName() const;

  unsigned long DataIndex() const;
  unsigned long Index() const;
  unsigned long DataIndex(unsigned long x, unsigned long y) const;
  unsigned long Index(unsigned long x, unsigned long y) const;
  bool Index(unsigned long theIndex);
  const NFmiPoint GridPoint(unsigned long theLocationIndex) const;
  bool operator==(const NFmiGridBase& theGrid) const;

  std::size_t HashValue() const;

 protected:
  bool Init(unsigned long theXNumber, unsigned long theYNumber);
  bool Swap(FmiDirection theCurrentDirection);

  bool NearestGridPoint(double& x, double& y) const;

  virtual void CalcX(unsigned long theIndex);
  virtual void CalcY(unsigned long theIndex);
  virtual void Setf(bool boo);

  bool Swap();

  bool MaximizedCrop(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight);
  bool MinimizedCrop(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight);

  NFmiDataPool* itsData;
  float* itsFloatData;
  FmiDirection itsStartingCorner;

  FmiInterpolationMethod itsInterpolationMethod;

  unsigned long itsXNumber;
  unsigned long itsYNumber;
  unsigned long itsCurrentX;
  unsigned long itsCurrentY;

  NFmiGridBase& operator=(const NFmiGridBase& theBase);

 private:
  //  bool itsBase;
  bool fWaitX;
  bool fWaitY;

  unsigned long itsFirstX;
  unsigned long itsFirstY;
  unsigned long itsLastX;
  unsigned long itsLastY;

  // Näiden datajäsenten tarkoitus: nopeuttaa gridin lineaarista interpolointia
  double itsPreviousX;
  double itsPreviousY;
  double itsTopLeftValue;
  double itsTopRightValue;
  double itsBottomLeftValue;
  double itsBottomRightValue;

};  // class NFmiGridBase

//! Undocumented, should be removed
typedef NFmiGridBase* PNFmiGridBase;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiGridBase::~NFmiGridBase()
{
  delete itsData;
  if (itsFloatData)
    delete[] itsFloatData;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theXNumber Undocumented
 * \param theYNumber Undocumented
 * \param theStartingCorner Undocumented
 * \param theInterpolationMethod Undocumented
 * \param theBase Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiGridBase::NFmiGridBase(unsigned long theXNumber,
                                  unsigned long theYNumber,
                                  FmiDirection theStartingCorner,
                                  FmiInterpolationMethod theInterpolationMethod)
    //						   bool theBase)
    : itsData(0),
      itsFloatData(0),
      itsStartingCorner(theStartingCorner),
      itsInterpolationMethod(theInterpolationMethod),
      itsXNumber(theXNumber),
      itsYNumber(theYNumber),
      itsCurrentX(0),
      itsCurrentY(0)
      //  , itsBase(theBase)
      ,
      fWaitX(true),
      fWaitY(true),
      itsFirstX(0),
      itsFirstY(0),
      itsLastX(theXNumber - 1),
      itsLastY(theYNumber - 1),
      itsPreviousX(kMinDouble),
      itsPreviousY(kMinDouble),
      itsTopLeftValue(kMaxDouble),
      itsTopRightValue(kMaxDouble),
      itsBottomLeftValue(kMaxDouble),
      itsBottomRightValue(kMaxDouble)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataPool* NFmiGridBase::DataPool() const
{
  return itsData;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::XNumber() const
{
  return (itsLastX - itsFirstX + 1);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::YNumber() const
{
  return (itsLastY - itsFirstY + 1);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::NextRow()
{
  return Next(XNumber());
}
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::DataIndex(unsigned long x, unsigned long y) const
{
  //  return ((y-itsBase)*itsXNumber + (x-itsBase));
  return ((y)*itsXNumber + (x));
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfRowSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const float* NFmiGridBase::Row(long numberOfRowSteps)
{
  return DataPool()->Data(DataIndex(itsFirstX, itsCurrentY + numberOfRowSteps), XNumber());
}

// ----------------------------------------------------------------------
/*!
 * \param theStartingCorner Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Origo(FmiDirection theStartingCorner)
{
  itsStartingCorner = theStartingCorner;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiDirection NFmiGridBase::Origo() const
{
  return itsStartingCorner;
}
// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Transform(NFmiTransformList& theList)
{
  itsData->Transform(theList);
}
// ----------------------------------------------------------------------
/*!
 * \param theInterpolationMethod Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::InterpolationMethod(FmiInterpolationMethod theInterpolationMethod)
{
  itsInterpolationMethod = theInterpolationMethod;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::OriginalXNumber() const
{
  return itsXNumber;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::OriginalYNumber() const
{
  return itsYNumber;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::OriginalSize() const
{
  return itsXNumber * itsYNumber;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::CalcFirstX() const
{
  //  return itsFirstX-itsBase;
  return itsFirstX;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::CalcFirstY() const
{
  //  return itsFirstY-itsBase;
  return itsFirstY;
}

// ----------------------------------------------------------------------
/*!
 * \param numOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsInsideX(signed long numOfSteps) const
{
  return (itsCurrentX + numOfSteps >= itsFirstX && itsCurrentX + numOfSteps <= itsLastX);
}

// ----------------------------------------------------------------------
/*!
 * \param numOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsInsideY(signed long numOfSteps) const
{
  return (itsCurrentY + numOfSteps >= itsFirstY && itsCurrentY + numOfSteps <= itsLastY);
}

// ----------------------------------------------------------------------
/*!
 * \param XPointNum Undocumented
 * \param YPointNum Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsInsideGrid(unsigned long XPointNum, unsigned long YPointNum) const
{
  return (XPointNum >= itsFirstX && XPointNum <= itsLastX && YPointNum >= itsFirstY &&
          YPointNum <= itsLastY);
}

// ----------------------------------------------------------------------
/*!
 * \param point Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsInsideGrid(const NFmiPoint& point) const
{
  // Negative coordinates were handled incorrectly for over 10 years
  // before this fix was added - Mika
  if (point.X() < 0 || point.Y() < 0)
    return false;

  return IsInsideGrid(static_cast<unsigned long>(point.X()), static_cast<unsigned long>(point.Y()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should this be const?
 */
// ----------------------------------------------------------------------

inline double NFmiGridBase::MinValue()
{
  return (itsData ? itsData->MinValue() : kFloatMissing);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should this be const?
 */
// ----------------------------------------------------------------------

inline double NFmiGridBase::MaxValue()
{
  return (itsData ? itsData->MaxValue() : kFloatMissing);
}
// ----------------------------------------------------------------------
/*!
 * \param theClampMinValue Undocumented
 * \param theClampMaxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Clamp(double theClampMinValue, double theClampMaxValue)
{
  itsData->Clamp(theClampMinValue, theClampMaxValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Normalize()
{
  itsData->Normalize();
}
// ----------------------------------------------------------------------
/*!
 * \param theInputMinValue Undocumented
 * \param theInputMaxValue Undocumented
 * \param theOutputMinValue Undocumented
 * \param theOutputMaxValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Normalize(double theInputMinValue,
                                    double theInputMaxValue,
                                    double theOutputMinValue,
                                    double theOutputMaxValue)
{
  itsData->Normalize(theInputMinValue, theInputMaxValue, theOutputMinValue, theOutputMaxValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::ResetMissingValues()
{
  itsData->ResetMissingValues();
}
// ----------------------------------------------------------------------
/*!
 * \param theMissingValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::AddMissingValue(double theMissingValue)
{
  itsData->AddMissingValue(theMissingValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theMissingValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsMissingValue(double theMissingValue)
{
  return itsData->IsMissingValue(theMissingValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::FirstX()
{
  fWaitX = false;
  GridPoint(static_cast<double>(itsFirstX), static_cast<double>(itsCurrentY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::FirstY()
{
  fWaitY = false;
  GridPoint(static_cast<double>(itsCurrentX), static_cast<double>(itsFirstY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::LastX()
{
  fWaitX = false;
  GridPoint(static_cast<double>(itsLastX), static_cast<double>(itsCurrentY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::LastY()
{
  fWaitY = false;
  GridPoint(static_cast<double>(itsCurrentX), static_cast<double>(itsLastY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::ResetX()
{
  FirstX();
  fWaitX = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::ResetY()
{
  FirstY();
  fWaitY = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::ResetLastX()
{
  LastX();
  fWaitX = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::ResetLastY()
{
  LastY();
  fWaitY = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::IsCropped() const
{
  return (itsXNumber != XNumber() || itsYNumber != YNumber());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
inline
bool NFmiGridBase::Base() const
{
  return itsBase;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param boo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::Setf(bool boo)
{
  fWaitX = boo;
  fWaitY = boo;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGridBase::FirstGridPoint() const
{
  return NFmiPoint(static_cast<double>(itsFirstX), static_cast<double>(itsFirstY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGridBase::LastGridPoint() const
{
  return NFmiPoint(static_cast<double>(itsLastX), static_cast<double>(itsLastY));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::DataIndex() const
{
  return DataIndex(itsCurrentX, itsCurrentY);
}
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::Index(unsigned long x, unsigned long y) const
{
  //  return (y-itsFirstY) * XNumber() + (x-itsFirstX) + itsBase;
  return (y - itsFirstY) * XNumber() + (x - itsFirstX);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::Index() const
{
  return Index(itsCurrentX, itsCurrentY);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiInterpolationMethod NFmiGridBase::InterpolationMethod() const
{
  return itsInterpolationMethod;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGridBase::Size() const
{
  return XNumber() * YNumber();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiGridBase::GridPoint() const
{
  return NFmiPoint(double(itsCurrentX), double(itsCurrentY));
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfXSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiGridBase::DeltaXValue(long numberOfXSteps) const
{
  return FloatValue() - FloatValue(numberOfXSteps);
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfYSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiGridBase::DeltaYValue(long numberOfYSteps) const
{
  return FloatValue() - FloatValue(0, numberOfYSteps);
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::InterpolateToGridPoint(double x_, double y_, double& theValue)
{
  return InterpolateToGridPoint(x_, y_, theValue, itsInterpolationMethod);
}

// ----------------------------------------------------------------------
/*!
 * \param xy_ Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::InterpolateToGridPoint(const NFmiPoint& xy_, double& theValue)
{
  return InterpolateToGridPoint(xy_.X(), xy_.Y(), theValue, itsInterpolationMethod);
}

// ----------------------------------------------------------------------
/*!
 * \param xy_ Undocumented
 * \param theValue Undocumented
 * \param method Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiGridBase::InterpolateToGridPoint(const NFmiPoint& xy_,
                                                 double& theValue,
                                                 FmiInterpolationMethod method)
{
  return InterpolateToGridPoint(xy_.X(), xy_.Y(), theValue, method);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::CenterX()
{
  GridPoint(FirstGridPoint().X() + (LastGridPoint().X() - FirstGridPoint().X()) / 2.,
            GridPoint().Y());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiGridBase::CenterY()
{
  GridPoint(GridPoint().X(),
            FirstGridPoint().Y() + (LastGridPoint().Y() - FirstGridPoint().Y()) / 2.);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiGridBase::ClassName() const
{
  return "NFmiGridBase";
}

// ======================================================================
