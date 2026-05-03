// ======================================================================
/*!
 * \file NFmiAreaMaskList.cpp
 * \brief Implementation of class NFmiAreaMaskList
 */
// ======================================================================
/*!
 * \class NFmiAreaMaskList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiAreaMaskList.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::~NFmiAreaMaskList() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::NFmiAreaMaskList() : itsMaskVector(), itsCurrentIndex(-1), fMaskInUse(false) {}

NFmiAreaMaskList::NFmiAreaMaskList(const NFmiAreaMaskList &theOther) = default;

NFmiAreaMaskList& NFmiAreaMaskList::operator = (const NFmiAreaMaskList&) = default;

std::shared_ptr<NFmiAreaMaskList> NFmiAreaMaskList::CreateShallowCopy(
    const std::shared_ptr<NFmiAreaMaskList> &theOther)
{
  try
  {
    if (theOther)
    {
      std::shared_ptr<NFmiAreaMaskList> copyMaskList(new NFmiAreaMaskList());
      for (theOther->Reset(); theOther->Next();)
      {
        std::shared_ptr<NFmiAreaMask> copyedMask(theOther->Current()->Clone());
        copyMaskList->Add(copyedMask);
      }
      copyMaskList->CheckIfMaskUsed();
      return copyMaskList;
    }

    return std::shared_ptr<NFmiAreaMaskList>();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

unsigned long NFmiAreaMaskList::NumberOfItems()
{
  try
  {
    return static_cast<unsigned long>(itsMaskVector.size());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theMask Undocumented, unused parameter
 * \return Undocumented
 *
 * \todo Investigate whether theMask parameter can be removed
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Add(std::shared_ptr<NFmiAreaMask> &theMask)
{
  try
  {
    itsMaskVector.push_back(theMask);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Reset()
{
  try
  {
    itsCurrentIndex = -1;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Next()
{
  try
  {
    itsCurrentIndex++;
    if (static_cast<std::size_t>(itsCurrentIndex) < itsMaskVector.size())
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tämä tarkistaa vektori-taulukkoon osoittavia indeksejä, jotka alkavat 0:sta.
bool NFmiAreaMaskList::IsValidIndex(int theIndex)
{
  try
  {
    if (theIndex >= 0 && static_cast<std::size_t>(theIndex) < itsMaskVector.size())
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
 * \return Undocumented, always zero pointer
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

std::shared_ptr<NFmiAreaMask> NFmiAreaMaskList::Current()
{
  try
  {
    if (IsValidIndex(itsCurrentIndex))
      return itsMaskVector[itsCurrentIndex];

    // jos indeksi on vektorin ulkopuolella, palautetaan 0-pointteri (shared_ptr tyhjänä)
    static std::shared_ptr<NFmiAreaMask> dummy;
    return dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Remove()
{
  try
  {
    if (IsValidIndex(itsCurrentIndex))
    {
      itsMaskVector.erase(itsMaskVector.begin() + itsCurrentIndex);
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
 *  Käy läpi listassa olevat maski-oliot ja kysyy
 *  niiltä IsMasked ja tekee tulosta AND operaation kanssa.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::IsMasked(const NFmiPoint &theLatLon)
{
  try
  {
    if (fMaskInUse)  // 1999.09.24/Marko Muutin tämän katsomaan ensin onko maski käytössä
    {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                     // näin pääsee eroon muutamasta ikävästä if-lause testeistä
      for (auto &index : itsMaskVector)
      {
        if (index->IsEnabled())
        {
          if (!(index->IsMasked(theLatLon)))
            return false;
        }
      }
      return true;
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * käy kaikki listan maskit läpi ja katsoo, ettei yhdenkään maskin kerroin ole 0,
 * tällöin palautetaan 0. Jos ei ole yhtään 0-kerrointa, lasketaan ramppimaskien
 * keskiarvo, joka palautetaan (tavallisia maskeja ei lasketa tässä koska ne
 * mukaan lukien tulisi liian rankkoja muutoksia). Jos vain tavallisia maskeja, palautetaan arvo 1.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always kFloatMissing
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskList::MaskValue(const NFmiPoint &theLatLon)
{
  try
  {
    if (fMaskInUse)  // 1999.09.24/Marko Muutin tämän katsomaan ensin onko maski käytössä
    {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                     // näin pääsee eroon muutamasta ikävästä if-lause testeistä
      double sum = 0, tempValue = 0;
      int count = 0;
      for (auto &index : itsMaskVector)
      {
        if (index->IsEnabled())
        {
          tempValue = index->MaskValue(theLatLon);
          if (!tempValue)
          {
            return 0.;
          }
          else
          {
            if (index->IsRampMask())
            {
              sum += tempValue;
              count++;
            }
          }
        }
      }
      if (count)
        return sum / count;
    }
    return 1.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *  Etsii annetun indeksin mukaisen otuksen listasta ja asettaa itsCurrentIndex:in osoittamaan
 * siihen.
 *  HUOM! theIndex on 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex 1:llä alkava indeksi jota etsitään maski vektorista.
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(unsigned long theIndex)
{
  try
  {
    int vectorIndex = static_cast<int>(theIndex) - 1;
    if (IsValidIndex(vectorIndex))
    {
      itsCurrentIndex = vectorIndex;
      return true;
    }

    itsCurrentIndex = -1;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Clear()
{
  try
  {
    itsMaskVector.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *  Tarkistaa listassa olevilta maskeilta, onko
 *  yhtään maskia käytössä (enabled). Jos on, asetetaan
 *  fMaskInUse attribuutti true:ksi jos ei asetetaan
 *  se false:ksi. Funktio palauttaa fMaskInUse:in arvon.
 *  Jatkossa listalta voidaan kysyä metodilla UseMask
 *  fMaskInUse:n tilaa. (tämä on optimointia varten tehty
 *  viritelmä)
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::CheckIfMaskUsed()
{
  try
  {
    fMaskInUse = false;
    for (auto &index : itsMaskVector)
    {
      if (index->IsEnabled())
        fMaskInUse = true;
    }
    return fMaskInUse;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *  Käy enabloidut maskit läpi antaa theTime:n
 *  niiden Time-metodille.
 * \param theTime Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::SyncronizeMaskTime(const NFmiMetTime &theTime)
{
  try
  {
    if (fMaskInUse)
    {
      for (auto &index : itsMaskVector)
      {
        if (index->IsEnabled())
          index->Time(theTime);
      }
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *  HUOM! theIndex on 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex theIndex alkaa 1:stä
 * \return jos löytyi halutulla indeksillä oleva maski, palauttaa true
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Index(unsigned long theIndex)
{
  try
  {
    return Find(theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam)
{
  try
  {
    for (auto &index : itsMaskVector)
    {
      if (index->IsWantedParam(theParam))
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
 * \param theParam Undocumented, unused
 * \param theLevel Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam, const NFmiLevel *theLevel)
{
  try
  {
    for (auto &index : itsMaskVector)
    {
      if (index->IsWantedParam(theParam, theLevel))
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
