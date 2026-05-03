// ======================================================================
/*!
 * \file NFmiDataPool.h
 * \brief Interface of class NFmiDataPool
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiVoid.h"

#include <cstdlib>
#include <memory>
#include <ostream>
#include <vector>

class NFmiTransformList;

//! This is not used in newbase, and should hence be removed
typedef unsigned char FmiByte;

//! Undocumented
const int kMaxNumberOfDifferentMissingValues = 10;

//! Undocumented
class NFmiDataPool
{
 public:
  virtual ~NFmiDataPool();
  NFmiDataPool();
  NFmiDataPool(const NFmiDataPool& theDataPool);

  bool Init(unsigned long theNumber, const float* thePoolData);
  bool Init(unsigned long theNumber);
  bool InitMissingValues(const NFmiDataPool& theDataPool);
  virtual void Destroy();

  const float* Data() const;
  const float* Data(unsigned long theBeginIndex, unsigned long theNumberOfItems);

  unsigned long Number() const { return itsSize; }
  unsigned long Index() const;

  virtual bool IsInside() const;
  virtual bool IsInside(unsigned long theIndex) const;

  bool Index(unsigned long newIndex);

  virtual void Reset(long theIndex = -1);
  virtual void First();
  virtual void Last();
  virtual bool Next(unsigned long numberOfSteps = 1);
  virtual bool Previous(unsigned long numberOfSteps = 1);

  float FloatValue(unsigned long theIndex) const;
  float IndexFloatValue(unsigned long theIndex) const;
  float FloatValue() const;
  bool FloatValue(unsigned long theIndex, float theData);
  bool FloatValue(float theData);

  void ResetMissingValues();
  void AddMissingValue(double theMissingValue);
  bool IsMissingValue(double theMissingValue);

  double MinValue();
  double MaxValue();

  void Normalize();
  void Normalize(double theInputMinValue,
                 double theInputMaxValue,
                 double theOutputMinValue = 0.,
                 double theOutputMaxValue = 1.);
  void Clamp(double theClampMinValue, double theClampMaxValue);

  void Transform(NFmiTransformList& theList);

  bool ReadBinaryData(unsigned long theNumber, const char* theFileName);
  bool ReadTextData(unsigned long theNumber, const char* theFileName);
  bool WriteBinaryData(unsigned long theNumber, const char* theFileName);
  bool WriteTextData(unsigned long theNumber, const char* theFileName);

  void UseBinaryStorage(bool newState) const;
  bool IsBinaryStorageUsed();

  NFmiDataPool& operator=(const NFmiDataPool& theDataPool);
  bool operator==(const NFmiDataPool& theDataPool) const;
  bool operator!=(const NFmiDataPool& theDataPool) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  bool FloatValueAddress(unsigned long theIndex, float** address) const;
  bool MemCopy(unsigned long theNumber, float* theData);
  void DoEndianByteSwap(bool newValue);

 protected:
  bool CalcMinValue();
  bool CalcMaxValue();
  bool CalcMinMaxValues();

  // (Marko) nää pitäis katsoa järkeviksi (NFmiColorpool2 tarvitsee)
  void Data(float* theData);
  void DoEndianByteSwap();

 private:
  unsigned long itsSize;  // number of floats
  long itsIndex;          // index counter
  float* itsData;

  bool fFirst;
  bool fLast;

  double itsMinValue;
  double itsMaxValue;

  int itsMissingValueIndex;
  int itsCurrentMissingValueIndex;

  double itsMissingValue[kMaxNumberOfDifferentMissingValues];

  mutable bool fUseBinaryStorage;
  bool fDoEndianByteSwap;  // tämä tieto asetetaan ulkoa käsin (eli qdata kysyy qinfolta, pitääkö
                           // byte-swapata binääri dataa)
};

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiDataPool::~NFmiDataPool()
{
  NFmiDataPool::Destroy();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataPool::Index() const
{
  return itsIndex;
}
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::Reset(long theIndex)
{
  itsIndex = theIndex;
}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::First()
{
  itsIndex = 0;
  fFirst = true;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::Last()
{
  itsIndex = itsSize - 1;
  fLast = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataPool::IsInside() const
{
  return (itsIndex >= 0 && static_cast<unsigned long>(itsIndex) < itsSize);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataPool::IsInside(unsigned long theIndex) const
{
  return theIndex < itsSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const float* NFmiDataPool::Data() const
{
  return itsData;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataPool::FloatValue() const
{
  return FloatValue(itsIndex);
}
// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataPool::FloatValue(float theData)
{
  return FloatValue(itsIndex, theData);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::ResetMissingValues()
{
  itsMissingValueIndex = -1;
  itsCurrentMissingValueIndex = -1;
}

// ----------------------------------------------------------------------
/*!
 * \param theMissingValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::AddMissingValue(double theMissingValue)
{
  itsMissingValue[++itsMissingValueIndex] = theMissingValue;
}

// ----------------------------------------------------------------------
/*!
 * Maps values into range [theClampMinValue..theClampMaxValue]
 * Values less than theClampMinValue will be clamped to theClampMinValue.
 * Respectively, values greater than theClampMaxValue will be clamped to theClampMaxValue
 *
 * \param theClampMinValue The minimum allowed value
 * \param theClampMaxValue The maximum allowed value
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::Clamp(double theClampMinValue, double theClampMaxValue)
{
  Normalize(theClampMinValue, theClampMaxValue, theClampMinValue, theClampMaxValue);
}

// ----------------------------------------------------------------------
/*!
 * Maps values into range [0..1].
 * Values less than MinValue() will be clamped to MinValue().
 * Respectively, values greater than MaxValue() will be clamped to MaxValue()
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::Normalize()
{
  Normalize(MinValue(), MaxValue(), 0., 1.);
}
// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::UseBinaryStorage(bool newState) const
{
  fUseBinaryStorage = newState;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataPool::IsBinaryStorageUsed()
{
  return fUseBinaryStorage;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::DoEndianByteSwap(bool newValue)
{
  fDoEndianByteSwap = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataPool::Data(float* theData)
{
  itsData = theData;
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiDataPool
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& file, const NFmiDataPool& ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiDataPool
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream& operator>>(std::istream& file, NFmiDataPool& ob)
{
  return ob.Read(file);
}

// ======================================================================
