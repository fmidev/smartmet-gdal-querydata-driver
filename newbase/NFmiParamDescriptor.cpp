// ======================================================================
/*!
 * \file NFmiParamDescriptor.cpp
 * \brief Implementation of class NFmiParamDescriptor
 */
// ======================================================================
/*!
 * \class NFmiParamDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiParamDescriptor.h"
#include "NFmiDataDescriptor.h"
#include "NFmiParamBag.h"
#include <macgyver/Exception.h>
#include <cassert>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor()
    : itsParamBag(nullptr), itsActivity(nullptr), fInterpolate(false)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamBag Undocumented
 * \param interpolate Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor(const NFmiParamBag &theParamBag, bool interpolate)
    : itsParamBag(new NFmiParamBag(theParamBag)), itsActivity(nullptr), fInterpolate(interpolate)
{
  try
  {
    itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];
    for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
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
 * \param theParamDescriptor The other object being copied
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor::NFmiParamDescriptor(const NFmiParamDescriptor &theParamDescriptor)
    : NFmiDataDescriptor(),
      itsParamBag(theParamDescriptor.itsParamBag
                      ? new NFmiParamBag(*(theParamDescriptor.itsParamBag))
                      : nullptr),
      itsActivity(nullptr),
      fInterpolate(theParamDescriptor.fInterpolate)
{
  try
  {
    if (itsParamBag != nullptr)
    {
      itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];  // 5.3.1997/Marko
      for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
        itsActivity[i] = theParamDescriptor.itsActivity[i];
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

void NFmiParamDescriptor::Destroy()
{
  try
  {
    if (itsActivity != nullptr)
    {
      delete[] static_cast<bool *>(itsActivity);
      itsActivity = nullptr;
    }

    if (itsParamBag != nullptr)
    {
      delete itsParamBag;
      itsParamBag = nullptr;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(const NFmiDataIdent &theDataIdent)
{
  try
  {
    return itsParamBag->Current(theDataIdent, true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(const NFmiParam &theParam)
{
  try
  {
    return itsParamBag->Current(theParam, true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam The new parameter
 * \return True on success
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Param(FmiParameterName theParam)
{
  try
  {
    return itsParamBag->SetCurrent(theParam, false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug Must fix this to use the respective method in NFmiParamBag
 */
// ----------------------------------------------------------------------

// TÄMÄ PITÄÄ KORJATA KÄYTTÄMÄÄN PARAMBAGIN VASTAAVAA METODIA?!?!?!

unsigned long NFmiParamDescriptor::SizeActive() const
{
  try
  {
    unsigned long theActiveSize = 0;

    for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
      if (itsActivity[i] == true)
        theActiveSize++;

    return theActiveSize;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::SetActivity(bool theActivityState, bool fIgnoreSubParam)
{
  try
  {
    // ei käytetä enää parDesc:in aktiviteetti taulua, vaan antaa parambagin hoitaa homma
    return itsParamBag->SetCurrentActive(theActivityState == true, fIgnoreSubParam == true);
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

bool NFmiParamDescriptor::SetActivity(bool theActivityState)
{
  try
  {
    // ei käytetä enää parDesc:in aktiviteetti taulua, vaan antaa parambagin hoitaa homma
    return itsParamBag->SetCurrentActive(theActivityState == true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::SetActivity(bool theActivityState,
                                      unsigned long theIndex,
                                      bool fIgnoreSubParam)
{
  try
  {
    return itsParamBag->SetActive(theIndex, theActivityState, fIgnoreSubParam == true);
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

bool NFmiParamDescriptor::NextActive()
{
  try
  {
    while (Next())
      if (IsActive())
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
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::NextActive(bool fIgnoreSubParam)
{
  try
  {
    while (Next(fIgnoreSubParam))
      if (IsActive())
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
 * Assignment operator
 *
 * \param theParamDescriptor The other object being copied
 * \result The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiParamDescriptor &NFmiParamDescriptor::operator=(const NFmiParamDescriptor &theParamDescriptor)
{
  try
  {
    Destroy();

    itsParamBag = theParamDescriptor.itsParamBag ? new NFmiParamBag(*theParamDescriptor.itsParamBag)
                                                 : nullptr;

    if (itsParamBag)
    {
      itsActivity = new bool[itsParamBag->GetSize()];
      for (int i = 0; i < static_cast<int>(itsParamBag->GetSize()); i++)
        itsActivity[i] = theParamDescriptor.itsActivity[i];
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
 * \param theParamDescriptor The object being compared to
 * \result True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::operator==(const NFmiParamDescriptor &theParamDescriptor) const
{
  try
  {
    bool retVal = false;
    if (this->Size() == theParamDescriptor.Size())
    {
      for (int i = 0; i < static_cast<int>(Size()); i++)
        if (!(this->itsActivity[i] == theParamDescriptor.itsActivity[i]))
          return false;
    }
    if (this->itsParamBag && theParamDescriptor.itsParamBag)
    {
      return ((this->fInterpolate == theParamDescriptor.fInterpolate) &&
              (*(this->itsParamBag) == *(theParamDescriptor.itsParamBag)));
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
 * \param theCombine Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParamDescriptor NFmiParamDescriptor::Combine(const NFmiParamDescriptor &theCombine)
{
  try
  {
    if (itsParamBag && theCombine.itsParamBag)
      return NFmiParamDescriptor(itsParamBag->Combine(*(theCombine).itsParamBag));
    else if (itsParamBag)
      return NFmiParamDescriptor(*itsParamBag);
    else if (theCombine.itsParamBag)
      return NFmiParamDescriptor(*theCombine.itsParamBag);
    else
      return NFmiParamDescriptor();
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
 * \result The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiParamDescriptor::Write(std::ostream &file) const
{
  try
  {
    file << ClassId() << " " << ClassName() << std::endl;

    file << fInterpolate << " "
         << "0 "
         << "0 "
         << "0 " << std::endl;  // Varalla tulevaisuuta varten

    file << *itsParamBag;

    for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
      file << itsActivity[i] << " ";

    file << std::endl;

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
 * \result The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiParamDescriptor::Read(std::istream &file)
{
  try
  {
    Destroy();

    unsigned long classIdent;
    char dirty[30];
    file >> classIdent >> dirty;

    unsigned long theReserve;
    unsigned long theInterpolate;
    file >> theInterpolate >> theReserve >> theReserve >> theReserve;
    fInterpolate = theInterpolate != 0;

    itsParamBag = new NFmiParamBag;

    file >> *itsParamBag;

    itsActivity = new bool[static_cast<int>(itsParamBag->GetSize())];
    itsParamBag->Reset();
    for (unsigned long i = 0; i < itsParamBag->GetSize(); i++)
    {
      // We trust all data to be at least version 6 by now
      if (DefaultFmiInfoVersion >= 3)
        file >> itsActivity[i];
      else
      {
        itsParamBag->Next();
        itsActivity[i] = itsParamBag->Current()->IsDataParam();
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
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiParamDescriptor::Param(unsigned long theIndex, bool fIgnoreSubParam) const
{
  try
  {
    // 18.8.2000/Marko lisäsi tarkastuksia.
    // Paluuarvo on ongelmallinen, koska parambagi voi palauttaa 0 pointterin.
    // ParamDescriptorinkin pitäisi kai palauttaa pointteri?
    // Laitoin assertin jolloin ohjelma pysähtyy debug moodissa mutta ei release moodissa
    // jos 0-pointteri.

    NFmiDataIdent *param = itsParamBag->Param(theIndex, fIgnoreSubParam == true);

    // 28.12.2001/Marko Poistin assertin ja palauttaa nyt virhetilanteessa dummy-dataidentin.
    // assert(param); // korjaa ohjelmaasi jos se pysähtyy tähän, tämä on vakava virhe (theIndex on
    // pielessä)

    if (param)
      return *param;
    else
    {
      // 28.12.2001/Marko Tämä on hätäviritys 'virhetilanteeseen',
      // jolloin palautetaan 1. parametrin tuottaja
      return *(itsParamBag->Param(0, fIgnoreSubParam == true));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiParamDescriptor::EditParam(unsigned long theIndex, bool fIgnoreSubParam)
{
  try
  {
    // 18.8.2000/Marko lisäsi tarkastuksia.
    // Paluuarvo on ongelmallinen, koska parambagi voi palauttaa 0 pointterin.
    // ParamDescriptorinkin pitäisi kai palauttaa pointteri?
    // Laitoin assertin jolloin ohjelma pysähtyy debug moodissa mutta ei release moodissa
    // jos 0-pointteri.

    NFmiDataIdent *param = itsParamBag->Param(theIndex, fIgnoreSubParam == true);

    // 28.12.2001/Marko Poistin assertin ja palauttaa nyt virhetilanteessa dummy-dataidentin.
    // assert(param); // korjaa ohjelmaasi jos se pysähtyy tähän, tämä on vakava virhe (theIndex on
    // pielessä)

    if (param)
      return *param;
    else
    {
      // 28.12.2001/Marko Tämä on hätäviritys virhetilanteeseen.
      static NFmiParam dummyParam(kFmiBadParameter, "virheparametri, korjaa koodiasi");
      static NFmiDataIdent dummy(dummyParam);
      return dummy;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::Index(unsigned long theIndex)
{
  try
  {
    return itsParamBag->SetCurrentIndex(theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Katsoo, löytyykö minkään parametrin aliparametreistä annettua parametriä.
 * Jos löytyy palauttaa true.
 *
 * \param theParam Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::FindSubParam(const NFmiParam &theParam)
{
  try
  {
    return itsParamBag->FindSubParam(theParam);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiParamDescriptor::FindSubParam(const NFmiDataIdent &theDataIdent)
{
  try
  {
    return itsParamBag->FindSubParam(theDataIdent);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
