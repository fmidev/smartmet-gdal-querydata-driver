// ======================================================================
/*!
 * \file NFmiLevelBag.cpp
 * \brief Implementation of class NFmiLevelBag
 */
// ======================================================================
/*!
 * \class NFmiLevelBag
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiLevelBag.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLevelBag::~NFmiLevelBag()
{
  try
  {
    if (itsLevels)
      delete[] static_cast<NFmiLevel *>(itsLevels);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag() : itsLevels(nullptr), itsStep(0) {}

// ----------------------------------------------------------------------
/*!
 * \param theLevelType Undocumented
 * \param theMinValue Undocumented
 * \param theMaxValue Undocumented
 * \param theStep Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(FmiLevelType theLevelType,
                           float theMinValue,
                           float theMaxValue,
                           float theStep)
    : NFmiSize(theStep ? static_cast<unsigned long>(((theMaxValue - theMinValue) / theStep) + 1)
                       : 1),
      itsLevels(nullptr),
      itsStep(theStep)
{
  try
  {
    itsLevels = new NFmiLevel[itsSize];
    for (unsigned long i = 0; i < itsSize; i++)
      itsLevels[i] = NFmiLevel(theLevelType, theMinValue + i * theStep);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevelArray Undocumented
 * \param numOfLevels Undocumented
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(NFmiLevel *theLevelArray, unsigned long numOfLevels)
    : NFmiSize(numOfLevels),
      itsLevels(theLevelArray ? new NFmiLevel[GetSize()] : nullptr),
      itsStep(0ul)
{
  try
  {
    if (itsLevels != nullptr)
      for (unsigned int i = 0; i < GetSize(); i++)
        itsLevels[i] = theLevelArray[i];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLevelBag::NFmiLevelBag(const NFmiLevelBag &theBag)
    : NFmiSize(theBag.itsIndex, theBag.itsSize),
      itsLevels(theBag.itsLevels ? new NFmiLevel[itsSize] : nullptr),
      itsStep(theBag.itsStep)
{
  try
  {
    if (itsLevels != nullptr)
      for (unsigned int i = 0; i < itsSize; i++)
        itsLevels[i] = (theBag.itsLevels)[i];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBag Undocumented
 * \todo Toimiiko vai eikö? Oli Markon epäilevä kommentti
 */
// ----------------------------------------------------------------------

const NFmiLevelBag NFmiLevelBag::Combine(const NFmiLevelBag &theBag) const
{
  try
  {
    return NFmiLevelBag(theBag);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::Level(const NFmiLevel &theLevel)
{
  try
  {
    Reset();
    while (Next())
    {
      if (theLevel == *Level())
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
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::AddLevel(const NFmiLevel &theLevel)
{
  try
  {
    int i;
    for (i = 0; i < static_cast<int>(itsSize); i++)
      if (itsLevels[i] == theLevel)
        break;

    if (i == static_cast<int>(itsSize))
    {
      int j;
      NFmiLevel *tempLevels;
      tempLevels = new NFmiLevel[GetSize() + 1];

      for (j = 0; j < static_cast<int>(itsSize); j++)
        tempLevels[j] = itsLevels[j];

      tempLevels[j] = theLevel;

      if (itsLevels)
        delete[] itsLevels;

      itsLevels = new NFmiLevel[itsSize + 1];
      itsSize = GetSize() + 1;
      for (j = 0; j < static_cast<int>(GetSize()); j++)
        itsLevels[j] = tempLevels[j];

      itsStep = 0ul;  // askel ei välttämättä enää vakio

      delete[] tempLevels;

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
 * Assignment operator
 *
 * \param theLevelBag The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiLevelBag &NFmiLevelBag::operator=(const NFmiLevelBag &theLevelBag)
{
  try
  {
    itsIndex = theLevelBag.CurrentIndex();
    itsSize = theLevelBag.GetSize();

    delete[] itsLevels;  // Salla 140998

    itsLevels = new NFmiLevel[theLevelBag.GetSize()];
    for (int i = 0; i < static_cast<int>(theLevelBag.GetSize()); i++)
      itsLevels[i] = theLevelBag.itsLevels[i];

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison operator
 *
 * \param theLevelBag The bag being compared to
 * \return True, if the bags are equal
 */
// ----------------------------------------------------------------------

bool NFmiLevelBag::operator==(const NFmiLevelBag &theLevelBag) const
{
  try
  {
    bool retVal = false;
    if (itsSize == theLevelBag.itsSize)
    {
      for (int i = 0;
           i < static_cast<int>(this->GetSize()) && static_cast<int>(theLevelBag.GetSize());
           i++)
      {
        if (!(this->itsLevels[i] == theLevelBag.itsLevels[i]))
          return false;
        else
          retVal = true;
      }
    }
    return retVal;
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

std::ostream &NFmiLevelBag::Write(std::ostream &file) const
{
  try
  {
    NFmiSize::Write(file);

    for (unsigned long i = 0; i < GetSize(); i++)
    {
      file << itsLevels[i];
    }

    file << itsStep << std::endl;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLevelBag::Read(std::istream &file)
{
  try
  {
    if (itsLevels)
      delete[] static_cast<NFmiLevel *>(itsLevels);

    NFmiSize::Read(file);
    if (GetSize())  // 11.02.97 Vili Add size check
      itsLevels = new NFmiLevel[GetSize()];

    for (unsigned long i = 0; i < GetSize(); i++)
    {
      file >> itsLevels[i];
    }

    file >> itsStep;

    Reset();
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
