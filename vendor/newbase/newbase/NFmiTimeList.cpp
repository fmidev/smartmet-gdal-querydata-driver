// ======================================================================
/*!
 * \file NFmiTimeList.cpp
 * \brief Implementation of class NFmiTimeList
 */
// ======================================================================
/*!
 * \class NFmiTimeList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiTimeList.h"
#include "NFmiMetTime.h"
#include "NFmiTimeBag.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>
#include <functional>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

using namespace std;

NFmiTimeList::~NFmiTimeList()
{
  try
  {
    Clear(true);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theList The object being copied
 */
// ----------------------------------------------------------------------

NFmiTimeList::NFmiTimeList(const NFmiTimeList &theList)
    : itsVectorList(theList.itsVectorList),
      itsIndex(theList.itsIndex),
      itsIsReset(theList.itsIsReset)
{
  try
  {
    int vecSize = itsVectorList.size();
    for (int i = 0; i < vecSize; i++)
      itsVectorList[i] = new NFmiMetTime(*theList.itsVectorList[i]);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor that produces timelist from timebag.
 *
 * \param theTimes The object being copied
 */
// ----------------------------------------------------------------------

NFmiTimeList::NFmiTimeList(const NFmiTimeBag &theTimes)
    : itsVectorList(theTimes.GetSize()),
      itsIndex(theTimes.CurrentIndex()),
      itsIsReset(theTimes.CurrentIndex() >= 0 ? false : true)
{
  try
  {
    NFmiTimeBag tmpBag(theTimes);
    tmpBag.Reset();
    int vecSize = itsVectorList.size();
    for (int i = 0; i < vecSize; i++)
    {
      tmpBag.Next();
      itsVectorList[i] = new NFmiMetTime(tmpBag.CurrentTime());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
// Marko: en ole testannut funktiota, enkä tajua mihin sitä voisi tarvita, joten varokaa.
bool NFmiTimeList::Next(NFmiMetTime **theItem) const
{
  try
  {
    *theItem = Current();
    if (*theItem)
    {
      Next();
      return true;
    }

    return false;
    /*
    *theItem=Current();
    if(*theItem)
          {						// VAROITUS !
            itsIter->Next();		// Viimeisen alkion jälkeen mennään listan ulkopuolelle
    true:lla;
            return true;			// vasta seuraavalla kerralla tämä Next palauttaa
    false
    !!
            // return Next();		<-- Näin kursori jääsi osoittamaan viimeistä itemiä, mutta
    toisaalta nyt }						// return false on harhaan johtava,
    sillä onhan saatu mielekäs theItem.

          return false;			// Suosittelen metodien Next() & Current() käyttöä,
    jolloin ei voi joutua ulos listalta
  */  // viljo 12.05.-97
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

bool NFmiTimeList::Next() const
{
  try
  {
    if (itsIsReset)
      return First();

    itsIndex++;
    return IndexOk(itsIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeList::IndexOk(int theIndex) const
{
  try
  {
    if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsVectorList.size())
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

bool NFmiTimeList::Previous() const
{
  try
  {
    if (itsIsReset)
      return false;

    itsIndex--;
    return IndexOk(itsIndex);
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

NFmiMetTime *NFmiTimeList::Current() const
{
  try
  {
    if (IndexOk(itsIndex))
      return itsVectorList[itsIndex];

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

bool NFmiTimeList::Reset() const
{
  try
  {
    itsIsReset = true;
    itsIndex = -1;
    return true;
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

bool NFmiTimeList::First() const
{
  try
  {
    if (itsVectorList.empty())
    {
      Reset();
      return false;
    }

    itsIsReset = false;
    itsIndex = 0;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Lisää uusi kellonaika aikalistaan
 *
 * Oletusarvoisesti aikalista on sortattu kasvavaan järjestykseen.
 *
 * Add ei lisää annettua aikaa, jos se löytyy jo listasta ennestään.
 * Lisäksi objekti deletoidaan, kuten destruktorikin tekisi.
 *
 * Jos aikaa ei ole lisätty aiemmin, lisätään se aikajärjestyksessä
 * oikeaan kohtaan.
 *
 * \param theItem Pointteri lisättävään aikaan
 */
// ----------------------------------------------------------------------

void NFmiTimeList::Add(NFmiMetTime *theItem, bool fAllowDuplicates, bool fAddEnd)
{
  try
  {
    // etsitään ensimmäinen kohta, jossa vanha aika >= uusi aika

    auto it(itsVectorList.begin());

    if (fAddEnd)  // pakko optimoida, koska salamadatassa on jumalattomasti aikoja ja ne ovat jo
                  // järjestyksessä!!!
      itsVectorList.push_back(theItem);
    else
    {
      for (; it != itsVectorList.end(); ++it)
        if (**it >= *theItem)
          break;

      // jos kaikki ajat olivat pienempiä, liitetään vain perään
      if (it == itsVectorList.end())
        itsVectorList.push_back(theItem);

      // jos löytyi sama aika, ei insertoida vaan deletoidaan pois (paitsi jos duplikaatit
      // sallitaan)
      else if ((**it == *theItem) && (!fAllowDuplicates))
        delete theItem;

      // muuten insertoidaan oikeaan kohtaan
      else
        itsVectorList.insert(it, theItem);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeList::AddOver(NFmiMetTime *theItem)
{
  try
  {
    if (Find(*theItem))
    {
      delete theItem;
      return;
    }
    Add(theItem);
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

void NFmiTimeList::Add(NFmiTimeList *theList)
{
  try
  {
    int vecSize = theList->itsVectorList.size();
    for (int i = 0; i < vecSize; i++)
      itsVectorList.push_back(new NFmiMetTime(*theList->itsVectorList[i]));
    // vanha versio resetoi lopuksi, en näe mitää syytä moiseen/Marko

    /*
      *itsList+=(*theList->itsList);
      delete itsIter;
      itsIter = new NFmiVoidPtrIterator(itsList);
      itsIter->Reset();
      */
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeList::Clear(bool fDeleteData)
{
  try
  {
    if (fDeleteData)
    {
      int vecSize = itsVectorList.size();
      for (int i = 0; i < vecSize; i++)
        delete itsVectorList[i];
    }
    std::vector<NFmiMetTime *>().swap(itsVectorList);
    Reset();  // vanha laittoi tyhjennyksen jälkeen Firstiin, missä ei ole järkeä

    /*
      if( fDeleteData )
            {
              NFmiMetTime * aItem = 0;
              First();
              while(Next(&aItem))
                    delete aItem;
            }
      if(itsList)
            itsList->Clear();
      First();
      */
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
 * \param theList The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiTimeList &NFmiTimeList::operator=(const NFmiTimeList &theList)
{
  try
  {
    Clear(true);  // vanha vuoti, koska ei tuhonnut metTimeja listasta, saattaa kaataa ohjelmia,
                  // jotka luottavat tähän ominaisuuteen
    int vecSize = theList.itsVectorList.size();
    for (int i = 0; i < vecSize; i++)
      itsVectorList.push_back(new NFmiMetTime(*theList.itsVectorList[i]));

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::operator==(const NFmiTimeList &theList) const
{
  try
  {
    bool retVal = false;
    for (this->Reset(), theList.Reset(); this->Next() && theList.Next();)
    {
      if (!(this->Current()->IsEqual(*(theList.Current()))))
        return false;
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
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Index(int theIndex) const
{
  try
  {
    if (IndexOk(theIndex))
    {
      itsIndex = theIndex;
      return true;
    }
    return false;
    //  return itsIter?itsIter->Index(theIndex):false;
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

int NFmiTimeList::Index() const
{
  try
  {
    return itsIndex;
    //  return itsIter?itsIter->Index():false;		// index = -1 out of list
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

std::ostream &NFmiTimeList::Write(std::ostream &file) const
{
  try
  {
    long items = NumberOfItems();
    file << items << std::endl;
    First();
    NFmiMetTime *aItem;
    for (long i = 0; i < items; i++)
    {
      Next(&aItem);

      file << aItem->GetYear() << " " << aItem->GetMonth() << " " << aItem->GetDay() << " "
           << aItem->GetHour() << " " << aItem->GetMin() << " " << aItem->GetSec() << std::endl;
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

std::istream &NFmiTimeList::Read(std::istream &file)
{
  try
  {
    int items;
    short year, month, day, hour, min, sec;

    file >> items;
    First();
    NFmiMetTime *aItem;
    for (long i = 0; i < items; i++)
    {
      file >> year >> month >> day >> hour >> min >> sec;
      aItem = new NFmiMetTime(year, month, day, hour, min, sec, 1);
      if (sec)
        aItem->SetSec(sec);

      Add(aItem, true);  // true= sallitaan duplikaatit
    }
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// apu-funktori, jolla vertaillaan aikalistalla olevia aika-pointtereita
template <typename T>
struct ComparePtrs
{
  bool operator()(const T *lhs, const T *rhs) { return *lhs < *rhs; }
};

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Find(const NFmiMetTime &theTime)
{
  try
  {
    itsIndex = -1;
    if (itsVectorList.empty())
      return false;
    auto pos = std::lower_bound(
        itsVectorList.begin(), itsVectorList.end(), &theTime, ComparePtrs<NFmiMetTime>());
    if (pos != itsVectorList.end())
    {
      if (theTime == *(*pos))  // tässä pitää vielä tarkistaa löytyikö varmasti oikea aika!
      {
        itsIndex = std::distance(itsVectorList.begin(), pos);
        return true;
      }
    }
    return false;

    /*
      if(First())
            {
              do
                    {
                      if(*Current() == theTime)
                            return true;
                    }
              while(Next());
            }
      return false;
    */
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime is searched time
 * \param theDirection tells which direction search is done. If its kBackward, search
 *  only backwards from theTime. If kCenter, search in both directions and if kForward,
 *  search only forward from theTime.
 * \param theTimeRangeInMinutes tells how far you are allowed to search in wanted direction.
 * If theTimeRangeInMinutes value is kUnsignedLongMissing, there is no time limit in search.
 * \return Undocumented
 * Assumption: times in the itsVectorList are in rising order.
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::FindNearestTime(const NFmiMetTime &theTime,
                                   FmiDirection theDirection,
                                   unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (itsVectorList.empty())
      return false;

    auto firstNotLess = std::lower_bound(
        itsVectorList.begin(), itsVectorList.end(), &theTime, ComparePtrs<NFmiMetTime>());
    if (firstNotLess != itsVectorList.end() && *(*firstNotLess) == theTime)
    {
      // Searched time was found from time-vector
      itsIndex = CalcTimeListIndex(firstNotLess);
      return true;
    }

    if (theDirection == kBackward)
      return FindNearestBackwardTime(firstNotLess, theTime, theTimeRangeInMinutes);

    if (theDirection == kForward)
      return FindNearestForwardTime(firstNotLess, theTime, theTimeRangeInMinutes);

    return FindNearestTime(firstNotLess, theTime, theTimeRangeInMinutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestBackwardTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                                           const NFmiMetTime &theTime,
                                           unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (firstNotLess == itsVectorList.begin())
      return false;  // All times in itsVectorList were bigger than theTime

    firstNotLess--;  // Lets move to previous time which is what we are searching here (parameter's
                     // descriptive name false after this)
    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestForwardTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                                          const NFmiMetTime &theTime,
                                          unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (firstNotLess == itsVectorList.end())
      return false;  // All times in itsVectorList were less than theTime

    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestTime(std::vector<NFmiMetTime *>::iterator &firstNotLess,
                                   const NFmiMetTime &theTime,
                                   unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (firstNotLess == itsVectorList.begin())
    {
      // Only list's first time is possible
      return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
    }

    if (firstNotLess == itsVectorList.end())
    {
      // Only list's last time is possible
      firstNotLess--;
      return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
    }

    // Must check the first not-less time and the previous time
    auto timeIter2 = firstNotLess;
    double diff2 = std::fabs(theTime.DifferenceInMinutes(*(*timeIter2)));
    firstNotLess--;
    auto timeIter1 = firstNotLess;
    double diff1 = std::fabs(theTime.DifferenceInMinutes(*(*timeIter1)));
    // first time in the list has precedence if difference is equal
    if (diff1 <= diff2)
      return CheckFoundTimeIter(timeIter1, theTime, theTimeRangeInMinutes);

    return CheckFoundTimeIter(timeIter2, theTime, theTimeRangeInMinutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Assumption: theIter -iterator is from the itsVectorList.
int NFmiTimeList::CalcTimeListIndex(const std::vector<NFmiMetTime *>::iterator &theIter)
{
  try
  {
    return static_cast<int>(std::distance(itsVectorList.begin(), theIter));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeList::IsSearchedTimeInRange(std::vector<NFmiMetTime *>::iterator &foundTimeIter,
                                         const NFmiMetTime &theTime,
                                         unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (theTimeRangeInMinutes == kUnsignedLongMissing)
      return true;

    if (theTimeRangeInMinutes >= std::fabs(theTime.DifferenceInMinutes(*(*foundTimeIter))))
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTimeList::CheckFoundTimeIter(std::vector<NFmiMetTime *>::iterator &foundTimeIter,
                                      const NFmiMetTime &theTime,
                                      unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (IsSearchedTimeInRange(foundTimeIter, theTime, theTimeRangeInMinutes))
    {
      itsIndex = CalcTimeListIndex(foundTimeIter);
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
 * \param theTime Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// apu funktio nearestTime:en
// onko annettu aika tietyn rajan sisällä currentista ajasta?

bool NFmiTimeList::TimeInSearchRange(const NFmiMetTime &theTime,
                                     unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (theTimeRangeInMinutes == kUnsignedLongMissing ||
        static_cast<unsigned long>(std::abs(Current()->DifferenceInMinutes(theTime))) <
            theTimeRangeInMinutes)
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
 * \param theList This-otus ja theList yhdistetään halutulla tavalla.
 * \param theStartTimeFunction Mistä otetaan alkuaika, jos 0, pienempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \param theEndTimeFunction Mistä otetaan loppuaika, jos 0, suurempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \return Palautetaan yhdistetty timelist.
 */
// ----------------------------------------------------------------------

const NFmiTimeList NFmiTimeList::Combine(NFmiTimeList &theList,
                                         int theStartTimeFunction,
                                         int theEndTimeFunction)
{
  try
  {
    NFmiTimeList combinedList(*this);
    for (theList.Reset(); theList.Next();)
    {
      auto *tempTime = new NFmiMetTime(*theList.Current());
      combinedList.Add(tempTime, false, false);  // 1. false ei salli duplikaatteja, 2. false etsii
                                                 // ajan paikan olemassa olevasta listasta
    }
    if (theStartTimeFunction == 0 && theEndTimeFunction == 0)
      return combinedList;

    // pitää mahdollisesti karsia aikoja, en optimoinut ollenkaan
    NFmiMetTime startTime;
    if (theStartTimeFunction == 0)
      startTime = FirstTime() < theList.FirstTime() ? FirstTime() : theList.FirstTime();
    else if (theStartTimeFunction == 1)
      startTime = FirstTime();
    else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
      startTime = theList.FirstTime();

    NFmiMetTime endTime;
    if (theEndTimeFunction == 0)
      endTime = LastTime() > theList.LastTime() ? LastTime() : theList.LastTime();
    else if (theEndTimeFunction == 1)
      endTime = LastTime();
    else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
      endTime = theList.LastTime();

    return combinedList.GetIntersection(startTime, endTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

const NFmiMetTime &NFmiTimeList::FirstTime() const
{
  try
  {
    static NFmiMetTime dummy;
    if (IndexOk(0))
      return *itsVectorList[0];

    return dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime &NFmiTimeList::LastTime() const
{
  try
  {
    static NFmiMetTime dummy;
    int index = NumberOfItems() - 1;
    if (IndexOk(index))
      return *itsVectorList[index];

    return dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

int NFmiTimeList::CurrentResolution() const
{
  try
  {
    if (itsIndex > 0 && itsIndex < static_cast<int>(itsVectorList.size()))
      return itsVectorList[itsIndex]->DifferenceInMinutes(*itsVectorList[itsIndex - 1]);

    if (itsIndex == 0 && itsVectorList.size() > 1)
      return itsVectorList[itsIndex + 1]->DifferenceInMinutes(*itsVectorList[itsIndex]);
    // HUOM! loppu osa koodia on poikeus tapausten tarkastelua ja pitäisi heittää esim. poikkeus.
    // Mutta laitoin koodin toimimaan kuten timebag:in Resolution-metodi toimisi
    // (palauttaa aina arvon), ettei tule ongelmia erilaisissa koodeissa nyt kun
    // käytetään enemmän timelist-dataa.
    if (itsVectorList.size() > 1)
      // jos esim reset-tilassa, palauttaa 1. ja 2. ajan välisen resoluution
      return itsVectorList[1]->DifferenceInMinutes(*itsVectorList[0]);

    return 60;  // jos listassa on vain yksi tai nolla aikaa palautetaan 60 minuuttia
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// etsii annettuun aikaan lähimmän edellisen ja lähimmän seuraavan ajan ja palauttaa ne
// parametreina.
// Palauttaa ensimmäisen ajan indeksin jos löytyi. Oletus on että seuraavan ajan indeksi on
// palautettu arvo + 1.
// Jos homma ei onnistunut, palauttaa -1.
int NFmiTimeList::FindNearestTimes(const NFmiMetTime &theTime,
                                   int theMaxMinuteRange,
                                   NFmiMetTime &theTime1,
                                   NFmiMetTime &theTime2)
{
  try
  {
    int oldIndex = itsIndex;
    int returnIndex = -1;
    if (FindNearestTime(theTime, kBackward, theMaxMinuteRange))
    {
      if (itsIndex >= 0 && itsIndex < static_cast<int>(itsVectorList.size()) - 1)
      {
        theTime1 = *itsVectorList[itsIndex];
        theTime2 = *itsVectorList[itsIndex + 1];
        // Do not accept too long gaps
        if (theMaxMinuteRange < 0 || theTime2.DifferenceInMinutes(theTime1) <= theMaxMinuteRange)
          returnIndex = itsIndex;
      }
    }
    itsIndex = oldIndex;  // palautetaan indeksi osoittamaan varmuuden vuoksi takaisin
    return returnIndex;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime *NFmiTimeList::Time(int theIndex) const
{
  try
  {
    if (IndexOk(theIndex))
      return itsVectorList[theIndex];

    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee this timelistasta leikkauksen siten, että otetaan this-listasta
 * ne ajat jotka ovat annettujen rajojen sisältä/rajalta ja laitetaan ne
 * uuteen timelistiin joka palautetaan ulos.
 * Jos rajat ovat ohi kokonaan timelististä ja vielä samaan suuntaan,
 * palautetaan tyhjä timebagi. Samoin käy jos rajojen sisään ei sovi yhtään
 * timelistin aikaa.
 * \param theStartLimit Leikkausta rajoittava alkuaika.
 * \param theEndLimit Leikkausta rajoittava loppuaika.
 * \return Palauttaa tulos timelistin.
 */
// ----------------------------------------------------------------------

const NFmiTimeList NFmiTimeList::GetIntersection(const NFmiMetTime &theStartLimit,
                                                 const NFmiMetTime &theEndLimit)
{
  try
  {
    NFmiTimeList returnTimeList;
    NFmiMetTime tmpTime;
    for (Reset(); Next();)
    {
      tmpTime = *Current();
      if (tmpTime >= theStartLimit && tmpTime <= theEndLimit)
        returnTimeList.Add(new NFmiMetTime(tmpTime));
    }
    return returnTimeList;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Tarkistaa onko annettu aika aikalistan sisällä. Oletus: aika-
 * listassa olevat ajat ovat järjestyksessä
 * \param theTime Tarkistettava aika.
 * \return Palauttaa true jos theTime on listan sisällä tai reunalla, muuten false.
 */
// ----------------------------------------------------------------------
bool NFmiTimeList::IsInside(const NFmiMetTime &theTime) const
{
  try
  {
    int timeCount = NumberOfItems();
    if (timeCount < 1)
      return false;

    if (timeCount == 1)
      return *itsVectorList[0] == theTime;

    return *itsVectorList[0] <= theTime && theTime <= *itsVectorList[timeCount - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

// template<typename T>
struct PointerDestroyer
{
  template <typename T>
  void operator()(T *thePtr)
  {
    delete thePtr;
  }
};

// ----------------------------------------------------------------------
/*!
 * Karsii halutut ylimääräiset ajat pois
 * \param theMaxTimeCount Kertoo kuinka monta aikaa saa olla maksimissaan.
 * \param fFromEnd kertoo, mistä päästä karsitaan ylimääräiset pois.
 * \todo EN OLE SITTEN TESTANNUT T:Marko
 */
// ----------------------------------------------------------------------
void NFmiTimeList::PruneTimes(int theMaxTimeCount, bool fFromEnd)
{
  try
  {
    if (NumberOfItems() > theMaxTimeCount)
    {
      if (fFromEnd)
      {
        std::vector<NFmiMetTime *> tmpList(theMaxTimeCount);
        std::copy(itsVectorList.begin(), itsVectorList.begin() + theMaxTimeCount, tmpList.begin());
        std::for_each(
            itsVectorList.begin() + theMaxTimeCount, itsVectorList.end(), PointerDestroyer());
        itsVectorList.swap(tmpList);
      }
      else
      {
        std::vector<NFmiMetTime *> tmpList(theMaxTimeCount);
        std::copy(itsVectorList.end() - theMaxTimeCount, itsVectorList.end(), tmpList.begin());
        std::for_each(
            itsVectorList.begin(), itsVectorList.end() - theMaxTimeCount, PointerDestroyer());
        itsVectorList.swap(tmpList);
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
 * Muuttaa timelistin aikoja niin että annettu aika laitetaan ensimmäiseksi,
 * ja kaikille muille arvoille lasketaan siirtymä, että ajat ovat toisiinsa nähden
 * yhtä etäällä kuin aiemminkin. Timebagin rakenne ja 'resoluutio' säilyvät.
 * Lisäksi timelisti laitetaan osoittamaan 1. aikaa.
 * \param theTime on timebagin uusi firstTime.
 */
// ----------------------------------------------------------------------
void NFmiTimeList::SetNewStartTime(const NFmiMetTime &theTime)
{
  try
  {
    if (NumberOfItems() > 0)
    {
      long diffInMinutes = theTime.DifferenceInMinutes(*itsVectorList[0]);
      for (size_t i = 0; i < itsVectorList.size(); i++)
        itsVectorList[i]->ChangeByMinutes(diffInMinutes);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
