// ======================================================================
/*!
 * \file NFmiProducerIdLister.cpp
 * \brief Implementation of class NFmiProducerIdLister
 */
// ======================================================================
/*!
 * \class NFmiProducerIdLister
 *
 *  Author         : pietarin
 *  Creation Date  : Wed - Nov 27, 2002
 *  Editoituun QueryDataan on tarkoitus laitta tieto, mitä alkuperäisiä tuottajia
 *  on käytetty kunkin ajanhetken pohjana. Tieto talletetaan QDatan QInfon
 *  stringlist:aan itsHeaderText jonkin 'avaimen' alle.
 *  Tämä luokka joka osaa tehdä tuottaja ID:eillä täytetyn stringin, jossa
 *  on myös tiedot alkuajasta ja aikaaskeleesta ja aika-askelten lukumäärästä
 *  ja lisäksi vielä listä tuottaja ID:tä.
 */
// ======================================================================

#include "NFmiProducerIdLister.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiQueryInfo.h"
#include "NFmiTimeList.h"
#include <macgyver/Exception.h>
#include <cmath>
#include <sstream>

using namespace std;

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiProducerIdLister::NFmiProducerIdLister()
    : itsTimes(),
      itsProducerString(),
      itsProducerIds(itsTimes.Size(), -1),
      itsModelOriginTimes(itsTimes.Size(), NFmiMetTime(1900, 1, 1, 0, 0)),
      itsDefaultProducerId(-1)
{
}

NFmiProducerIdLister::NFmiProducerIdLister(const NFmiTimeDescriptor &theTimes,
                                           int theDefaultProducerId)
    : itsTimes(theTimes),
      itsProducerString(),
      itsProducerIds(theTimes.Size(), theDefaultProducerId),
      itsModelOriginTimes(itsTimes.Size(), NFmiMetTime(1900, 1, 1, 0, 0)),
      itsDefaultProducerId(theDefaultProducerId)
{
}

NFmiProducerIdLister::NFmiProducerIdLister(const std::string &theProducersString)
    : itsTimes(),
      itsProducerString(),
      itsProducerIds(),
      itsModelOriginTimes(),
      itsDefaultProducerId(-1)
{
  try
  {
    IntepretProducerIdString(theProducersString);
    // pitäisikö paluuarvo tarkistaa ja jos epäonnistui, heittää poikkeus?
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiProducerIdLister::NFmiProducerIdLister(const NFmiProducerIdLister &theOther)

    = default;

// Huom! pitäisi olla const info, mutta pari metodia pitää muuttaa ensin
NFmiProducerIdLister::NFmiProducerIdLister(NFmiQueryInfo &theInfo)
    : itsTimes(),
      itsProducerString(),
      itsProducerIds(),
      itsModelOriginTimes(),
      itsDefaultProducerId(-1)
{
  try
  {
    if (theInfo.FindFirstKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey()))
      IntepretProducerIdString(string(theInfo.GetCurrentKeyValue()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// GetProducerIdString
//--------------------------------------------------------
/*!
 * Rakentaa timebagin ja producerId vectorin mukaisen stringin,
 * tallettaa sen dataosaksi ja palauttaa kopion siihen. Jos ei voinut
 * rakentaa stringiä jostain syystä, palauttaa tyhjän stringin.
 */
const std::string NFmiProducerIdLister::MakeProducerIdString()
{
  try
  {
    if (itsProducerIds.size() != itsTimes.Size())
      return "";

    ostringstream out;
    out << itsTimes.FirstTime() << " ";
    int vsize = itsTimes.Size();  // tämä on myös id ja origintime vektoreiden koko
    out << vsize << " ";
    if (itsTimes.UseTimeList())
    {
      out << "1 ";  // timelist-merkki
      for (itsTimes.Reset(); itsTimes.Next();)
        out << static_cast<long>(itsTimes.Resolution()) << " ";
    }
    else
    {
      out << "0 ";  // timebag-merkki
      out << static_cast<long>(itsTimes.Resolution()) << " ";
    }
    int i = 0;
    for (i = 0; i < vsize; i++)
    {
      out << itsProducerIds[i];
      out << " ";  // tässä laitetaan myös viimeisen perään space, koska jatketaan tämän perään
                   // origintimeilla
    }
    for (i = 0; i < vsize; i++)
    {
      out << itsModelOriginTimes[i];
      if (i < vsize - 1)  // viimeiseen ei enää laiteta spacea väliin
        out << " ";
    }
    itsProducerString = out.str();
    return itsProducerString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// IntepretProducerIdString
//--------------------------------------------------------
/*!
 * Purkaa annetun stringin jos mahdollista ja muodostaa siitä timebagi, ja producer id vectorin.
 * Palauttaa false, jos ei voinut purkaa/tulkata
 * annettua stringiä.
 */
bool NFmiProducerIdLister::IntepretProducerIdString(const std::string &theString)
{
  try
  {
    istringstream in(theString);
    NFmiMetTime firstTime;
    in >> firstTime;
    int vSize = 0;
    in >> vSize;
    int timeDescType = 0;  // 0=timebag ja 1=timelist
    in >> timeDescType;
    if (in.fail())
      return false;
    if (timeDescType == 0)
    {
      int timeResolutionInMinutes = 0;
      in >> timeResolutionInMinutes;
      if (in.fail())
        return false;
      NFmiMetTime lastTime(firstTime);
      lastTime.ChangeByMinutes(timeResolutionInMinutes * (vSize - 1));
      NFmiTimeBag times(firstTime, lastTime, timeResolutionInMinutes);
      NFmiTimeDescriptor timesDesc(firstTime, times);
      itsTimes = timesDesc;
    }
    else
    {
      NFmiTimeList timeList;
      int timeResolutionInMinutes = 0;
      NFmiMetTime currentTime(firstTime);
      timeList.Add(new NFmiMetTime(currentTime));
      for (int i = 0; i < vSize - 1;
           i++)  // tama menee yhta vajaa loppuun asti, koska viimeinen aika ero ei merkitse mitaan
      {
        if (i == 0)
          in >> timeResolutionInMinutes;  // 1. aikaresoluutio on turha ja se luetaan vain alta
                                          // pois!!!
        in >> timeResolutionInMinutes;
        if (in.fail())
          return false;
        currentTime.ChangeByMinutes(timeResolutionInMinutes);
        timeList.Add(new NFmiMetTime(currentTime));
      }
      NFmiTimeDescriptor timesDesc(firstTime, timeList);
      itsTimes = timesDesc;
    }

    std::vector<int> prodIds(vSize, -1);
    int prodId = -1;
    int i = 0;
    for (i = 0; i < vSize; i++)
    {
      in >> prodId;
      prodIds[i] = prodId;
    }
    NFmiMetTime orgTime(1900, 1, 1, 0, 0);
    std::vector<NFmiMetTime> originTimes(vSize, orgTime);
    for (i = 0; i < vSize; i++)
    {
      in >> orgTime;
      if (in.fail())
        break;  // pieleen meni ei jatketa
      originTimes[i] = orgTime;
    }

    if (!in.fail())
    {
      itsProducerString = theString;
      itsProducerIds.swap(prodIds);
      itsModelOriginTimes.swap(originTimes);
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// ChangeTimeResolution
//--------------------------------------------------------
/*!
 * Muuttaa esim. 3h resoluutioisen olion tunti resoluutioiseksi.
 * Tekee uuden timebagin ja tuottaja id vektorin ja täyttää sen niin,
 * että ID:t leviävät taaksepäin, eli eivät interpoloidu välille.
 * Uuden ja vanhan resoluution pitää olla toistensä suhteen tasa
 * jaollisia. Tarkoittaen sitä, että jos vanha oli 60 ja uusi on 180,
 * tämä sallittua, koska 180 on jaollinen 60:llä. Mutta jos vanha on
 * 60 ja uusi on 150, se ei käy, koska 150/60=2.5. Isomman luvun pitää
 * olla jaollinen tasan pienemmällä.
 */
bool NFmiProducerIdLister::ChangeTimeResolution(int theNewResolutionInMinutes)
{
  try
  {
    if (theNewResolutionInMinutes <= 0)
      return false;

    int currentRes = itsTimes.Resolution();
    double ratio = static_cast<double>(currentRes) / theNewResolutionInMinutes;
    NFmiTimeBag newTimeBag(itsTimes.FirstTime(), itsTimes.LastTime(), theNewResolutionInMinutes);
    if (ratio == 0)
      return false;
    else if (ratio == 1)
      return true;  // ei tarvitse tehdä mitään!
    else if (ratio > 1 && currentRes % theNewResolutionInMinutes != 0)
      return false;
    else if (ratio < 1 && theNewResolutionInMinutes % currentRes != 0)
      return false;

    int vSize = newTimeBag.GetSize();
    std::vector<int> newProdIds(vSize, -1);
    int i = 0;
    std::vector<NFmiMetTime> newOriginTimes(vSize, NFmiMetTime(1900, 1, 1, 0, 0));
    newTimeBag.Reset();
    for (i = 0; i < vSize; i++)
    {
      newTimeBag.Next();
      newProdIds[i] = ProducerId(newTimeBag.CurrentTime(), true);
      newOriginTimes[i] = ModelOriginTime(newTimeBag.CurrentTime(), true);
    }

    itsTimes = NFmiTimeDescriptor(newTimeBag.FirstTime(), newTimeBag);
    itsProducerIds.swap(newProdIds);
    itsModelOriginTimes.swap(newOriginTimes);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// ProducerId
//--------------------------------------------------------
int NFmiProducerIdLister::ProducerId(const NFmiMetTime &theTime, bool fAllowInterpolation)
{
  try
  {
    if (itsTimes.Time(theTime))
      return itsProducerIds[itsTimes.Index()];

    if (fAllowInterpolation)
    {
      if (itsTimes.TimeToNearestStep(theTime))
        return itsProducerIds[itsTimes.Index()];
    }
    return -1;  // -1 on virhekoodi/puuttuva koodi
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// ProducerId
//--------------------------------------------------------
int NFmiProducerIdLister::ProducerId(int theIndex) const
{
  try
  {
    if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsProducerIds.size())
      return itsProducerIds[theIndex];

    return -1;  // -1 on virhekoodi
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// ProducerId
//--------------------------------------------------------
void NFmiProducerIdLister::ProducerId(int theIndex, int theProducerId)
{
  try
  {
    if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsProducerIds.size())
      itsProducerIds[theIndex] = theProducerId;
    //	else
    // pitäisikö heittää poikkeus?
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//--------------------------------------------------------
// ProducerId
//--------------------------------------------------------
void NFmiProducerIdLister::ProducerId(const NFmiMetTime &theTime, int theProducerId)
{
  try
  {
    if (itsTimes.Time(theTime))
      itsProducerIds[itsTimes.Index()] = theProducerId;
    //	else
    // pitäisikö heittää poikkeus?
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime &NFmiProducerIdLister::ModelOriginTime(const NFmiMetTime &theTime,
                                                         bool fAllowInterpolation)
{
  try
  {
    static NFmiMetTime dummy(1900, 1, 1, 0, 0);
    if (itsTimes.Time(theTime))
      return itsModelOriginTimes[itsTimes.Index()];

    if (fAllowInterpolation)
    {
      if (itsTimes.TimeToNearestStep(theTime))
        return itsModelOriginTimes[itsTimes.Index()];
    }
    return dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiMetTime &NFmiProducerIdLister::ModelOriginTime(int theIndex) const
{
  try
  {
    static NFmiMetTime dummy(1900, 1, 1, 0, 0);
    if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsModelOriginTimes.size())
      return itsModelOriginTimes[theIndex];

    return dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiProducerIdLister::ModelOriginTime(const NFmiMetTime &theTime,
                                           const NFmiMetTime &theOriginTime)
{
  try
  {
    if (itsTimes.Time(theTime))
      itsModelOriginTimes[itsTimes.Index()] = theOriginTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiProducerIdLister::ModelOriginTime(int theIndex, const NFmiMetTime &theOriginTime)
{
  try
  {
    if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsModelOriginTimes.size())
      itsModelOriginTimes[theIndex] = theOriginTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiProducerIdLister::IsEmpty() const
{
  try
  {
    return itsProducerIds.empty();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
