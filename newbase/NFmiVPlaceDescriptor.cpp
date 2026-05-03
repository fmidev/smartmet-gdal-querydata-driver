// ======================================================================
/*!
 * \file NFmiVPlaceDescriptor.cpp
 * \brief Implementation of class NFmiVPlaceDescriptor
 */
// ----------------------------------------------------------------------

#include "NFmiVPlaceDescriptor.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::~NFmiVPlaceDescriptor()
{
  try
  {
    Destroy();
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

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor() : itsLevelBag(nullptr), itsActivity(nullptr) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLevelBag Undocumented
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor(NFmiLevelBag &theLevelBag)
    : itsLevelBag(new NFmiLevelBag(theLevelBag)), itsActivity(nullptr)
{
  try
  {
    itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
    for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
      itsActivity[i] = true;
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
 * \param theVPlaceDescriptor The other object being copied
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor::NFmiVPlaceDescriptor(const NFmiVPlaceDescriptor &theVPlaceDescriptor)
    : NFmiDataDescriptor(),
      itsLevelBag(theVPlaceDescriptor.itsLevelBag
                      ? new NFmiLevelBag(*theVPlaceDescriptor.itsLevelBag)
                      : nullptr),
      itsActivity(nullptr)
{
  try
  {
    if (itsLevelBag)
    {
      itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
      for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
      {
        itsActivity[i] = true;
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
 *
 */
// ----------------------------------------------------------------------

void NFmiVPlaceDescriptor::Destroy()
{
  try
  {
    if (itsActivity)
    {
      delete[] static_cast<bool *>(itsActivity);
      itsActivity = nullptr;
    }
    if (itsLevelBag)
    {
      delete itsLevelBag;
      itsLevelBag = nullptr;
    }
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

bool NFmiVPlaceDescriptor::Next()
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->Next();

    return false;
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

bool NFmiVPlaceDescriptor::Previous()  // 15.1.1997/Marko
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->Previous();

    return false;
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

NFmiLevel *NFmiVPlaceDescriptor::Level() const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->Level();

    return nullptr;
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

NFmiLevel *NFmiVPlaceDescriptor::Level(unsigned long theIndex) const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->Level(theIndex);

    return nullptr;
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

NFmiLevel *NFmiVPlaceDescriptor::LevelMinValue() const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->LevelMinValue();

    return nullptr;
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

NFmiLevel *NFmiVPlaceDescriptor::LevelMaxValue() const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->LevelMaxValue();

    return nullptr;
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
 * \todo Siirrä koodi NFmiLevelBag:iin!
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::Level(const NFmiLevel &theLevel)
{
  try
  {
    bool tempBoolean;
    itsLevelBag->Reset();

    do
    {
      tempBoolean = itsLevelBag->Next();
      if (!tempBoolean)
        break;
    } while (!(*itsLevelBag->Level() == theLevel));

    if (tempBoolean)
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
 * \param theLevelBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVPlaceDescriptor::LevelBag(NFmiLevelBag &theLevelBag)
{
  try
  {
    if (itsLevelBag)
      Destroy();

    itsLevelBag = new NFmiLevelBag(theLevelBag);

    itsActivity = new bool[Size()];
    for (int i = 0; i < static_cast<int>(Size()); i++)
      itsActivity[i] = true;
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

unsigned long NFmiVPlaceDescriptor::Index() const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->CurrentIndex();

    return static_cast<unsigned long>(-1);
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

unsigned long NFmiVPlaceDescriptor::Size() const
{
  try
  {
    if (itsLevelBag)
      return itsLevelBag->GetSize();

    return static_cast<unsigned long>(0);
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

bool NFmiVPlaceDescriptor::IsLevel() const
{
  try
  {
    if (itsLevelBag)
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::NextActive()
{
  try
  {
    while (Next())
    {
      if (IsActive())
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
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::SetActivity(bool theActivityState)
{
  try
  {
    bool temp = itsActivity[Index()];
    itsActivity[Index()] = theActivityState;
    return temp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 * \param theVPlaceDescriptor The other object being copied
 * \return The object assigned to
 * \bug Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiVPlaceDescriptor &NFmiVPlaceDescriptor::operator=(
    const NFmiVPlaceDescriptor &theVPlaceDescriptor)
{
  try
  {
    Destroy();

    itsLevelBag = theVPlaceDescriptor.itsLevelBag
                      ? new NFmiLevelBag(*theVPlaceDescriptor.itsLevelBag)
                      : nullptr;

    if (itsLevelBag)
    {
      itsActivity = new bool[itsLevelBag->GetSize()];
      for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
        itsActivity[i] = theVPlaceDescriptor.itsActivity[i];
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
 * \param theVPlaceDescriptor The other object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiVPlaceDescriptor::operator==(const NFmiVPlaceDescriptor &theVPlaceDescriptor) const
{
  try
  {
    bool retVal = false;
    if (this->itsLevelBag && theVPlaceDescriptor.itsLevelBag)
    {
      if (*(this->itsLevelBag) == *(theVPlaceDescriptor.itsLevelBag))
        retVal = true;
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

std::ostream &NFmiVPlaceDescriptor::Write(std::ostream &file) const
{
  try
  {
    if (itsLevelBag)
      file << *itsLevelBag;
    else
    {
      // Huom: Unixissa suora tulostus hankalaa, pitää castata objekti
      //       constiksi jne, hankalampaa lukea.
      NFmiLevelBag tmp;
      file << tmp;
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
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiVPlaceDescriptor::Read(std::istream &file)
{
  try
  {
    Destroy();

    itsLevelBag = new NFmiLevelBag;
    file >> *itsLevelBag;

    itsActivity = new bool[static_cast<int>(itsLevelBag->GetSize())];
    for (int i = 0; i < static_cast<int>(itsLevelBag->GetSize()); i++)
      itsActivity[i] = true;

    return file;
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

bool NFmiVPlaceDescriptor::Index(unsigned long theIndex)
{
  try
  {
    return itsLevelBag->SetCurrentIndex(theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
