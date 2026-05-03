// ======================================================================
/*!
 * \file NFmiDataPool.cpp
 * \brief Implementation of class NFmiDataPool
 */
// ======================================================================
/*!
 * \class NFmiDataPool
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataPool.h"
#include "NFmiString.h"
#include "NFmiTransformList.h"
#include <macgyver/Exception.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
// memcpy()

using namespace std;

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataPool::NFmiDataPool()
    : itsSize(0),
      itsIndex(-1),
      itsData(nullptr),
      fFirst(false),
      fLast(false),
      itsMinValue(kFloatMissing),
      itsMaxValue(kFloatMissing),
      itsMissingValueIndex(),
      itsCurrentMissingValueIndex(),
      itsMissingValue(),
      fUseBinaryStorage(false),
      fDoEndianByteSwap()
{
  ResetMissingValues();
  AddMissingValue(kFloatMissing);
}

// ----------------------------------------------------------------------
/*!
 * \param theDataPool Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataPool::NFmiDataPool(const NFmiDataPool &theDataPool)
    : itsSize(theDataPool.itsSize),
      itsIndex(theDataPool.itsIndex),
      itsData(nullptr),
      fFirst(false),
      fLast(false),
      itsMinValue(theDataPool.itsMinValue),
      itsMaxValue(theDataPool.itsMaxValue),
      itsMissingValueIndex(),
      itsCurrentMissingValueIndex(),
      itsMissingValue(),
      fUseBinaryStorage(theDataPool.fUseBinaryStorage),
      fDoEndianByteSwap()
{
  Init(itsSize, theDataPool.itsData);
  InitMissingValues(theDataPool);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
void NFmiDataPool::Destroy()
{
  try
  {
    if (itsData)
    {
      delete[] itsData;
      itsData = nullptr;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::Init(unsigned long theNumber)
{
  try
  {
    if (!theNumber)
      return false;

    Destroy();

    itsSize = theNumber;

#ifdef FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED
    try
#endif  // FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED
    {
      itsData = new float[itsSize];
    }
#ifdef FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED
    catch (...)
    {
      throw static_cast<double>(sizeof(float) * itsSize / (1024. * 1024.));

      // tässä on tarkoitus heittää speciaali poikkeus mutta vain meteorologin editorissa
      // tämä double poikkeus kertoo pyydetyn datan koon MB yksikössä ja että datassa sinänsä
      // ei ollut vikaa, mutta ei saatu varattua
      // tarpeeksi isoa yhtenäistä muistialuettä char taulukolle johon data luettaisiin.
    }
#endif  // FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED

    if (itsData)
    {
      // Test
      Reset();
      for (unsigned long i = 0; i < theNumber; i++)
        itsData[i] = kFloatMissing;

      return true;
    }
    else
    {
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::Init(unsigned long theNumber, const float *theData)
{
  try
  {
    if (!theNumber)
      return false;

    Destroy();

    itsSize = theNumber;
    itsData = new float[itsSize];

    memcpy(itsData, theData, itsSize * sizeof(float));
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataPool Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::InitMissingValues(const NFmiDataPool &theDataPool)
{
  try
  {
    itsMissingValueIndex = theDataPool.itsMissingValueIndex;
    itsCurrentMissingValueIndex = theDataPool.itsCurrentMissingValueIndex;

    memcpy(itsMissingValue,
           theDataPool.itsMissingValue,
           sizeof(double) * kMaxNumberOfDifferentMissingValues);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::Index(unsigned long newIndex)
{
  try
  {
    if (IsInside(newIndex))
    {
      itsIndex = long(newIndex);
      if (itsIndex == 0)
        fFirst = true;
      else if (newIndex == itsSize)
        fLast = true;
      return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::Next(unsigned long numberOfSteps)
{
  try
  {
    if (numberOfSteps && IsInside(itsIndex += numberOfSteps))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param numberOfSteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::Previous(unsigned long numberOfSteps)
{
  try
  {
    if (numberOfSteps && IsInside(itsIndex -= numberOfSteps))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param address Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::FloatValueAddress(unsigned long theIndex, float **address) const
{
  try
  {
    if (IsInside(theIndex))
    {
      *address = &(itsData[theIndex]);
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::MemCopy(unsigned long theNumber, float *theData)
{
  try
  {
    if (!(theNumber && itsData && (itsIndex != -1)))
      return false;

    if ((itsSize - itsIndex) >= theNumber)
    {
      memcpy(&(itsData[itsIndex]), theData, theNumber * sizeof(float));
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataPool::FloatValue(unsigned long theIndex) const
{
  try
  {
    if (!IsInside(theIndex))
      return kFloatMissing;

    return itsData[theIndex];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataPool::IndexFloatValue(unsigned long theIndex) const
{
  try
  {
    return FloatValue(theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param theData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::FloatValue(unsigned long theIndex, float theData)
{
  try
  {
    if (IsInside(theIndex))
    {
      itsData[theIndex] = theData;
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theMissingValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::IsMissingValue(double theMissingValue)
{
  try
  {
    // At first, try this:
    if (itsCurrentMissingValueIndex >= 0)
    {
      if (itsMissingValue[itsCurrentMissingValueIndex] == theMissingValue)
        return true;
    }

    int index = 0;
    bool valueFound = false;
    itsCurrentMissingValueIndex = -1;

    while (index <= itsMissingValueIndex)
    {
      valueFound = (itsMissingValue[index] == theMissingValue);
      if (valueFound)
      {
        itsCurrentMissingValueIndex = index;
        break;
      }

      index++;
    }

    return valueFound;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataPool::MinValue()
{
  try
  {
    if (fabs(itsMinValue) == kFloatMissing)  // Min and max have not been calculated yet
      CalcMinMaxValues();

    return itsMinValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataPool::MaxValue()
{
  try
  {
    if (fabs(itsMaxValue) == kFloatMissing)  // Min and max have not been calculated yet
      CalcMinMaxValues();

    return itsMaxValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::CalcMinMaxValues()
{
  try
  {
    unsigned long saveIndex = itsIndex;
    double value;

    itsMaxValue = kMinDouble;
    itsMinValue = kMaxDouble;

    Reset();
    while (Next())
    {
      value = FloatValue();

      if (!IsMissingValue(value))
      {
        if (value > itsMaxValue)
          itsMaxValue = value;

        if (value < itsMinValue)
          itsMinValue = value;
      }
    }

    itsIndex = saveIndex;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theInputMinValue Undocumented
 * \param theInputMaxValue Undocumented
 * \param theOutputMinValue Undocumented
 * \param theOutputMaxValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataPool::Normalize(double theInputMinValue,
                             double theInputMaxValue,
                             double theOutputMinValue,
                             double theOutputMaxValue)
{
  try
  {
    // Maps values from range [theInputMinValue..theInputMaxValue]
    // into a new range [theOutputMinValue..theOutputMaxValue].
    // Input values less than theInputMinValue will be clamped to theInputMinValue.
    // Respectively, values greater than theInputMaxValue will be clamped to theInputMaxValue
    //
    // NOTE: Data values coded as "missing data values" are not transformed anyhow

    // Find current min and max on the data pool
    double gridMin = MinValue();
    double gridMax = MaxValue();
    bool clamped = (gridMin < theInputMinValue) || (gridMax > theInputMaxValue);
    bool inputRangeEqualsOutputRange =
        (theInputMinValue == theOutputMinValue) && (theInputMaxValue == theOutputMaxValue);

    if (!clamped && inputRangeEqualsOutputRange)
      return;  // Unclamped input range equals output range - nothing to do !

    float currentValue;
    double transformedValue;
    double outputRange = theOutputMaxValue - theOutputMinValue;
    double inputRange = theInputMaxValue - theInputMinValue;
    double inputOutputRatio = 0.0;

    if (inputRange != 0.)
      inputOutputRatio = outputRange / inputRange;

    Reset();
    while (Next())
    {
      currentValue = FloatValue();
      if (!IsMissingValue(currentValue))
      {
        currentValue =
            std::min(static_cast<float>(theInputMaxValue),
                     std::max(currentValue, static_cast<float>(theInputMinValue)));  // Clamp

        if (inputRange != 0.)
          transformedValue =
              theOutputMinValue + (currentValue - theInputMinValue) * inputOutputRatio;
        else
          transformedValue = theOutputMaxValue;

        FloatValue(static_cast<float>(transformedValue));
      }
    }

    if (inputRange != 0.)
    {
      itsMinValue = theOutputMinValue + (gridMin - theInputMinValue) * inputOutputRatio;
      itsMaxValue = theOutputMaxValue + (gridMax - theInputMaxValue) * inputOutputRatio;
    }
    else
    {
      itsMinValue = theOutputMinValue;
      itsMaxValue = theOutputMaxValue;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataPool::Transform(NFmiTransformList &theList)
{
  try
  {
    // Transforms current data pool values as specified in the input transform list
    // NOTE: This method changes current data pool values - EVERY single data value
    // in the data pool gets transformed as specified in the input transformation list.
    // Min and max values get changed accordingly.

    double currentValue;
    float newValue;

    itsMaxValue = kMinDouble;
    itsMinValue = kMaxDouble;

    Reset();
    while (Next())
    {
      currentValue = FloatValue();
      if (!IsMissingValue(currentValue))
      {
        newValue = static_cast<float>(theList.Transform(currentValue));
        FloatValue(newValue);

        if (newValue < itsMinValue)
          itsMinValue = newValue;
        else if (newValue > itsMaxValue)
          itsMaxValue = newValue;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBeginIndex Undocumented
 * \param theNumberOfItems Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const float *NFmiDataPool::Data(unsigned long theBeginIndex, unsigned long theNumberOfItems)
{
  try
  {
    // Returns the pointer location in data pool corresponding to 'theBeginIndex'.
    // Also checks that at least 'theNumberOfItems' items can be safely read
    // starting from this location.

    if (!IsInside(theBeginIndex) || !IsInside(theBeginIndex + theNumberOfItems - 1))
      return nullptr;

    return &(itsData[theBeginIndex]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::ReadBinaryData(unsigned long theNumber, const char *theFileName)
{
  try
  {
    // TODO: miksei dataa lueta suoraan oikeaan paikkaan??

    std::ifstream in(theFileName, std::ios::binary);

    if (!in)
      return false;

    itsSize = theNumber;

    auto *floatdata = new float[theNumber];

    in.read(reinterpret_cast<char *>(floatdata), theNumber * sizeof(float));

    if (in.fail())
    {
      delete[] floatdata;
      throw Fmi::Exception(BCP,
                           "NFmiDataPool::ReadBinaryData - Failed to read binary querydata buffer");
    }

#ifdef IRIX
    if (theNumber > 1)
    {
      char tmp1, tmp2, tmp3, tmp4;
      char *ptr = reinterpret_cast<char *>(floatdata);
      for (long i = 3; i < theNumber * sizeof(float); i += 4)
      {
        tmp1 = ptr[i - 3];
        tmp2 = ptr[i - 2];
        tmp3 = ptr[i - 1];
        tmp4 = ptr[i - 0];

        ptr[i - 3] = tmp4;
        ptr[i - 2] = tmp3;
        ptr[i - 1] = tmp2;
        ptr[i - 0] = tmp1;
      }
    }
#endif

    Init(theNumber, floatdata);

    in.close();
    delete[] floatdata;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::ReadTextData(unsigned long theNumber, const char *theFileName)
{
  try
  {
    // TODO: miksei dataa lueta suoraan oikeaan paikkaan?

    std::ifstream in(theFileName);

    if (!in)
      return false;

    itsSize = theNumber;
    unsigned long i;

    auto *floatdata = new float[theNumber];

    for (i = 0; i < theNumber; i++)
      in >> floatdata[i];

    // Initialize data pool with array 'byteData'

    Init(theNumber, floatdata);

    in.close();
    delete[] floatdata;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::WriteBinaryData(unsigned long theNumber, const char *theFileName)
{
  try
  {
    std::ofstream out(theFileName, std::ios::binary);

    if (!out)
      return false;

    if (theNumber == 0)
      return false;

    out.write(reinterpret_cast<char *>(itsData), theNumber * sizeof(float));

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theNumber Undocumented
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::WriteTextData(unsigned long theNumber, const char *theFileName)
{
  try
  {
    std::ofstream out(theFileName);

    if (!out)
      return false;

    if (theNumber == 0)
      return false;

    for (unsigned long i = 0; i < theNumber; i++)
      out << itsData[i];

    out.close();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theDataPool The object being copied
 * \return The object assigned to
 * \todo Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiDataPool &NFmiDataPool::operator=(const NFmiDataPool &theDataPool)
{
  try
  {
    if (this != &theDataPool)
    {
      itsSize = theDataPool.itsSize;
      itsIndex = theDataPool.itsIndex;
      itsMinValue = theDataPool.itsMinValue;
      itsMaxValue = theDataPool.itsMaxValue;
      fFirst = theDataPool.fFirst;
      fLast = theDataPool.fLast;

      InitMissingValues(theDataPool);
      Init(theDataPool.itsSize, theDataPool.itsData);
    }

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theDataPool The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiDataPool::operator==(const NFmiDataPool &theDataPool) const
{
  try
  {
    // NOTE: Currently does NOT compare the missing value equality !

    if ((itsSize == theDataPool.itsSize) && (itsIndex == theDataPool.itsIndex) &&
        (itsMinValue == theDataPool.itsMinValue) && (itsMaxValue == theDataPool.itsMaxValue) &&
        (fFirst == theDataPool.fFirst) && (fLast == theDataPool.fLast))
    {
      if (memcmp(itsData, theDataPool.itsData, itsSize * sizeof(float)) == 0)
        return true;
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theDataPool The object being compared to
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

// NOTE: Currently does NOT compare the missing value inequality !

bool NFmiDataPool::operator!=(const NFmiDataPool &theDataPool) const
{
  try
  {
    return !(*this == theDataPool);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataPool::Write(std::ostream &file) const
{
  try
  {
    // Used to be data type - now old kFloat value 6 for backward compatibility
    const int kFloat = 6;
    file << kFloat << std::endl;

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 6)
    {
      file << fUseBinaryStorage << std::endl;
    }
    file << itsSize * sizeof(float) << std::endl;

    if (fUseBinaryStorage)
    {
      // Native endianness output
      file.write(reinterpret_cast<char *>(itsData), itsSize * sizeof(float));
      file << std::endl;
    }
    else
    {
      // Muutin metodin constiksi, jolloin Reset ja Next eivdt kelpaa

      long index = -1;
      while (IsInside(++index))
      {
        file << itsData[index] << " ";
      }
    }

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiDataPool::Read(std::istream &file)
{
  try
  {
    Destroy();

    // Backward compatibility only
    unsigned long theType;
    file >> theType;

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 6)
      file >> fUseBinaryStorage;
    else
      fUseBinaryStorage = false;

    int poolsize;
    file >> poolsize;
    if (poolsize % sizeof(float) != 0)
      throw Fmi::Exception(BCP, "Datapool size must be multiple of sizeof(float)");

    itsSize = poolsize / sizeof(float);

    Init(itsSize);

    if (fUseBinaryStorage)
    {
      char ch;
      file.get(ch);
      file.read(reinterpret_cast<char *>(itsData), itsSize * sizeof(float));
      DoEndianByteSwap();  // tarkistaa ensin, pitääkö swapata ensinkään!!
    }
    else
    {
      if (itsSize > 0)
      {
        Reset();
        while (Next())
        {
          file >> itsData[itsIndex];
        }
      }
    }

    if (file.fail())
      throw Fmi::Exception(BCP, "NFmiDataPool::Read - Failed to read querydata data buffer");

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
void NFmiDataPool::DoEndianByteSwap()
{
  try
  {
    if (fDoEndianByteSwap)  // tämä tieto on annettu ulkoa (qdata on kysynyt qinfolta, tarvitaanko
                            // swappaus)
    {
      // Mika: Lisätty swab operaatio
      if (itsSize > 1)
      {
        char tmp1, tmp2, tmp3, tmp4;
        char *ptr;
        ptr = reinterpret_cast<char *>(itsData);
        for (unsigned long i = 3; i < itsSize * sizeof(float); i += 4)
        {
          tmp1 = ptr[i - 3];
          tmp2 = ptr[i - 2];
          tmp3 = ptr[i - 1];
          tmp4 = ptr[i - 0];

          ptr[i - 3] = tmp4;
          ptr[i - 2] = tmp3;
          ptr[i - 1] = tmp2;
          ptr[i - 0] = tmp1;
        }
      }
      fDoEndianByteSwap = false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
