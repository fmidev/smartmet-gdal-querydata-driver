// ======================================================================
/*!
 * \file NFmiQueryInfo.cpp
 * \brief Implementation of class NFmiQueryInfo
 * \todo Refactor
 */
// ======================================================================
/*!
 * \class NFmiQueryInfo
 *
 * Undocumented
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // poistaa MSVC++2008 kääntäjän tekemiä varoituksia: "warning
                                 // C4996: 'sprintf': This function or variable may be unsafe.
                                 // Consider using sprintf_s instead"
#endif

#include "NFmiQueryInfo.h"
#include "NFmiBitMask.h"
//#include "NFmiDataModifier.h"
#include "NFmiDataModifierExtreme.h"
#include "NFmiDataModifierExtremePlace.h"
#include "NFmiFastQueryInfo.h"  // Rumaa lapsiluokka riippuvaista koodia optimoinnin nimissä ModifyTimesLocationData_FullMT -metodissa
#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiInterpolation.h"
#include "NFmiQueryData.h"
#include "NFmiQueryInfoSatel.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiStringList.h"
#include "NFmiTimeList.h"
#include "NFmiTotalWind.h"
#include "NFmiWeatherAndCloudiness.h"
#include <macgyver/Exception.h>
#include <macgyver/FileSystem.h>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
// abort()

#include <boost/math/special_functions.hpp>

// Mika: isspace on määritelty ctypessä
#ifdef UNIX
#include <cctype>
#endif

#ifdef _MSC_VER
#pragma warning( \
    disable : 4244 4267 4512)  // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244 4267 4512)  // laitetaan 4244 takaisin päälle, koska se on tärkeä
                                           // (esim. double -> int auto castaus varoitus)
#endif

using namespace std;

static const NFmiString gInfoHeaderKeyWord("FMI_");
#include "NFmiVersion.h"

// Jos on maailma dataa, joka ei mene maailman ympäri kokonaan, vaan esim. 0 -> 359.75
// koska tuo 360 asteen sarake olisi sama kuin 0 asteen sarake. Tällöin jos tarvitaan interpoloida
// alueelle 359.75 - 360 arvoja, käytetään tätä indeksiä merkitsemään NFmiLocationCache -luokan
// itsLocationIndex -dataosan arvoa, jotta osataan toimia oikein haettaessa interpolaatio arvoja
// datan 1. ja viimeisestä sarakkeesta.
static const unsigned long gStrechedGlobalGridIndex = static_cast<unsigned long>(-2);

#if 0
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \param topLeftValue Undocumented
 * \param topRightValue Undocumented
 * \param bottomLeftValue Undocumented
 * \param bottomRightValue Undocumented
 * \return Undocumented
 * \todo Refactor this into an external file
 *
 * The modular mean is calculated using the Mitsuta algorithm
 * for wind direction means.
 *
 * Reference: Mori, Y., 1986.<br>
 * <em>Evaluation of Several Single-Pass Estimators of the Mean and
 *     the Standard Deviation of Wind Direction.</em><br>
 * J Climate Appl. Metro., 25, 1387-1397.
 *
 * The formula for weighted mean is easily extrapolated.
 *
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ModBiLinearInterpolation(double x,
											 double y,
											 float & theValue,
											 float topLeftValue,
											 float topRightValue,
											 float bottomLeftValue,
											 float bottomRightValue)
{
  try
  {
    NFmiModMeanCalculator calculator;

    double dx = x - int(x);
    double dy = y - int(y);

    if( (dx != 0.) && (dx < 0.0001)) { dx = 0;}
    if( (dy != 0.) && (dy < 0.0001)) { dy = 0;}

    if(dx > (1-0.0001)) { dx = 1; }
    if(dy > (1-0.0001)) { dy = 1; }

    if(topLeftValue != kFloatMissing &&
     topRightValue != kFloatMissing &&
     bottomLeftValue != kFloatMissing &&
     bottomRightValue != kFloatMissing)
    {
      calculator(topRightValue, static_cast<float>(dx*dy));
      calculator(topLeftValue, static_cast<float>((1-dx)*dy));
      calculator(bottomRightValue, static_cast<float>(dx*(1-dy)));
      calculator(bottomLeftValue, static_cast<float>((1-dx)*(1-dy)));
      theValue = calculator();
      return true;
    }

    if(dy == 0. &&
     bottomLeftValue != kFloatMissing &&
     bottomRightValue != kFloatMissing)
    {
      calculator(bottomRightValue, static_cast<float>(dx));
      calculator(bottomLeftValue, static_cast<float>(1-dx));
      theValue =  calculator();
      return true;
    }

    if(dx == 0. &&
     bottomLeftValue != kFloatMissing &&
     topLeftValue != kFloatMissing)
    {
      calculator(topLeftValue, static_cast<float>(dy));
      calculator(bottomLeftValue, static_cast<float>(1-dy));
      theValue =  calculator();
      return true;
    }

    if(dy == 1. &&
     topLeftValue != kFloatMissing &&
     topRightValue != kFloatMissing)
    {
      calculator(topRightValue, static_cast<float>(dx));
      calculator(topLeftValue, static_cast<float>(1-dx));
      theValue =  calculator();
      return true;
    }

    if(dx == 1. &&
     bottomRightValue != kFloatMissing &&
     topRightValue != kFloatMissing)
    {
      calculator(topRightValue, static_cast<float>(dy));
      calculator(bottomRightValue, static_cast<float>(1-dy));
      theValue =  calculator();
      return true;
    }

    if(dx == 0.f &&
     dy == 0.f &&
     bottomLeftValue != kFloatMissing)
    {
      theValue =  bottomLeftValue;
      return true;
    }

    if(dx == 0.f &&
     dy == 1.f &&
     topLeftValue != kFloatMissing)
    {
      theValue =  topLeftValue;
      return true;
    }

    if(dx == 1.f &&
     dy == 0.f &&
     bottomRightValue != kFloatMissing)
    {
      theValue =  bottomRightValue;
      return true;
    }

    if(dx == 1.f &&
     dy == 1.f &&
     topRightValue != kFloatMissing)
    {
      theValue =  topRightValue;
      return true;
    }

    theValue = kFloatMissing;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tein myös aikainterpolaatiossa käytetyn version, eli
// minne annetaan kahden ajan välinen kerroin ja kaksi arvoa.
bool NFmiQueryInfo::ModBiLinearInterpolation(double factor,
											 float & theValue,
											 float value1,
											 float value2)
{
  try
  {
    if (value1 != kFloatMissing && value2 != kFloatMissing)
    {
      NFmiModMeanCalculator calculator;
      calculator(value2, static_cast<float>(1-factor));
      calculator(value1, static_cast<float>(factor));
      theValue = calculator();
      return true;
    }
    else if(value1 == kFloatMissing &&
        value2 != kFloatMissing)
    {
     theValue = value2;
     return true;
    }
    else if(value1 != kFloatMissing &&
        value2 == kFloatMissing)
    {
     theValue = value1;
     return true;
    }

    theValue = kFloatMissing;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(double theInfoVersion)
    : itsRefRawData(nullptr),
      itsRefQueryData(nullptr)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(),
      itsHPlaceDescriptor(),
      itsVPlaceDescriptor(),
      itsTimeDescriptor(),
      itsHeaderText(new NFmiStringList),
      itsPostProc(),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(nullptr),
      itsInfoVersion(theInfoVersion),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                             const NFmiTimeDescriptor &theTimeDescriptor,
                             const NFmiHPlaceDescriptor &theHPlaceDescriptor,
                             const NFmiVPlaceDescriptor &theVPlaceDescriptor,
                             double theInfoVersion)

    : itsRefRawData(nullptr),
      itsRefQueryData(nullptr)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(new NFmiParamDescriptor(theParamDescriptor)),
      itsHPlaceDescriptor(),
      itsVPlaceDescriptor(),
      itsTimeDescriptor(new NFmiTimeDescriptor(theTimeDescriptor)),
      itsHeaderText(new NFmiStringList),
      itsPostProc(),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(nullptr),
      itsInfoVersion(theInfoVersion),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  try
  {
    if (theVPlaceDescriptor.Size())
    {
      itsVPlaceDescriptor.reset(new NFmiVPlaceDescriptor(theVPlaceDescriptor));
    }
    else
    {
      NFmiLevelBag theLevelBag(kFmiAnyLevelType, 0, 0, 1);
      itsVPlaceDescriptor.reset(new NFmiVPlaceDescriptor(theLevelBag));
    }

    if (theHPlaceDescriptor.Size())
    {
      itsHPlaceDescriptor.reset(new NFmiHPlaceDescriptor(theHPlaceDescriptor));
    }
    else
    {
      NFmiLocationBag theLocationBag(new NFmiStation(), 1);
      itsHPlaceDescriptor.reset(new NFmiHPlaceDescriptor(theLocationBag));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theInfo Undocumented
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(NFmiQueryData *theInfo,
                             NFmiParamDescriptor *theParamDescriptor,
                             NFmiTimeDescriptor *theTimeDescriptor,
                             NFmiHPlaceDescriptor *theHPlaceDescriptor,
                             NFmiVPlaceDescriptor *theVPlaceDescriptor)
    : itsRefRawData(theInfo->itsRawData),
      itsRefQueryData(theInfo)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(theInfo->Info()->itsParamDescriptor
                             ? new NFmiParamDescriptor(*(theInfo->Info()->itsParamDescriptor))
                             : nullptr),
      itsHPlaceDescriptor(theInfo->Info()->itsHPlaceDescriptor
                              ? new NFmiHPlaceDescriptor(*(theInfo->Info()->itsHPlaceDescriptor))
                              : nullptr),
      itsVPlaceDescriptor(theInfo->Info()->itsVPlaceDescriptor
                              ? new NFmiVPlaceDescriptor(*(theInfo->Info()->itsVPlaceDescriptor))
                              : nullptr),
      itsTimeDescriptor(theInfo->Info()->itsTimeDescriptor
                            ? new NFmiTimeDescriptor(*(theInfo->Info()->itsTimeDescriptor))
                            : nullptr),
      itsHeaderText(),
      itsPostProc(),
      itsNewClassIdent(theInfo->Info()->itsNewClassIdent),
      itsCombinedParamParser(nullptr),
      itsInfoVersion(theInfo->InfoVersion()),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(0),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  try
  {
    if (theParamDescriptor != nullptr && itsParamDescriptor != nullptr)
    {
      itsParamDescriptor->Reset();
      while (itsParamDescriptor->Next())
        itsParamDescriptor->SetActivity(false);

      theParamDescriptor->Reset();
      while (theParamDescriptor->Next())
      {
        if (Param(theParamDescriptor->Param()))
        {
          itsParamDescriptor->SetActivity(true);
        }
      }
    }

    if (theTimeDescriptor != nullptr && itsTimeDescriptor != nullptr)
    {
      itsTimeDescriptor->Reset();
      while (itsTimeDescriptor->Next())
        itsTimeDescriptor->SetActivity(false);

      theTimeDescriptor->Reset();
      while (theTimeDescriptor->Next())
      {
        if (Time(theTimeDescriptor->Time()))
        {
          itsTimeDescriptor->SetActivity(true);
        }
      }
    }

    if (theHPlaceDescriptor != nullptr && itsHPlaceDescriptor != nullptr)
    {
      itsHPlaceDescriptor->Reset();
      while (itsHPlaceDescriptor->Next())
        itsHPlaceDescriptor->SetActivity(false);

      theHPlaceDescriptor->Reset();
      while (theHPlaceDescriptor->Next())
      {
        if (Location(*(theHPlaceDescriptor->Location())))
        {
          itsHPlaceDescriptor->SetActivity(true);
        }
      }
    }

    if (theVPlaceDescriptor != nullptr && itsVPlaceDescriptor != nullptr)
    {
      itsVPlaceDescriptor->Reset();
      while (itsVPlaceDescriptor->Next())
        itsVPlaceDescriptor->SetActivity(false);

      theVPlaceDescriptor->Reset();
      while (theVPlaceDescriptor->Next())
      {
        if (Level(*(theVPlaceDescriptor->Level())))
        {
          itsVPlaceDescriptor->SetActivity(true);
        }
      }
    }

    if (theInfo->Info()->Header())
      itsHeaderText.reset(new NFmiStringList(*(theInfo->Info()->Header())));

    if (theInfo->Info()->PostProcText())
      itsPostProc.reset(new NFmiStringList(*(theInfo->Info()->PostProcText())));

    // tehdään First, niin Info-iteraattori on heti käytettävissä
    // (ja mm. levelit osoittaa 1. leveliin, mikä unohtuu helposti käyttäjiltä)

    First();
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
 * \param theInfo The object being copied
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const NFmiQueryInfo &theInfo)
    : itsRefRawData(theInfo.itsRefRawData),
      itsRefQueryData(theInfo.itsRefQueryData)
      //  , itsStaticDataMask(theInfo.itsStaticDataMask)
      ,
      itsParamDescriptor(theInfo.itsParamDescriptor
                             ? new NFmiParamDescriptor(*(theInfo.itsParamDescriptor))
                             : nullptr),
      itsHPlaceDescriptor(theInfo.itsHPlaceDescriptor
                              ? new NFmiHPlaceDescriptor(*(theInfo.itsHPlaceDescriptor))
                              : nullptr),
      itsVPlaceDescriptor(theInfo.itsVPlaceDescriptor
                              ? new NFmiVPlaceDescriptor(*(theInfo.itsVPlaceDescriptor))
                              : nullptr),
      itsTimeDescriptor(theInfo.itsTimeDescriptor
                            ? new NFmiTimeDescriptor(*(theInfo.itsTimeDescriptor))
                            : nullptr),
      itsHeaderText(),
      itsPostProc(),
      itsNewClassIdent(theInfo.itsNewClassIdent),
      itsCombinedParamParser(
          theInfo.itsCombinedParamParser ? theInfo.itsCombinedParamParser->Clone() : nullptr),
      itsInfoVersion(theInfo.InfoVersion()),
      itsGridXNumber(theInfo.itsGridXNumber),
      itsGridYNumber(theInfo.itsGridYNumber)
      //  , fUseStaticDataMask(theInfo.fUseStaticDataMask)
      ,
      fIsStrechableGlobalGrid(theInfo.fIsStrechableGlobalGrid),
      itsTimeUnCertaintyStart(theInfo.itsTimeUnCertaintyStart),
      itsTimeUnCertaintyEnd(theInfo.itsTimeUnCertaintyEnd),
      itsAreaUnCertaintyStart(theInfo.itsAreaUnCertaintyStart),
      itsAreaUnCertaintyEnd(theInfo.itsAreaUnCertaintyEnd),
      fDoEndianByteSwap(theInfo.fDoEndianByteSwap),
      fHasNonFiniteValueSet(theInfo.fHasNonFiniteValueSet)
{
  try
  {
    if (theInfo.itsHeaderText)
    {
      itsHeaderText.reset(new NFmiStringList);
      *itsHeaderText = *theInfo.itsHeaderText;
    }

    if (theInfo.itsPostProc)
    {
      itsPostProc.reset(new NFmiStringList);
      *itsPostProc = *theInfo.itsPostProc;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------.
/*!
 * \brief Construct from file
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const string &filename)
    : itsRefRawData(nullptr),
      itsRefQueryData(nullptr)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(nullptr),
      itsHPlaceDescriptor(nullptr),
      itsVPlaceDescriptor(nullptr),
      itsTimeDescriptor(nullptr),
      itsHeaderText(nullptr),
      itsPostProc(nullptr),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(nullptr),
      itsInfoVersion(DefaultFmiInfoVersion),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  try
  {
    ifstream file(filename.c_str(), ios::in | ios::binary);
    if (!file)
      throw Fmi::Exception(BCP, "Could not open '" + filename + "' for reading");

    if (Fmi::is_compressed(filename))
    {
      Fmi::IStream filter(file, filename);
      Read(filter);
      if (!filter.good())
        throw Fmi::Exception(BCP, "Error while reading '" + filename + "'");
    }
    else
    {
      Read(file);
      if (!file.good())
        throw Fmi::Exception(BCP, "Error while reading '" + filename + "'");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::~NFmiQueryInfo()
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
 *
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::Destroy()
{
  try
  {
    itsHeaderText.reset();

    itsPostProc.reset();

    itsTimeDescriptor.reset();

    itsHPlaceDescriptor.reset();

    itsVPlaceDescriptor.reset();

    itsParamDescriptor.reset();

    delete itsCombinedParamParser;
    itsCombinedParamParser = nullptr;

    itsNewClassIdent = kNFmiQueryInfo;

    //  delete itsStaticDataMask;
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

void NFmiQueryInfo::Reset()
{
  try
  {
    if (itsParamDescriptor != nullptr)
      itsParamDescriptor->Reset();
    if (itsTimeDescriptor != nullptr)
      itsTimeDescriptor->Reset();
    if (itsHPlaceDescriptor != nullptr)
      itsHPlaceDescriptor->Reset();
    if (itsVPlaceDescriptor != nullptr)
      itsVPlaceDescriptor->Reset();
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

bool NFmiQueryInfo::First()
{
  try
  {
    Reset();
    return (itsTimeDescriptor->Next() && itsParamDescriptor->Next() &&
            itsHPlaceDescriptor->Next() && itsVPlaceDescriptor->Next());
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

bool NFmiQueryInfo::FirstParam(bool fIgnoreSubParam)
{
  try
  {
    ResetParam();
    return NextParam(fIgnoreSubParam);
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

/*----------------------------------------------------------------------------------------------*/
bool NFmiQueryInfo::FirstLocation()
{
  try
  {
    ResetLocation();
    return NextLocation();
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

bool NFmiQueryInfo::FirstLevel()
{
  try
  {
    ResetLevel();
    return NextLevel();
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

bool NFmiQueryInfo::FirstTime()
{
  try
  {
    ResetTime();
    return NextTime();
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

unsigned long NFmiQueryInfo::SizeActiveLocations() const
{
  try
  {
    return 0;  // itsHPlaceDescriptor->SizeActive(); TARKISTA
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

unsigned long NFmiQueryInfo::SizeActiveLevels() const
{
  try
  {
    return 0;  // itsVPlaceDescriptor?itsVPlaceDescriptor->SizeActive():0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug Does not work correctly
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiQueryInfo::EditParam()
{
  try
  {
    // ************** TÄMÄ TOIMII VÄÄRIN KORJAA!!!!! /Marko 1999.08.24 *********************
    //	if(fUseSubParam)
    //		return itsSubParam;
    return *(itsParamDescriptor->ParamBag()->Current());
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

size_t NFmiQueryInfo::Index() const
{
  try
  {
    //  if(!IsLocationMasked(itsHPlaceDescriptor->Index()))
    //	return static_cast<unsigned long>(-1);

    long timeIndex = itsTimeDescriptor->Index();
    long theVPlaceIndex = itsVPlaceDescriptor->Index();

    if (theVPlaceIndex == -1)
      theVPlaceIndex = 0;

    return ((itsParamDescriptor->Index()) * (itsHPlaceDescriptor->Size()) *
                (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
            (itsHPlaceDescriptor->Index()) * (itsVPlaceDescriptor->Size()) *
                (itsTimeDescriptor->Size()) +
            theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

size_t NFmiQueryInfo::Index(unsigned long theParamIndex,
                            unsigned long theLocationIndex,
                            unsigned long theLevelIndex,
                            unsigned long theTimeIndex) const
{
  try
  {
    //  if(!IsLocationMasked(theLocationIndex))
    //	return static_cast<unsigned long>(-1);

    if (theLevelIndex == static_cast<unsigned long>(-1))
      theLevelIndex = 0;

    size_t idx = ((theParamIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
                      (itsTimeDescriptor->Size()) +
                  (theLocationIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
                  theLevelIndex * itsTimeDescriptor->Size() + theTimeIndex);

    return idx;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiQueryInfo::Clone() const
{
  try
  {
    return new NFmiQueryInfo(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theCombine Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo NFmiQueryInfo::Combine(const NFmiQueryInfo &theCombine)
{
  try
  {
    return NFmiQueryInfo(itsParamDescriptor->Combine(*theCombine.itsParamDescriptor),
                         itsTimeDescriptor->Combine(*(theCombine).itsTimeDescriptor),
                         itsHPlaceDescriptor->Combine(*theCombine.itsHPlaceDescriptor),
                         *itsVPlaceDescriptor);
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

const NFmiString NFmiQueryInfo::Owner()
{
  try
  {
    if (!itsHeaderText)
      return NFmiString();

    if (!itsHeaderText->Reset())
      return NFmiString("Unsigned");

    return *(itsHeaderText->Current());
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

const NFmiString NFmiQueryInfo::TimeStamp()
{
  try
  {
    if (!itsHeaderText)
      return NFmiString();

    if (!itsHeaderText->Reset())
      return NFmiString("Unsigned");

    if (!itsHeaderText->Next())
      return NFmiString("Unsigned");

    return *(itsHeaderText->Current());
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

const NFmiString NFmiQueryInfo::Leader()
{
  try
  {
    if (!itsHeaderText)
      return NFmiString();

    if (!itsHeaderText->Reset())
      return NFmiString("Unsigned");

    if (!itsHeaderText->Next())
      return NFmiString("Unsigned");

    if (!itsHeaderText->Next())
      return NFmiString("Unsigned");

    return *(itsHeaderText->Current());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::Header(NFmiStringList *theHeader)
{
  try
  {
    if (theHeader)
    {
      itsHeaderText.reset(new NFmiStringList(*theHeader));
    }
    else
    {
      itsHeaderText.reset(new NFmiStringList());
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

NFmiStringList *NFmiQueryInfo::Header() const
{
  try
  {
    return itsHeaderText.get();
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

const NFmiString NFmiQueryInfo::Text() const
{
  try
  {
    if (!itsHeaderText)
      return NFmiString();

    return *(itsHeaderText->Current());
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

bool NFmiQueryInfo::ResetText()
{
  try
  {
    if (!itsHeaderText)
      return false;

    if (!itsHeaderText->Reset())
      return false;

    if (!itsHeaderText->Next())
      return false;

    if (!itsHeaderText->Next())
      return false;

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

bool NFmiQueryInfo::NextText()
{
  try
  {
    if (!itsHeaderText)
      return false;

    if (!itsHeaderText->Next())
      return false;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::PostProcText(NFmiStringList *theHeader)
{
  try
  {
    if (theHeader)
    {
      itsPostProc.reset(new NFmiStringList(*theHeader));
    }
    else
    {
      itsPostProc.reset(new NFmiStringList());
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

NFmiStringList *NFmiQueryInfo::PostProcText() const
{
  try
  {
    return itsPostProc.get();
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

bool NFmiQueryInfo::ResetPostProc()
{
  try
  {
    if (!itsPostProc)
      return false;

    if (!itsPostProc->Reset())
      return false;

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

bool NFmiQueryInfo::NextPostProc()
{
  try
  {
    if (!itsPostProc)
      return false;

    if (!itsPostProc->Next())
      return false;

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

const NFmiString NFmiQueryInfo::PostProc() const
{
  try
  {
    if (!itsPostProc)
      return NFmiString();

    return *(itsPostProc->Current());
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
/*
void * NFmiQueryInfo::VoidValue()
{
  return itsRefQueryData->VoidValue(*this);
}
*/

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiGrid NFmiQueryInfo::GridValue()
{
  try
  {
    if (Grid())
    {
      NFmiDataPool theDataPool;
      NFmiGrid theGrid(*Grid());
      auto *theFloatArray = new float[Grid()->Size()];
      unsigned long arrayCount = 0;
      ResetLocation();
      while (NextLocation())
      {
        theFloatArray[arrayCount++] = FloatValue();
      }
      theDataPool.Init(Grid()->Size(), theFloatArray);
      theGrid.Init(&theDataPool);
      delete[] theFloatArray;
      return theGrid;
    }

    return NFmiGrid();
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
 * \param theInfo The other object being copied
 * \return The object assigned to
 *
 * \bug Leaks memory, since no Destroy is called
 * \bug Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiQueryInfo &NFmiQueryInfo::operator=(const NFmiQueryInfo &theInfo)
{
  try
  {
    // HUOM!!!! Eikö tämä vuoda kun mitään ei deletoida??????
    // Pitäisi kutsua Destroy():ta !!!!!!!!! /Marko

    itsParamDescriptor.reset(new NFmiParamDescriptor(*theInfo.itsParamDescriptor));
    itsTimeDescriptor.reset(new NFmiTimeDescriptor(*theInfo.itsTimeDescriptor));
    itsHPlaceDescriptor.reset(new NFmiHPlaceDescriptor(*theInfo.itsHPlaceDescriptor));
    itsVPlaceDescriptor.reset(new NFmiVPlaceDescriptor(*theInfo.itsVPlaceDescriptor));

    itsRefRawData = theInfo.itsRefRawData;
    itsRefQueryData = theInfo.itsRefQueryData;

    if (theInfo.Header())
      itsHeaderText.reset(new NFmiStringList(*(theInfo.Header())));

    if (theInfo.PostProcText())
      itsPostProc.reset(new NFmiStringList(*(theInfo.PostProcText())));

    itsCombinedParamParser = nullptr;  // ei tarvita, jos kutsutaan Destroy():ta
    if (theInfo.itsCombinedParamParser)
      itsCombinedParamParser = theInfo.itsCombinedParamParser->Clone();
    itsTimeUnCertaintyStart = theInfo.itsTimeUnCertaintyStart;
    itsTimeUnCertaintyEnd = theInfo.itsTimeUnCertaintyEnd;
    itsAreaUnCertaintyStart = theInfo.itsAreaUnCertaintyStart;
    itsAreaUnCertaintyEnd = theInfo.itsAreaUnCertaintyEnd;
    fDoEndianByteSwap = theInfo.fDoEndianByteSwap;
    fHasNonFiniteValueSet = theInfo.fHasNonFiniteValueSet;
    fIsStrechableGlobalGrid = theInfo.fIsStrechableGlobalGrid;
    itsInfoVersion = theInfo.InfoVersion();
    itsGridXNumber = theInfo.itsGridXNumber;
    itsGridYNumber = theInfo.itsGridYNumber;

    return *this;
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

std::ostream &NFmiQueryInfo::Write(std::ostream &file) const
{
  try
  {
    static const int endianInfo = 0x4f464e49;  //  == 'INFO'

    // Tämä kun kirjoitetaan tiedostoon, tulee siitä dataan tieto missä
    // arkkitehtuurissa data tiedosto on talletettu. Tekstieditorissa katsottuna
    // little-endian-koneissa (esim. Windowsissa) kirjoitetussa datatiedostossa
    // se näkyy muodossa "INFO", mutta big-endian koneissa (esim. UNIX)
    // kirjoitettu datatiedossa näkyy teksti "OFNI". Tämä johtuu siitä, että
    // integer luku kirjoitetaan stringi muodossa ja big-endian koneissa
    // bytet ovat erilailla järjestyksessä kuin little-endian koneissa.

    file << "@$\260\243Q";  // '°' => \260 ja '£' => \243, koska cpp tiedoston character set
                            // muutettu Utf-8:ksi ja string literalien non-ascii koodaus muutettava.
    char buffer[5];
    strncpy(buffer,
            reinterpret_cast<const char *>(&endianInfo),
            4);  // rumaa, kirjoitetaan buffer:iin integer tavuina!
    buffer[4] = '\0';
    file.write(
        buffer,
        4);  // tämä kirjoittaa tiedostoon little-endian koneessa "INFO" ja big-endianissa "OFNI".
    file << "@$\260\243"  // '°' => \260 ja '£' => \243, koska cpp tiedoston character set muutettu
                          // Utf-8:ksi ja string literalien non-ascii koodaus muutettava.
         << " ";

    file << "VER " << itsInfoVersion << std::endl;

    file << ClassId() << " " << ClassName() << std::endl;
    file << "0 "
         << "0 "
         << "0 "
         << "0 " << std::endl;  // Varalla tulevaisuuta varten

    // Lisätty 9.3.1998/Vili
    if (itsHeaderText)
      file << *itsHeaderText;
    else
    {
      NFmiStringList tmp;
      file << tmp;
    }

    if (itsPostProc)
      file << *itsPostProc;
    else
    {
      NFmiStringList tmp;
      file << tmp;
    }

    if (itsParamDescriptor)
      file << *itsParamDescriptor;
    else
      file << 0 << " NFmiParamDescriptor" << std::endl;

    if (itsHPlaceDescriptor)
      file << *itsHPlaceDescriptor;
    else
      file << 0 << " NFmiHPlaceDescriptor" << std::endl;

    if (itsVPlaceDescriptor)
      file << *itsVPlaceDescriptor;
    else
    {
      NFmiVPlaceDescriptor theVPlaceDescriptor;
      file << theVPlaceDescriptor;
    }

    if (itsTimeDescriptor)
      file << *itsTimeDescriptor;
    else
      file << 0 << " NFmiTimeDescriptor" << std::endl;

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
 * \todo Kill the dog
 */
// ----------------------------------------------------------------------

std::istream &NFmiQueryInfo::Read(std::istream &file)
{
  try
  {
    const int correctEndianInfo =
        0x4f464e49;  //  == 'INFO' // little-endian systeemeissä mm. Windows
    const int swappedEndianInfo = 0x494e464f;  //  == 'OFNI' // big-endian systeemeissä mm. unix

    Destroy();

    char dirty[30];
    unsigned char buffer[6];
    file.read(reinterpret_cast<char *>(buffer), 5);  // tämä on luettava "@$°£Q"
    file.read(reinterpret_cast<char *>(buffer), 4);  // tässä tulee INFO binäärisenä integerinä

    // This is needed in case the stream is for example empty
    if (!file.good())
      throw Fmi::Exception(BCP, "NFmiQueryInfo::Read : Input stream does not contain querydata");

    const int infoInt = *reinterpret_cast<int *>(buffer);

    if (infoInt == correctEndianInfo)
      fDoEndianByteSwap = false;
    else if (infoInt == swappedEndianInfo)
      fDoEndianByteSwap = true;
    else
      throw Fmi::Exception(BCP, "NFmiQueryInfo::File does not contain endian-information bytes.");

    file.read(reinterpret_cast<char *>(buffer), 4);  // tämä on luettava "@$°£"

    // VERSIONHALLINTAA
    char space;
    file.get(space);

    char ver;
    file.get(ver);
    if (ver == 'V')
    {
      char tmp[3];
      file >> tmp;
      file >> itsInfoVersion;
      if (itsInfoVersion > DefaultFmiInfoVersion)
      {
        std::string errorMessage = __FUNCTION__;
        errorMessage += " trying to read higher queryData version data than supported, ";
        errorMessage += std::to_string(itsInfoVersion);
        errorMessage += " > ";
        errorMessage += std::to_string(DefaultFmiInfoVersion);
        throw Fmi::Exception(BCP, errorMessage);
      }
    }
    else
    {
      throw Fmi::Exception(BCP, "NFmiQueryInfo::File is not QueryInfo-class");
    }

    if (itsInfoVersion < 6)
    {
      throw Fmi::Exception(BCP, "Querydata versions below 6 are no longer supported");
    }

    // VERSIOHALLINTAA

    unsigned long classIdent;
    file >> classIdent >> dirty;

    if (classIdent != ClassId())
    {
      if (classIdent != kNFmiRadarQueryInfo)
      {
        itsNewClassIdent = classIdent;
        return file;
      }
      else
      {
        itsNewClassIdent = kNFmiRadarQueryInfo;
      }
    }

    unsigned long theReserve;
    file >> theReserve >> theReserve >> theReserve >> theReserve;

    try
    {
      itsHeaderText.reset(new NFmiStringList);
      file >> *itsHeaderText;

      itsPostProc.reset(new NFmiStringList);
      file >> *itsPostProc;

      itsParamDescriptor.reset(new NFmiParamDescriptor);
      file >> *itsParamDescriptor;

      itsHPlaceDescriptor.reset(new NFmiHPlaceDescriptor);
      file >> *itsHPlaceDescriptor;

      itsVPlaceDescriptor.reset(new NFmiVPlaceDescriptor);
      file >> *itsVPlaceDescriptor;

      if (!itsVPlaceDescriptor->Size())
      {
        itsVPlaceDescriptor.reset();
      }

      itsTimeDescriptor.reset(new NFmiTimeDescriptor);
      file >> *itsTimeDescriptor;
    }
    catch (...)
    {
      Destroy();
      throw;
    }

    // One final check on the validity of the input
    if (!file.good())
      throw Fmi::Exception(BCP,
                           "NFmiQueryInfo::Read : Input stream does not contain valid querydata");

    // FmiInfoVersion = OldFmiInfoVersion;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Parissa interpolointi funktiossa halutaan tietyille parametreille
// tehdä erikois interpolaatio. Siksi laitoin ehdon yhteen funktioon, että
// koodi toimisi aina samoin kaikkialla. Nyt ehdot olivat hieman
// erilaisia eri paikoissa.

static bool IsWeatherSubParam(const NFmiQueryInfo &theInfo)
{
  try
  {
    auto param = static_cast<FmiParameterName>(theInfo.Param().GetParam()->GetIdent());

    return (param == kFmiWeatherSymbol3 || param == kFmiWeatherSymbol1 ||
            param == kFmiPrecipitationForm || param == kFmiPrecipitationType ||
            param == kFmiMiddleAndLowCloudCover || param == kFmiProbabilityThunderstorm);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static const double gInterpolatedValueEps = 0.00001;

float NFmiQueryInfo::InterpolatedValue(const NFmiPoint &theLatLonPoint)
{
  try
  {
    if (!Param().GetParam())
      return kFloatMissing;  // parametria ei ole asetettu kohdalleen!!!!

    if (IsGrid())
    {
      NFmiLocationCache locCache = CalcLocationCache(theLatLonPoint);
      return CachedInterpolation(locCache);
    }
    else
    {
      Location(theLatLonPoint);  // asemadatalle haetaan lähimmän pisteen arvo
      return FloatValue();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::InterpolatedValueForCombinedParam(const NFmiPoint &theGridPoint)
{
  try
  {
    // jos esim. hessaa ei olekaan weatherandcloudinesissa vaan omana parametrinaa,
    // homma kaatuu tähän, tämä on vain hätä korjaus kaatumatautia vastaan itsCombinedParamParser
    // on nolla-pointteri, ja sitä ei auta kuitenkaan luoda, koska siihen ei saa kuitenkaan arvoja.
    if (!IsSubParamUsed())
      return FloatValue();  // kokeilen korjata tätä näin, jolloin palautetaan nearest arvo (tämä on
                            // hätä paskan hätä paska)

    auto xShift = static_cast<int>(round(theGridPoint.X()) - static_cast<int>(theGridPoint.X()));
    auto yShift = static_cast<int>(round(theGridPoint.Y()) - static_cast<int>(theGridPoint.Y()));

    PeekLocationValue(0 - xShift, 0 - yShift);
    NFmiWeatherAndCloudiness bottomLeftweather(
        *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
    PeekLocationValue(1 - xShift, 0 - yShift);
    NFmiWeatherAndCloudiness bottomRightweather(
        *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
    PeekLocationValue(0 - xShift, 1 - yShift);
    NFmiWeatherAndCloudiness topLeftweather(
        *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
    PeekLocationValue(1 - xShift, 1 - yShift);
    NFmiWeatherAndCloudiness topRightweather(
        *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));

    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    if ((dx != 0.) && (dx < 0.0001))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < 0.0001))
    {
      dy = 0;
    }

    if (dx > (1 - 0.0001))
    {
      dx = 1;
    }
    if (dy > (1 - 0.0001))
    {
      dy = 1;
    }

    if (itsCombinedParamParser)
      delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                              float((1. - dx) * (1. - dy)),
                                              &bottomRightweather,
                                              float(dx * (1. - dy)),
                                              &topLeftweather,
                                              float((1. - dx) * dy),
                                              &topRightweather,
                                              float(dx * dy));
    return float(
        itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForWeatherAndCloudiness(const NFmiPoint &theGridPoint,
                                                              int theXShift,
                                                              int theYShift)
{
  try
  {
    NFmiWeatherAndCloudiness bottomLeftweather(itsInfoVersion);
    bottomLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 0 - theYShift));

    NFmiWeatherAndCloudiness bottomRightweather(itsInfoVersion);
    bottomRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 0 - theYShift));

    NFmiWeatherAndCloudiness topLeftweather(itsInfoVersion);
    topLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 1 - theYShift));

    NFmiWeatherAndCloudiness topRightweather(itsInfoVersion);
    topRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 1 - theYShift));

    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    if ((dx != 0.) && (dx < 0.0001))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < 0.0001))
    {
      dy = 0;
    }

    if (dx > (1 - 0.0001))
    {
      dx = 1;
    }
    if (dy > (1 - 0.0001))
    {
      dy = 1;
    }

    if (itsCombinedParamParser)
      delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                              float((1. - dx) * (1. - dy)),
                                              &bottomRightweather,
                                              float(dx * (1. - dy)),
                                              &topLeftweather,
                                              float((1. - dx) * dy),
                                              &topRightweather,
                                              float(dx * dy));

    return itsCombinedParamParser->TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForWeatherAndCloudiness(const NFmiMetTime &theTime,
                                                              int theMaxMinuteRange,
                                                              const NFmiPoint &theGridPoint,
                                                              int theXShift,
                                                              int theYShift)
{
  try
  {
    unsigned int oldLocationIndex = LocationIndex();

    // tämä liikkuminen on melkoista viritystä
    MoveLeft(theXShift);
    MoveDown(theYShift);
    NFmiWeatherAndCloudiness bottomLeftweather(itsInfoVersion);
    bottomLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveRight(1);
    NFmiWeatherAndCloudiness bottomRightweather(itsInfoVersion);
    bottomRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveUp(1);
    NFmiWeatherAndCloudiness topRightweather(itsInfoVersion);
    topRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveLeft(1);
    NFmiWeatherAndCloudiness topLeftweather(itsInfoVersion);
    topLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));

    LocationIndex(oldLocationIndex);

    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    if ((dx != 0.) && (dx < 0.0001))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < 0.0001))
    {
      dy = 0;
    }

    if (dx > (1 - 0.0001))
    {
      dx = 1;
    }
    if (dy > (1 - 0.0001))
    {
      dy = 1;
    }

    if (itsCombinedParamParser)
      delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                              float((1. - dx) * (1. - dy)),
                                              &bottomRightweather,
                                              float(dx * (1. - dy)),
                                              &topLeftweather,
                                              float((1. - dx) * dy),
                                              &topRightweather,
                                              float(dx * dy));

    return itsCombinedParamParser->TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForTotalWind(const NFmiPoint &theGridPoint,
                                                   int theXShift,
                                                   int theYShift)
{
  try
  {
    NFmiTotalWind bottomLeftweather(itsInfoVersion);
    bottomLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 0 - theYShift));

    NFmiTotalWind bottomRightweather(itsInfoVersion);
    bottomRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 0 - theYShift));

    NFmiTotalWind topLeftweather(itsInfoVersion);
    topLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 1 - theYShift));

    NFmiTotalWind topRightweather(itsInfoVersion);
    topRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 1 - theYShift));

    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    if ((dx != 0.) && (dx < 0.0001))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < 0.0001))
    {
      dy = 0;
    }

    if (dx > (1 - 0.0001))
    {
      dx = 1;
    }
    if (dy > (1 - 0.0001))
    {
      dy = 1;
    }

    if (itsCombinedParamParser)
      delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                              float((1. - dx) * (1. - dy)),
                                              &bottomRightweather,
                                              float(dx * (1. - dy)),
                                              &topLeftweather,
                                              float((1. - dx) * dy),
                                              &topRightweather,
                                              float(dx * dy));
    return itsCombinedParamParser->TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForTotalWind(const NFmiMetTime &theTime,
                                                   int theMaxMinuteRange,
                                                   const NFmiPoint &theGridPoint,
                                                   int theXShift,
                                                   int theYShift)
{
  try
  {
    unsigned int oldLocationIndex = LocationIndex();
    // tämä liikkuminen on melkoista viritystä
    MoveLeft(theXShift);
    MoveDown(theYShift);
    NFmiTotalWind bottomLeftweather(itsInfoVersion);
    bottomLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveRight(1);
    NFmiTotalWind bottomRightweather(itsInfoVersion);
    bottomRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveUp(1);
    NFmiTotalWind topRightweather(itsInfoVersion);
    topRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
    MoveLeft(1);
    NFmiTotalWind topLeftweather(itsInfoVersion);
    topLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));

    LocationIndex(oldLocationIndex);

    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    if ((dx != 0.) && (dx < 0.0001))
    {
      dx = 0;
    }
    if ((dy != 0.) && (dy < 0.0001))
    {
      dy = 0;
    }

    if (dx > (1 - 0.0001))
    {
      dx = 1;
    }
    if (dy > (1 - 0.0001))
    {
      dy = 1;
    }

    if (itsCombinedParamParser)
      delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                              float((1. - dx) * (1. - dy)),
                                              &bottomRightweather,
                                              float(dx * (1. - dy)),
                                              &topLeftweather,
                                              float((1. - dx) * dy),
                                              &topRightweather,
                                              float(dx * dy));
    return itsCombinedParamParser->TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

#if 0
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \param topLeftValue Undocumented
 * \param topRightValue Undocumented
 * \param bottomLeftValue Undocumented
 * \param bottomRightValue Undocumented
 * \return Undocumented
 * \todo Refactor this into an external file
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::BiLinearInterpolation(double x,
										  double y,
										  float & theValue,
										  float topLeftValue,
										  float topRightValue,
										  float bottomLeftValue,
										  float bottomRightValue)
{
  try
  {
    double dx = x - int(x);
    double dy = y - int(y);

    if( (dx != 0.) && (dx < 0.0001)) { dx = 0;}
    if( (dy != 0.) && (dy < 0.0001)) { dy = 0;}

    if(dx > (1-0.0001)) { dx = 1; }
    if(dy > (1-0.0001)) { dy = 1; }

    if((topLeftValue != kFloatMissing) && (topRightValue != kFloatMissing)	&&
     (bottomLeftValue != kFloatMissing) && (bottomRightValue != kFloatMissing))
    {
      float interpolatedTopValue = float(dx*topRightValue + (1.f - dx)*topLeftValue);
      float interpolatedBottomValue = float(dx*bottomRightValue + (1.f - dx)*bottomLeftValue);

      theValue = float(dy*interpolatedTopValue + (1.f - dy)*interpolatedBottomValue);
      return true;
    }

    if(	(dy == 0.)	&& (bottomLeftValue != kFloatMissing)
      && (bottomRightValue != kFloatMissing))
    {
      theValue =  float(dx*bottomRightValue + (1.f - dx)*bottomLeftValue);
      return true;
    }
    if(	(dx == 0.)	&& (bottomLeftValue != kFloatMissing)
      && (topLeftValue != kFloatMissing))
    {
      theValue =  float(dy*topLeftValue + (1.f - dy)*bottomLeftValue);
      return true;
    }
    if(	(dy == 1.)	&& (topLeftValue != kFloatMissing)
      && (topRightValue != kFloatMissing))
    {
      theValue =  float(dx*topRightValue + (1.f - dx)*topLeftValue);
      return true;
    }
    if(	(dx == 1.)	&& (bottomRightValue != kFloatMissing)
      && (topRightValue != kFloatMissing))
    {
      theValue =  float(dy*topRightValue + (1.f - dy)*bottomRightValue);
      return true;
    }
    if(	(dx == 0.f && dy == 0.f)	&& (bottomLeftValue != kFloatMissing))
    {
      theValue =  bottomLeftValue;
      return true;
    }
    if(	(dx == 0.f && dy == 1.f)	&& (topLeftValue != kFloatMissing))
    {
      theValue =  topLeftValue;
      return true;
    }
    if(	(dx == 1.f && dy == 0.f)	&& (bottomRightValue != kFloatMissing))
    {
      theValue =  bottomRightValue;
      return true;
    }
    if(	(dx == 1.f && dy == 1.f)	&& (topRightValue != kFloatMissing))
    {
      theValue =  topRightValue;
      return true;
    }

    theValue = kFloatMissing;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

// ----------------------------------------------------------------------
/*!
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// lähinnä gridi-datan tutkimiseen, offsettien avulla voidaan pyytää currentista
// paikasta poikkevia paikkoja ei toimi kaikissa tilanteissa oikein, parempi versio
//  fastqueryinfossa

float NFmiQueryInfo::PeekLocationValue(int theXOffset, int theYOffset) const
{
  try
  {
    unsigned long paramIndex = itsParamDescriptor->Index();
    unsigned long timeIndex = itsTimeDescriptor->Index();
    unsigned long theVPlaceIndex = itsVPlaceDescriptor->Index();
    unsigned long theHPlaceIndex = itsHPlaceDescriptor->Index();

    if (IsGrid())
      theHPlaceIndex = theHPlaceIndex + theYOffset * Grid()->XNumber() + theXOffset;
    else
      theHPlaceIndex = theHPlaceIndex + theXOffset;

    unsigned long idx =
        ((paramIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
             (itsTimeDescriptor->Size()) +
         (theHPlaceIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
         theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
    return PeekValue(idx);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// HUOM!!!! En toteuta tätä ainakaan vielä qinfoon, vaan vain fastinfoon.
float NFmiQueryInfo::PeekLocationValue(int /* theXOffset */,
                                       int /* theYOffset*/,
                                       const NFmiMetTime & /* theTime */)
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::NextParam(bool fIgnoreSubParam)
{
  try
  {
    bool status = itsParamDescriptor->Next(fIgnoreSubParam);
    bool useSubParam = itsParamDescriptor->IsSubParamUsed();
    if (useSubParam)
    {
      NFmiDataIdent &param =
          itsParamDescriptor->Param(true);  // huom! tässä pitää pyytää yliparametria (->true)
      ChangeCombinedParamParser(param);
    }
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::PreviousParam(bool fIgnoreSubParam)
{
  try
  {
    bool status = itsParamDescriptor->Previous(fIgnoreSubParam);
    bool useSubParam = itsParamDescriptor->IsSubParamUsed();
    if (useSubParam)
    {
      NFmiDataIdent &param = itsParamDescriptor->Param();
      ChangeCombinedParamParser(param);
    }
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiDataIdent &theDataIdent)
{
  try
  {
    if (itsParamDescriptor->Param(theDataIdent))
      return true;
    else if (FindSubParam(theDataIdent))  // katsotaan löytyykö parametri jonkin yhdistelmä
                                          // parametrin aliparametrina
    {
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
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiParam &theParam)
{
  try
  {
    if (itsParamDescriptor->Param(theParam))
      return true;
    else if (FindSubParam(theParam))  // katsotaan löytyykö parametri jonkin yhdistelmä parametrin
                                      // aliparametrina
    {
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
 * \param theParam Undocumented
 * \param theSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiParam &theParam, const NFmiParam &theSubParam)
{
  try
  {
    // Tätä metodia tarvitaan silloin, kun on mahdollista, että jokin parametri löytyy
    // paitsi varsinaisena parametrina myös jonkin toisen parametrin aliparametrina tai
    // kenties jokin parametri löytyy kahden eri parametrin sisältä aliparametrinä.
    //
    // Esim.
    // Tunturisääpalvelussa tuotetaan NFmiQueryData-olio, jossa on sekä kFmiWeatherAndCloudiness
    // että kFmiTopWeatherAndCloudiness	paramerit. Näiden avulla luotujen NFmiDataIdent-olioiden
    // sisään on luettu useita saman nimisiä parametreja. Jos käyttäjä haluaa esim. lukea tunturin
    // huipun kFmiFogIntensity:n, asetetaan parametri tällä metodilla
    //
    //		NFmiQueryInfo::Param(kFmiTopWeatherAndCloudiness, kFmiFogIntensity)
    //
    // Mikäli kFmiFogIntensity yritetään asettaa suoraan kutsumalla Param(kFmiFogIntensity),
    // asettuu itsParamDescriptor osoittamaan esimmäisenä (aliparametrina )löydettyä itemiä,
    // jonka nimi on kFmiFogIntensity.

    if (!itsParamDescriptor->Param(theParam))
      return false;

    else if (FindSubParam(theSubParam))  // katsotaan löytyykö parametri jonkin yhdistelmä
                                         // parametrin aliparametrina
    {
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
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// katsoo, löytyykö minkään parametrin aliparametreistä annettua parametriä. Jos löytyy
// palauttaa true ja asettaa itsSubParam:in arvoksi kyseisen parametrin

bool NFmiQueryInfo::FindSubParam(const NFmiParam &theParam)
{
  try
  {
    if (itsParamDescriptor->FindSubParam(theParam))
    {
      NFmiDataIdent &param = itsParamDescriptor->Param();
      ChangeCombinedParamParser(param);
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
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// katsoo, löytyykö minkään parametrin aliparametreistä annettua parametriä. Jos löytyy
// palauttaa true ja asettaa itsSubParam:in arvoksi kyseisen parametrin

bool NFmiQueryInfo::FindSubParam(const NFmiDataIdent &theDataIdent)
{
  try
  {
    if (itsParamDescriptor->FindSubParam(theDataIdent))
    {
      NFmiDataIdent &param = itsParamDescriptor->Param();
      ChangeCombinedParamParser(param);
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
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// luo annetun parametrin (yhdistelmä parametrin!) mukaisen parserin

bool NFmiQueryInfo::ChangeCombinedParamParser(const NFmiDataIdent &theParam)
{
  try
  {
    if (itsCombinedParamParser)
    {
      delete itsCombinedParamParser;
      itsCombinedParamParser = nullptr;
    }
    auto paramName = FmiParameterName(theParam.GetParam()->GetIdent());
    switch (paramName)
    {
      case kFmiTotalWindMS:
        itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);  // Marko testi
        return true;
      case kFmiWeatherAndCloudiness:
        itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);  // Marko testi
        return true;
      default:
        break;
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiProducer *NFmiQueryInfo::Producer()
{
  try
  {
    return (itsParamDescriptor ? itsParamDescriptor->ParamBag()->Current()->GetProducer()
                               : nullptr);
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

NFmiDataIdent &NFmiQueryInfo::Param() const
{
  try
  {
    return itsParamDescriptor->Param(false);
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

float NFmiQueryInfo::SubParamFloatValue() const
{
  try
  {
    return SubValueFromFloat(IndexFloatValue(Index()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::SubValueFromFloat(float fValue) const
{
  try
  {
    if (itsCombinedParamParser)
    {
      itsCombinedParamParser->TransformFromFloatValue(fValue);
      return float(itsCombinedParamParser->SubValue(
          FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent())));
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFloatData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SubParamFloatValue(float theFloatData)
{
  try
  {
    if (itsCombinedParamParser)
    {
      size_t index = Index();
      float fValue = IndexFloatValue(index);
      itsCombinedParamParser->TransformFromFloatValue(fValue);
      itsCombinedParamParser->SubValue(
          theFloatData, FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent()));
      return IndexFloatValue(index, itsCombinedParamParser->TransformedFloatValue());
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

float NFmiQueryInfo::SubParamFloatValue(size_t theIndex) const
{
  try
  {
    if (itsCombinedParamParser)
    {
      float fValue = IndexFloatValue(theIndex);
      itsCombinedParamParser->TransformFromFloatValue(fValue);
      return float(itsCombinedParamParser->SubValue(
          FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent())));
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param period Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodFloatValue(unsigned long period,
                                          float startWeight,
                                          float centreWeight,
                                          float endWeight)
{
  try
  {
    if (UseSubParam())
    {
      if (itsCombinedParamParser)
        delete itsCombinedParamParser;
      // case: onko tuuli
      itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);

      itsCombinedParamParser->SetToWeightedPeriod(
          this, NFmiPoint(), period, true, startWeight, centreWeight, endWeight);
      return static_cast<float>(
          itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
    }
    return TimePeriodValue(NFmiPoint(), period, startWeight, centreWeight, endWeight);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodFloatValue(const NFmiMetTime &theStartTime,
                                          const NFmiMetTime &theEndTime,
                                          float startWeight,
                                          float centreWeight,
                                          float endWeight)
{
  try
  {
    if (UseSubParam())
    {
      if (itsCombinedParamParser)
        delete itsCombinedParamParser;
      // case: onko tuuli
      itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
      itsCombinedParamParser->SetToWeightedPeriod(
          this, NFmiPoint(), theStartTime, theEndTime, true, startWeight, centreWeight, endWeight);
      return static_cast<float>(
          itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
    }

    return TimePeriodValue(
        NFmiPoint(), theStartTime, theEndTime, startWeight, centreWeight, endWeight);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param latlon Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedTimePeriodFloatValue(const NFmiPoint &latlon,
                                                      const NFmiMetTime &theStartTime,
                                                      const NFmiMetTime &theEndTime,
                                                      float startWeight,
                                                      float centreWeight,
                                                      float endWeight)
{
  try
  {
    if (UseSubParam())
    {
      if (itsCombinedParamParser)
        delete itsCombinedParamParser;

      itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
      itsCombinedParamParser->SetToWeightedPeriod(this,
                                                  latlon,
                                                  theStartTime,
                                                  theEndTime,
                                                  true,
                                                  startWeight,
                                                  centreWeight,
                                                  endWeight);  // 10.2.00
      return static_cast<float>(
          itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
    }

    return TimePeriodValue(latlon, theStartTime, theEndTime, startWeight, centreWeight, endWeight);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLat Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodValue(const NFmiPoint &theLonLat,
                                     const NFmiMetTime &theStartTime,
                                     const NFmiMetTime &theEndTime,
                                     float factor1,
                                     float factor2,
                                     float factor3)
{
  try
  {
    bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

    std::unique_ptr<NFmiQueryInfo> newInfo(this->Clone());
    NFmiMetTime middleTime(theStartTime);
    double halfPeriod = theEndTime.DifferenceInHours(theStartTime) / 2.;
    middleTime.ChangeByHours(long(halfPeriod));

    double sum = 0.;
    double factorSum = 0;
    double factor;
    float value;

    if (!newInfo->TimeToNearestStep(theStartTime))
      return kFloatMissing;

    while (newInfo->Time() <= theEndTime)
    {
      if (locationMissing)  // ei haluta interpoloida
        value = newInfo->FloatValue();
      else
        value = newInfo->InterpolatedValue(theLonLat);

      if (value != kFloatMissing)
      {
        if (newInfo->Time() < Time())
        {
          factor = (static_cast<float>(newInfo->Time().DifferenceInHours(theStartTime))) /
                       halfPeriod * (factor2 - factor1) +
                   factor1;
        }
        else
        {
          factor = (static_cast<float>(newInfo->Time().DifferenceInHours(middleTime))) /
                       halfPeriod * (factor3 - factor2) +
                   factor2;
        }

        if (value != kFloatMissing)
        {
          sum += value * factor;
          factorSum += factor;
        }
      }
      if (!newInfo->NextTime())
        break;
    }

    if (factorSum > 0.)
      return static_cast<float>(sum / factorSum);

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLat Undocumented
 * \param period Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// 10.2.2000/Lasse: JOLLEI KUKAAN KÄYTÄ TÄMÄN VERSION VOI POISTAA

float NFmiQueryInfo::TimePeriodValue(
    const NFmiPoint &theLonLat, unsigned long period, float factor1, float factor2, float factor3)
{
  try
  {
    bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

    NFmiQueryInfo newInfo(*this);
    NFmiMetTime startTime(Time());
    NFmiMetTime middleTime(startTime);
    NFmiMetTime endTime(startTime);
    long halfPeriod = period / 2;
    startTime.ChangeByHours(-halfPeriod);
    endTime.ChangeByHours(halfPeriod);

    double sum = 0.;
    double factorSum = 0;
    float factor;
    float value;

    if (!newInfo.TimeToNearestStep(startTime))
      return kFloatMissing;

    while (newInfo.Time() <= endTime)
    {
      if (locationMissing)  // ei haluta interpoloida
        value = newInfo.FloatValue();
      else
        value = newInfo.InterpolatedValue(theLonLat);

      if (value != kFloatMissing)
      {
        if (newInfo.Time() < Time())
        {
          factor = (static_cast<float>(newInfo.Time().DifferenceInHours(startTime))) / halfPeriod *
                       (factor2 - factor1) +
                   factor1;
        }
        else
        {
          factor = (static_cast<float>(newInfo.Time().DifferenceInHours(middleTime))) / halfPeriod *
                       (factor3 - factor2) +
                   factor2;
        }

        if (value != kFloatMissing)
        {
          sum += value * factor;
          factorSum += factor;
        }
      }
      newInfo.NextTime();
    }

    if (factorSum > 0.)
      return static_cast<float>(sum / factorSum);

    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theArea Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData(NFmiDataModifier *theModifier,
                                     NFmiArea *theArea,
                                     NFmiLocationBag *theLocationBag)
{
  try
  {
    if (theArea)
      CalcLocationData2(theModifier, theArea);
    else if (theLocationBag)
      CalcLocationData2(theModifier, theLocationBag);
    else
      CalcLocationData2(theModifier);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLevelData(NFmiDataModifier *theModifier)
{
  try
  {
    for (ResetLevel(); NextLevel();)
      theModifier->Calculate(FloatValue());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag)
{
  try
  {
    if (theTimeBag)
    {
      if (static_cast<unsigned long>(theTimeBag->Resolution()) == TimeResolution())
      {
        theTimeBag->First();
        do
        {
          if (Time(theTimeBag->CurrentTime()))
            break;
        } while (theTimeBag->Next());
        do
        {
          theModifier->Calculate(FloatValue());
        } while (theTimeBag->Next() && NextTime());
      }
      else
      {
        for (theTimeBag->Reset(); theTimeBag->Next();)
        {
          if (Time(theTimeBag->CurrentTime()))
            theModifier->Calculate(FloatValue());
        }
      }
    }
    else
    {
      for (ResetTime(); NextTime();)
        theModifier->Calculate(FloatValue());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeDesc Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeDescriptor &theTimeDesc)
{
  try
  {
    if (theTimeDesc.ValidTimeBag())
      CalcTimeData(theModifier,
                   theTimeDesc.ValidTimeBag());  // jos on timebagi, käyttää valmista funktiota
    else
    {
      // käydään timeDesc:issä oleva timelist läpi
      for (theTimeDesc.Reset(); theTimeDesc.Next();)
        if (Time(theTimeDesc.Time()))
          theModifier->Calculate(FloatValue());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier,
                                 const NFmiMetTime &theStartTime,
                                 const NFmiMetTime &theEndTime)
{
  try
  {
    // 2.5.02/Lasse: TOIMII SEURAAVASTI: ks CalcInterpolatedTimeData alla

    NFmiMetTime saveTime(Time());
    if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
    {
      theModifier->Calculate(FloatValue());
      while (NextTime() && Time() <= theEndTime)
        theModifier->Calculate(FloatValue());

      if (SizeTimes() > 1)
      {  // resoluutio voi olla eri alussa ja lopussa
        itsTimeDescriptor->FirstTime();
        long resStart = TimeResolution();
        itsTimeDescriptor->LastTime();
        long resEnd = TimeResolution();
        if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
            theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
        {
          theModifier->Calculate(kFloatMissing);
        }
      }
    }
    Time(saveTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeDataWithExtremeTime(NFmiDataModifierExtreme *theModifier,
                                                const NFmiMetTime &theStartTime,
                                                const NFmiMetTime &theEndTime)
{
  try
  {
    // 2.5.02/Lasse: TOIMII SEURAAVASTI: ks CalcInterpolatedTimeData alla

    NFmiMetTime saveTime(Time());
    if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
    {
      theModifier->Calculate(FloatValue(), this);
      while (NextTime() && Time() <= theEndTime)
        theModifier->Calculate(FloatValue(), this);

      if (SizeTimes() > 1)
      {  // resoluutio voi olla eri alussa ja lopussa
        itsTimeDescriptor->FirstTime();
        long resStart = TimeResolution();
        itsTimeDescriptor->LastTime();
        long resEnd = TimeResolution();
        if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
            theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
        {
          theModifier->Calculate(kFloatMissing);
        }
      }
    }
    Time(saveTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeDesc Undocumented
 * \param lonLat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                             NFmiTimeDescriptor &theTimeDesc,
                                             const NFmiPoint &lonLat)
{
  try
  {
    // HUOM oma aika säilyy
    NFmiMetTime saveTime(Time());
    for (theTimeDesc.Reset(); theTimeDesc.Next();)
    {
      if (Time(theTimeDesc.Time()))
        theModifier->Calculate(InterpolatedValue(lonLat));
    }
    Time(saveTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param lonLat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                             const NFmiMetTime &theStartTime,
                                             const NFmiMetTime &theEndTime,
                                             const NFmiPoint &lonLat)

// TOIMII SEURAAVASTI:
// .........D...D...D...D...D...D.......... datan aika-askeleet (res. tässä = 4h)
//               >         <                par:na tulevat alku/loppuaika;tulos= EI puuttuvia
//       >                       <          EI puuttuvia koska res. mukaan ei tarvita vielä tässä
//       alkupisteessä
//      >                        <          ON puuttuvia koska res. mukaan pitäisi olla dataa alussa
//                       >               <  ON puuttuvia
//           > <                            lopputulos puuttuva
//    >   <                                 lopputulos puuttuva
//                               >      <   lopputulos puuttuva
// jos yksikin data löytynyt ja ON puuttuvia on lopputulos kiinni modifierista eli sallitaanko niitä
// jos datassa vain yksi aika on tulos hyväksytty jos se vain sisältyy jaksoon (resoluutio tavallaan
// ääretön)

{
  try
  {
    NFmiMetTime saveTime(Time());

    if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
    {
      theModifier->Calculate(InterpolatedValue(lonLat));
      while (NextTime() && Time() <= theEndTime)
        theModifier->Calculate(InterpolatedValue(lonLat));

      if (SizeTimes() > 1)
      {  // resoluutio voi olla eri alussa ja lopussa
        itsTimeDescriptor->FirstTime();
        long resStart = TimeResolution();
        itsTimeDescriptor->LastTime();
        long resEnd = TimeResolution();
        if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
            theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
        {
          theModifier->Calculate(kFloatMissing);
        }
      }
    }

    Time(saveTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyTimesLocationData(NFmiDataModifier *theModifier,
                                            NFmiQueryInfo *theQueryInfoCopy,
                                            NFmiTimeBag *theTimeBag)
{
  try
  {
    if (!theTimeBag)
    {
      if (theQueryInfoCopy)
        theQueryInfoCopy->ResetTime();
      for (ResetTime(); NextTime();)
      {
        if (theQueryInfoCopy)
          theQueryInfoCopy->NextTime();
        ModifyLocationData(theModifier, theQueryInfoCopy);
      }
    }
    else
    {
      for (theTimeBag->Reset(); theTimeBag->Next();)
      {
        if (!Time(theTimeBag->CurrentTime()))
          continue;
        if (theQueryInfoCopy)
          if (!theQueryInfoCopy->Time(theTimeBag->CurrentTime()))
            continue;
        ModifyLocationData(theModifier, theQueryInfoCopy);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::ModifyTimesLocationData(NFmiDataModifier *theModifier,
                                            NFmiQueryInfo *theQueryInfoCopy,
                                            NFmiTimeDescriptor &theTimeDescriptor)
{
  try
  {
    for (theTimeDescriptor.Reset(); theTimeDescriptor.Next();)
    {
      if (!Time(theTimeDescriptor.Time()))
        continue;
      if (theQueryInfoCopy)
        if (!theQueryInfoCopy->Time(theTimeDescriptor.Time()))
          continue;
      ModifyLocationData(theModifier, theQueryInfoCopy);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

class TimeToModifyCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  TimeToModifyCalculator(const NFmiTimeDescriptor &theTimeDescriptor)
      : itsTimeDescriptor(theTimeDescriptor), fNoMoreWork(false)
  {
    try
    {
      itsTimeDescriptor.First();
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  bool GetCurrentTime(NFmiMetTime &theTime)
  {
    try
    {
      WriteLock lock(itsMutex);
      if (fNoMoreWork)
        return false;
      theTime = itsTimeDescriptor.Time();
      if (itsTimeDescriptor.Next() == false)
        fNoMoreWork = true;
      return true;
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

 private:
  NFmiTimeDescriptor itsTimeDescriptor;
  MutexType itsMutex;
  bool fNoMoreWork;

  TimeToModifyCalculator(const TimeToModifyCalculator &) =
      delete;  // ei toteuteta kopio konstruktoria
};

static void ModifySingleTimeGridInThread(NFmiFastQueryInfo &theModifiedInfo,
                                         TimeToModifyCalculator &theTimeToModifyCalculator,
                                         NFmiDataModifier *theModifier)
{
  try
  {
    NFmiMetTime aTime;
    for (; theTimeToModifyCalculator.GetCurrentTime(aTime);)
    {
      if (theModifiedInfo.Time(aTime))
      {
        theModifier->SetTimeIndex(theModifiedInfo.TimeIndex());
        for (theModifiedInfo.ResetLocation(); theModifiedInfo.NextLocation();)
        {
          theModifier->SetLocationIndex(theModifiedInfo.LocationIndex());
          theModifiedInfo.FloatValue(theModifier->FloatOperation(theModifiedInfo.FloatValue()));
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::ModifyTimesLocationData_FullMT(NFmiDataModifier *theModifier,
                                                   NFmiTimeDescriptor &theTimeDescriptor)
{
  try
  {
    unsigned int usedThreadCount = boost::thread::hardware_concurrency();
    unsigned long timeCount = theTimeDescriptor.Size();
    if (usedThreadCount > timeCount)
      usedThreadCount = timeCount;

    LatLon();  // multi-thread koodin varmistus, että latlon-cachet on alustettu
    theModifier->InitLatlonCache();
    std::vector<std::shared_ptr<NFmiFastQueryInfo> > modifiedInfoVector(usedThreadCount);
    std::vector<std::shared_ptr<NFmiDataModifier> > dataModifierVector(usedThreadCount);
    auto *thisFastInfo = dynamic_cast<NFmiFastQueryInfo *>(this);
    if (thisFastInfo == nullptr)
      return;  // ei voi edetä, koska this info ei ollutkaan oikeasti fastInfo
    for (unsigned int i = 0; i < usedThreadCount; i++)
    {
      // HUOM! pakko tehdä fastQueryInfo copioita, että ei menetetä nopeutta, eikä saa tehdä
      // Clone:ja, koska tällöin kopioituu myös data-osio!!!
      modifiedInfoVector[i] =
          std::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*thisFastInfo));
      dataModifierVector[i] = std::shared_ptr<NFmiDataModifier>(theModifier->Clone());
    }

    TimeToModifyCalculator timeIndexCalculator(theTimeDescriptor);
    boost::thread_group calcParts;
    for (unsigned int i = 0; i < usedThreadCount; i++)
      calcParts.add_thread(new boost::thread(::ModifySingleTimeGridInThread,
                                             boost::ref(*modifiedInfoVector[i]),
                                             boost::ref(timeIndexCalculator),
                                             dataModifierVector[i].get()));
    calcParts.join_all();  // odotetaan että threadit lopettavat
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theArea Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData(NFmiDataModifier *theModifier,
                                       NFmiQueryInfo *theQueryInfoCopy,
                                       NFmiArea *theArea,
                                       NFmiLocationBag *theLocationBag)
{
  try
  {
    if (theQueryInfoCopy)
      theQueryInfoCopy->ResetLocation();

    if (theArea)
      ModifyLocationData2(theModifier, theQueryInfoCopy, theArea);
    else if (theLocationBag)
      ModifyLocationData2(theModifier, theQueryInfoCopy, theLocationBag);
    else
      ModifyLocationData2(theModifier, theQueryInfoCopy);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag)
{
  try
  {
    if (theTimeBag)
    {
      for (theTimeBag->Reset(); theTimeBag->Next();)
        if (Time(theTimeBag->CurrentTime()))
          FloatValue(theModifier->FloatOperation(FloatValue()));
    }
    else
    {
      for (ResetTime(); NextTime();)
        FloatValue(theModifier->FloatOperation(FloatValue()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLevelData(NFmiDataModifier *theModifier)
{
  try
  {
    for (ResetLevel(); NextLevel();)
      FloatValue(theModifier->FloatOperation(FloatValue()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::PeekTimeValue(int theTimeOffset)
{
  try
  {
    float returnValue = kFloatMissing;
    NFmiMetTime copyTime = ValidTime(), copyTime2 = ValidTime();

    copyTime2.ChangeByMinutes(theTimeOffset * TimeResolution());
    if (Time(copyTime2))
      returnValue = FloatValue();
    Time(copyTime);

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQueryInfo Undocumented
 * \param fIgnoreLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel)
{
  try
  {
    bool returnValue = true;
    if (theQueryInfo->IsParamUsable())  // Tällainen testaus, siksi, että ei ohjelma
    {                                   // ei kaadu, jos ollaan resetissä.
      if (!Param(NFmiParam(theQueryInfo->Param().GetParam()->GetIdent())))
      {
        returnValue = false;
      }
    }
    else
      returnValue = false;

    // Mika 08.11.2001 : Paluuarvoa ei kayteta - onko sivuvaikutus??
    // bool boo = theQueryInfo->IsLevel();
    theQueryInfo->IsLevel();
    if (!fIgnoreLevel && theQueryInfo->Level())
    {
      if (!Level(*(theQueryInfo->Level())))
        returnValue = false;
    }
    else
      returnValue = false;

    // Mika 08.11.2001 - paluuarvoa ei kayteta, onko sivuvaikutus?
    // const NFmiLocation *loac = theQueryInfo->Location();
    // bool poo = IsGrid();
    theQueryInfo->Location();
    IsGrid();

    if (theQueryInfo->IsLocationUsable())  // Tällainen testaus, siksi, että ei ohjelma
    {
      if (IsGrid())
      {
        if (!Location(theQueryInfo->LatLon()))
          returnValue = false;
      }
      else if (theQueryInfo->Location())
      {
        if (!Location(*(theQueryInfo->Location())))
          returnValue = false;
      }
      else
        returnValue = false;
    }
    else
      returnValue = false;

    if (theQueryInfo->IsTimeUsable())  // Tällainen testaus, siksi, että ei ohjelma
      if (!Time(theQueryInfo->Time()))
        returnValue = false;

    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier)
{
  try
  {
    for (ResetLocation(); NextLocation();)
      //	if(IsLocationMasked(LocationIndex()))
      theModifier->Calculate(FloatValue());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationDataWithExtremePlace(NFmiDataModifierExtremePlace *theModifier)
{
  try
  {
    for (ResetLocation(); NextLocation();)
      //	if(IsLocationMasked(LocationIndex()))
      theModifier->Calculate(FloatValue(), this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier, NFmiArea *theArea)
{
  try
  {
    if (theArea)
    {
      for (ResetLocation(); NextLocation();)
      {
        //		  if (theArea->IsInside(LatLon()) && IsLocationMasked(LocationIndex()))
        if (theArea->IsInside(LatLon()))
          theModifier->Calculate(FloatValue());
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
 * \param theModifier Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier,
                                      NFmiLocationBag *theLocationBag)
{
  try
  {
    if (theLocationBag)
    {
      for (theLocationBag->Reset(); theLocationBag->Next();)
      {
        if (Location(*theLocationBag->Location()))
          theModifier->Calculate(FloatValue());
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
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy)
{
  try
  {
    if (theQueryInfoCopy)
      theQueryInfoCopy->ResetLocation();

    for (ResetLocation(); NextLocation();)
    {
      if (theQueryInfoCopy)
        theQueryInfoCopy->LocationIndex(LocationIndex());
      FloatValue(theModifier->FloatOperation(FloatValue()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy,
                                        NFmiArea *theArea)
{
  try
  {
    if (theQueryInfoCopy)
      theQueryInfoCopy->ResetLocation();

    if (theArea)
    {
      for (ResetLocation(); NextLocation();)
      {
        if (theArea->IsInside(LatLon()))
        {
          FloatValue(theModifier->FloatOperation(FloatValue()));
        }
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
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy,
                                        NFmiLocationBag *theLocationBag)
{
  try
  {
    if (theQueryInfoCopy)
      theQueryInfoCopy->ResetLocation();

    if (theLocationBag)
    {
      for (theLocationBag->Reset(); theLocationBag->Next();)
      {
        if (Location(*theLocationBag->Location()))
        {
          if (theQueryInfoCopy)
            theQueryInfoCopy->Location(*theLocationBag->Location());
          FloatValue(theModifier->FloatOperation(FloatValue()));
        }
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
 * \param theTime Undocumented
 * \param theMaxMinuteRange Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValue(const NFmiMetTime &theTime, int theMaxMinuteRange)
{
  try
  {
    float tmpValue = kFloatMissing;
    unsigned long oldTimeIndex = TimeIndex();

    // Return value stored for the time if it is not missing
    if (Time(theTime))
      tmpValue = FloatValue();

    if (tmpValue == kFloatMissing)
    {
      if (itsTimeDescriptor->ValidTimeBag())
        tmpValue = InterpolatedValueFromTimeBag(theTime, theMaxMinuteRange);
      else if (itsTimeDescriptor->ValidTimeList())
        tmpValue = InterpolatedValueFromTimeList(theTime, theMaxMinuteRange);
    }

    TimeIndex(oldTimeIndex);  // asetetaan lopuksi indeksi takaisin siihen, missä se oli
    return tmpValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::InterpolatedValueFromTimeBag(const NFmiMetTime &theTime, int theMaxMinuteRange)
{
  try
  {
    NFmiTimeBag *timeBag = itsTimeDescriptor->ValidTimeBag();
    if (timeBag && timeBag->IsInside(theTime))
    {
      int sizeTimes = SizeTimes();
      int timeIndex = TimeIndex();
      if (timeIndex == -1 || sizeTimes < timeIndex)
      {
        // eli jos aika osoittaa kuuseen, laitetaan aika osoittamaan
        // 1. aikaan ja otetaan indeksi uudestaan
        FirstTime();
        timeIndex = TimeIndex();
      }
      float returnValue = kFloatMissing;
      NFmiMetTime firstTime(timeBag->FirstTime());
      int timeResolution = timeBag->Resolution();
      // timeResolution voi olla 0, jos timeBagissa on vain yksi aika, tai jos alku ja loppu aika
      // ovat samoja. Pitää estää 0:lla jako
      if (timeResolution == 0)
        return kFloatMissing;
      int minuteOffSetOfWantedTimeInTimeBag =
          theTime.DifferenceInMinutes(firstTime) - (timeResolution * timeIndex);
      float wantedTimeOffsetFloat = float(minuteOffSetOfWantedTimeInTimeBag) / timeResolution;
      int wantedTimeOffset = minuteOffSetOfWantedTimeInTimeBag / timeResolution;
      auto backwardOffset = static_cast<int>(floor(wantedTimeOffsetFloat));
      auto forwardOffset = static_cast<int>(ceil(wantedTimeOffsetFloat));
      if (backwardOffset == forwardOffset)
        forwardOffset++;
      if (minuteOffSetOfWantedTimeInTimeBag % timeResolution ==
          0)  // jos haluttu aika menee tasa aikasteppiin
      {
        backwardOffset = wantedTimeOffset - 1;
        forwardOffset = wantedTimeOffset + 1;
        returnValue = PeekTimeValue(wantedTimeOffset);
      }
      if (returnValue == kFloatMissing || returnValue == kTCombinedWeatherFloatMissing)
      {
        // aletaan etsiä lähimpiä arvoja molemmista suunnista
        int i;
        float value1 = kFloatMissing, value2 = kFloatMissing;
        for (i = backwardOffset; i + timeIndex >= 0; i--)
        {
          backwardOffset = i;
          value1 = PeekTimeValue(i);
          if (!(value1 == kFloatMissing || value1 == kTCombinedWeatherFloatMissing))
          {
            if (theMaxMinuteRange &&
                theMaxMinuteRange < (wantedTimeOffsetFloat - i) * timeResolution)
              value1 = kFloatMissing;
            break;
          }
          else
            value1 = kFloatMissing;  // tarkastus helpottuu myöhemmin, kun vain yksi missing arvo
        }
        for (i = forwardOffset; i + timeIndex < int(sizeTimes); i++)
        {
          forwardOffset = i;
          value2 = PeekTimeValue(i);
          if (!(value2 == kFloatMissing || value2 == kTCombinedWeatherFloatMissing))
          {
            if (theMaxMinuteRange &&
                theMaxMinuteRange < (i - wantedTimeOffsetFloat) * timeResolution)
              value2 = kFloatMissing;
            break;
          }
          else
            value2 = kFloatMissing;  // tarkastus helpottuu myöhemmin, kun vain yksi missing arvo
        }
        if (value1 == kFloatMissing || value2 == kFloatMissing)
          returnValue = kFloatMissing;
        else
        {
          float offset1 =
              (forwardOffset - wantedTimeOffsetFloat) / (forwardOffset - backwardOffset);

          // 30.10.2000/Marko Muutin ottamaan huomioon combinedparam jutut ja epälineaariset
          // parametrit
          NFmiParam *parameter = Param().GetParam();
          FmiInterpolationMethod interp = parameter->InterpolationMethod();
          unsigned long param = parameter->GetIdent();
          if (interp == kByCombinedParam ||
              param == kFmiTotalWindMS)  // HUOM!!!! totalwindillä ei ole tätä arvoa (korjaa!)
          {
            switch (param)
            {
              case kFmiTotalWindMS:
              {  // korjasin käyttämään SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien
                // avulla
                NFmiTotalWind resultWind(itsInfoVersion);
                NFmiTotalWind tempWind1(value1, kFmiPackedWind, itsInfoVersion);
                NFmiTotalWind tempWind2(value2, kFmiPackedWind, itsInfoVersion);
                resultWind.SetToWeightedMean(
                    &tempWind1, offset1, &tempWind2, 1 - offset1, &tempWind1, 0, &tempWind1, 0);
                returnValue = resultWind.TransformedFloatValue();
              }
              break;
              case kFmiWeatherAndCloudiness:
              {
                NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
                NFmiWeatherAndCloudiness tempWeather1(
                    value1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
                NFmiWeatherAndCloudiness tempWeather2(
                    value2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

                resultWeather.SetToWeightedMean(&tempWeather1,
                                                offset1,
                                                &tempWeather2,
                                                1 - offset1,
                                                &tempWeather1,
                                                0,
                                                &tempWeather1,
                                                0);
                returnValue = resultWeather.TransformedFloatValue();
              }
              break;
            }
          }
          else if (interp == kNearestNonMissing)
          {
            if (offset1 > 0.5)
            {
              returnValue = value1;
              if (returnValue == kFloatMissing)
                returnValue = value2;
            }
            else
            {
              returnValue = value2;
              if (returnValue == kFloatMissing)
                returnValue = value1;
            }
          }
          else if (interp != kLinearly)
          {
            if (offset1 > 0.5)
              returnValue = value1;
            else
              returnValue = value2;
          }
          else  // muuten lineaarinen interpolointi
          {
            if (param == kFmiWindDirection)
            {  // tehdään wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
              Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
              float ws1 = PeekTimeValue(backwardOffset);
              float ws2 = PeekTimeValue(forwardOffset);
              Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
              NFmiInterpolation::WindInterpolator windInterpolator;
              windInterpolator.operator()(ws1, value1, offset1);
              windInterpolator.operator()(ws2, value2, (1 - offset1));
              returnValue = static_cast<float>(windInterpolator.Direction());  // meitä kiinnostaa
                                                                               // vain tuulen suunta
              // (tämä pitää tehdä
              // jotenkin fiksummin)
            }
            else if (param == kFmiWaveDirection)
            {
              returnValue =
                  static_cast<float>(NFmiInterpolation::ModLinear(offset1, value1, value2, 360));
            }
            else if (param == kFmiWindVectorMS)
            {
              returnValue =
                  static_cast<float>(NFmiInterpolation::WindVector(offset1, value1, value2));
            }
            else
              returnValue = float(offset1 * value1 + (1.f - offset1) * value2);
          }
        }
      }
      return returnValue;
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::FindNearestNonMissingValueFromTimeList(const NFmiMetTime &theTime,
                                                            int theMaxMinuteRange,
                                                            int &theTimeIndexInOut,
                                                            NFmiMetTime &theFoundTime,
                                                            FmiDirection theDirection)
{
  try
  {
    // etsitään lähin aika takaa päin, mistä löytyy ei puuttuva arvo
    bool backWard = (theDirection == kBackward);
    float value = kFloatMissing;
    NFmiTimeList *timeList = itsTimeDescriptor->ValidTimeList();
    if (timeList == nullptr)
      throw Fmi::Exception(
          BCP,
          "Error: Programming error in NFmiQueryInfo::FindNearestNonMissingValueFromTimeList, no "
          "timelist found.");
    do
    {
      value = PeekValue(Index(ParamIndex(), LocationIndex(), LevelIndex(), theTimeIndexInOut));
      if (value != kFloatMissing)
      {
        if (timeList->Time(theTimeIndexInOut) == nullptr)
          throw Fmi::Exception(
              BCP,
              "Error: Programming error in NFmiQueryInfo::FindNearestNonMissingValueFromTimeList, "
              "time not found.");
        theFoundTime = *timeList->Time(theTimeIndexInOut);
        if (theMaxMinuteRange != 0 &&
            ::abs(theTime.DifferenceInMinutes(theFoundTime)) > theMaxMinuteRange)
          value = kFloatMissing;  // aika josta löytyi ei puuttuva arvo, löytyi liian kaukaa
        break;
      }
      backWard ? theTimeIndexInOut-- : theTimeIndexInOut++;
    } while (backWard ? theTimeIndexInOut >= 0 : theTimeIndexInOut < timeList->NumberOfItems());
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee theTime etäisyyden theTime2:een kun se suhteutetaan time1:n ja time2:n erotukseen.
// Eli mitä lähempänä theTime on time2:sta, sitä lähempänä arvo on 0:aa ja jos theTime
// on lähempänä time1:stä, arvo lähenee 1:stä.
// Jos time2:llä ja time2:lla ei ole eroa, palautetaan 0.
float NFmiQueryInfo::CalcTimeOffsetToLastTime(const NFmiMetTime &theTime,
                                              const NFmiMetTime &time1,
                                              const NFmiMetTime &time2)
{
  try
  {
    auto totalDiff = static_cast<float>(time2.DifferenceInMinutes(time1));
    auto diff1 = static_cast<float>(theTime.DifferenceInMinutes(time1));
    if (totalDiff)
    {
      // offset1 nimi on ehkä hämäävä, tarkoittaa että jos offset on 1, ollaan time1:n kohdalla ja
      // palautetaan theValue1
      // jos arvo 0, ollaan time2:n kohdalla ja palautetaan theValue2, muuten interpoloidaan niiden
      // välille.
      float offset1 = 1 - (diff1 / totalDiff);
      return offset1;
    }
    else
      return 1;  // eli palautetaan offset, jolla ollaan kiinni time1:ssä
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Jos X/Y on negatiivinen, mutta sen itseisarvo on pienempi kuin eps, annetaan arvoksi 0.
static double FixGridPointEpsErrorBelowZero(double theGridPointValue)
{
  try
  {
    if (theGridPointValue != kFloatMissing)
    {
      if (theGridPointValue < 0 && ::fabs(theGridPointValue) < gInterpolatedValueEps)
        return 0;
    }
    return theGridPointValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Jos X/Y on suurempi kuin datan reuna piste (=edge), mutta niiden erotus on pienempi kuin eps,
// annetaan arvoksi reunapiste.
static double FixGridPointEpsErrorOverTopEdge(double theGridPointValue, unsigned long theGridSize)
{
  try
  {
    if (theGridPointValue != kFloatMissing)
    {
      double edge = theGridSize - 1.;
      if (theGridPointValue > edge && ::fabs(theGridPointValue - edge) < gInterpolatedValueEps)
        return edge;
    }
    return theGridPointValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Jos theGridPoint on pikkuisen (käytetyn epsilonin rajoissa) data-alueen
// ulkopuolella (x/y suunnassa), korjataan hilapisteen arvoksi data-alueen reunaksi.
static void FixGridPointEpsError(NFmiPoint &theGridPoint,
                                 unsigned long theGridSizeX,
                                 unsigned long theGridSizeY)
{
  try
  {
    theGridPoint.X(::FixGridPointEpsErrorBelowZero(theGridPoint.X()));
    theGridPoint.Y(::FixGridPointEpsErrorBelowZero(theGridPoint.Y()));

    theGridPoint.X(::FixGridPointEpsErrorOverTopEdge(theGridPoint.X(), theGridSizeX));
    theGridPoint.Y(::FixGridPointEpsErrorOverTopEdge(theGridPoint.Y(), theGridSizeY));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee NFmiLocationCache -olin, missä on tieto siitä mihin kohtaan annettu latlon-piste osuu
// hilassa.
// Jos thePossibleSourceSizeX + Y ovat arvoltaan muuta kuin gMissingIndex, päätellään tarvitaanko
// tehdä bilineaarista interpolaatiota,
// vai riittääkö nearest piste interpolaatio.
// Osaa käsitellä myös ns. vajaan globaalin datan tapauksen, missä data menee pituuspiiri suunnassa
// esim. 0 => 359.5. Tällöin
// NFmiLocationCache -olion itsLocationIndex -dataosalle annetaan arvo gStrechedGlobalGridIndex.
// Tämän jälkeen GetCachedValues -metodi
// osaa kerätä oikeat arvot hilan äärilaidoilta (0-asteen sarake ja 359.5 -sarake) interpolointeja
// varten.
NFmiLocationCache NFmiQueryInfo::CalcLocationCache(const NFmiPoint &theLatlon,
                                                   unsigned long thePossibleSourceSizeX,
                                                   unsigned long thePossibleSourceSizeY)
{
  try
  {
    NFmiLocationCache locCache;
    if (IsGrid())  // laskut järkeviä vain jos on hiladatasta kysymys
    {
      if (Location(theLatlon, &locCache.itsGridPoint))
      {
        ::FixGridPointEpsError(locCache.itsGridPoint, itsGridXNumber, itsGridYNumber);
        locCache.itsLocationIndex = LocationIndex();
        if (thePossibleSourceSizeX != gMissingIndex && thePossibleSourceSizeY != gMissingIndex)
          locCache.CalcIsInterpolationNeeded(thePossibleSourceSizeX, thePossibleSourceSizeY);
      }
      else if (DoGlobalWrapFix(locCache.itsGridPoint))
      {
        locCache.itsLocationIndex = gStrechedGlobalGridIndex;
      }
    }
    return locCache;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Halutaan laskea this-infon avulla interpoloituja arvoja theTargetInfo:lle. Sitä varten lasketaan
// kerran
// halutut hilapisteet (targetin hilan pisteet this-hilassa). Sisältää myös tiedon hilapisteen
// indeksistä.
// Tätä voidaan käyttää apuna kun interpolidaan monta kertaan hilasta toiseen arvoja (esim. eri
// ajoille,
// leveleille ja parametreille), cache lasketaan vain kerran. Cache-pisteet voidaan sitten antaa eri
// InterpolatedValue-metodeille. Ks. mallia NFmiQueryDataUtil::FillGridData-metodista.
// Huom! toimii siis vain hiloille.
bool NFmiQueryInfo::CalcLatlonCachePoints(NFmiQueryInfo &theTargetInfo,
                                          NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  try
  {
    if (Grid() && theTargetInfo.Grid())
    {
      unsigned long sourceSizeX = Grid()->XNumber();
      unsigned long sourceSizeY = Grid()->YNumber();
      unsigned long targetSizeX = theTargetInfo.Grid()->XNumber();
      unsigned long targetSizeY = theTargetInfo.Grid()->YNumber();
      theLocationCache.Resize(targetSizeX, targetSizeY);
      for (theTargetInfo.ResetLocation(); theTargetInfo.NextLocation();)
      {
        NFmiLocationCache locCache =
            CalcLocationCache(theTargetInfo.LatLon(), sourceSizeX, sourceSizeY);
        theLocationCache[theTargetInfo.LocationIndex() % targetSizeX]
                        [theTargetInfo.LocationIndex() / targetSizeX] = locCache;
      }
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiTimeCache NFmiQueryInfo::CalcTimeCache(const NFmiMetTime &theTime)
{
  try
  {
    NFmiTimeCache timeCache;
    if (IsInside(theTime))
    {
      if (Time(theTime))
        timeCache.itsTimeIndex2 = timeCache.itsTimeIndex1 = TimeIndex();
      else
      {
        FindNearestTime(theTime, kBackward);  // tämä pitää löytyä!
        timeCache.itsTimeIndex1 = TimeIndex();
        timeCache.itsTimeIndex2 =
            timeCache.itsTimeIndex1 +
            1;  // vain peräkkäiset aikaindeksit mukaan, ei välitä onko niissä oikeasti arvoa vai ei
        NFmiMetTime time1 = Time();
        NextTime();
        NFmiMetTime time2 = Time();
        timeCache.itsOffset = 1.f - CalcTimeOffsetToLastTime(theTime, time1, time2);
      }
      timeCache.CalcIsInterpolationNeeded();
    }
    return timeCache;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Lasketaan this-infosta aika-laskuissa käytettävä timeCache vektori (jokaiselle aika-askeleelle)
// arvot.
// Jos targetin aika-askel ei osu ollenkaan this-datan aikaDescriptorin sisään, laitetaan
// cache-arvoksi
// oletus arvot eli puuttuvan arvot. Tällöin aikainterpolointi laskuissa palautetaan vain puuttuvaa.
// Muuten katsotaan osuuko target aika suoraan kohdalle. Jos osuu, kertoimellea ai ole väliä,
// asetetaan
// vain aikaindeksit samoiksi, jolloin tiedetään että aika-interpolointia ei tarvitse tehdä.
// Jos aikainterpolointia tarvitaan, laitetaan aikaIndex1:ksi edeltävä aika ja 2:een seuraava, miltä
// siis
// löytyy arvoja. Lisäsksi lasketaan offset-kerroin, jolloa interplointi tehdään.
// HUOM! nopeassa aikainterpolointi laskuissa ei voi ottaa huomioon maksimi aikaväliä, koska sitä ei
// voi laskea
// yhden pisteen avulla, joka saatetaan haluta ottaa huomioon, tällöin pitäisi käyttää vain
// normaalia aikainterpolaatiota.
bool NFmiQueryInfo::CalcTimeCache(NFmiQueryInfo &theTargetInfo,
                                  std::vector<NFmiTimeCache> &theTimeCache)
{
  try
  {
    theTimeCache.clear();
    for (theTargetInfo.ResetTime(); theTargetInfo.NextTime();)
    {
      NFmiTimeCache timeCache = CalcTimeCache(theTargetInfo.Time());
      theTimeCache.push_back(timeCache);
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::InterpolatedValueFromTimeList(const NFmiMetTime &theTime,
                                                   int theMaxMinuteRange)
{
  try
  {
    unsigned long oldTimeIndex = TimeIndex();
    float returnValue = kFloatMissing;
    NFmiTimeList *timeList = itsTimeDescriptor->ValidTimeList();
    if (timeList)
    {
      if (theTime < timeList->FirstTime())
        return kFloatMissing;  // jos 1. aika oli jo suurempi kuin  haluttu, ei voida interpoloida.
      // Tämä tilanne pitää tarkistaa, koska jos theMaxMinuteRange on 0, interpolointi
      // onnistuisi aina tälläisessä tapauksessa. Tämä korjaa reiän koodissa, ylemmille tasoille
      // voisi laittaa varmaan suoraan tarkastuksen, että jos haluttu aika ei ole timeDescriptorin
      // sisällä, ei interpolointi onnistu.

      NFmiMetTime time1 = NFmiMetTime::gMissingTime;  // this avoids calling SetZoneDifferenceHour
                                                      // which may cause locks
      NFmiMetTime time2 = NFmiMetTime::gMissingTime;

      int index1 = 0, index2 = 0;
      if ((index1 = timeList->FindNearestTimes(
               theTime, theMaxMinuteRange ? theMaxMinuteRange : kLongMissing, time1, time2)) != -1)
      {
        FmiInterpolationMethod interpolationMethod = Param().GetParam()->InterpolationMethod();
        if (interpolationMethod == kNearestPoint || interpolationMethod == kNoneInterpolation)
          returnValue = PeekValue(Index(ParamIndex(), LocationIndex(), LevelIndex(), index1));
        else
        {
          TimeIndex(index1);
          float value1 = FloatValue();
          index2 = index1 + 1;
          TimeIndex(index2);
          float value2 = FloatValue();
          if ((value1 != kFloatMissing && value1 != kTCombinedWeatherFloatMissing) ||
              (value2 != kFloatMissing && value2 != kTCombinedWeatherFloatMissing))
          {
            if (Param().GetParamIdent() == kFmiWindDirection)
            {  // tehdään wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
              Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
              TimeIndex(
                  index1);  // otetaan 1. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
              float ws1 = FloatValue();
              TimeIndex(
                  index2);  // otetaan 2. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
              float ws2 = FloatValue();
              Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
              NFmiInterpolation::WindInterpolator windInterpolator;
              float offset1 = CalcTimeOffsetToLastTime(theTime, time1, time2);
              windInterpolator.operator()(ws1, value1, offset1);
              windInterpolator.operator()(ws2, value2, (1 - offset1));
              returnValue = static_cast<float>(windInterpolator.Direction());  // meitä kiinnostaa
                                                                               // vain tuulen suunta
                                                                               // (tämä
              // pitää tehdä jotenkin
              // fiksummin)
            }
            else
              returnValue = Interpolate(Param(), theTime, time1, time2, value1, value2);
          }
          else
            returnValue = kFloatMissing;
        }
      }
    }

    TimeIndex(oldTimeIndex);
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::Interpolate(const NFmiDataIdent &theDataIdent,
                                 const NFmiMetTime &theTime,
                                 const NFmiMetTime &theTime1,
                                 const NFmiMetTime &theTime2,
                                 float theValue1,
                                 float theValue2)
{
  try
  {
    if (theValue1 == kFloatMissing && theValue2 == kFloatMissing)
      return kFloatMissing;

    FmiInterpolationMethod interp = theDataIdent.GetParam()->InterpolationMethod();
    float returnValue = kFloatMissing;
    float offset1 = CalcTimeOffsetToLastTime(theTime, theTime1, theTime2);
    if (offset1 == 1)
      returnValue = theValue1;
    else if (offset1 == 0)
      returnValue = theValue2;
    else if (interp == kNearestNonMissing)
    {
      if (offset1 <= 0.5)
      {
        returnValue = theValue2;
        if (returnValue == kFloatMissing)
          returnValue = theValue1;
      }
      else
      {
        returnValue = theValue1;
        if (returnValue == kFloatMissing)
          returnValue = theValue2;
      }
    }
    else if (theValue1 == kFloatMissing || theValue2 == kFloatMissing)
    {  // Jos toinen arvoista puuttuva, palautetaan suoraan se arvo, jota lähempänä offset1 on, oli
       // se
      // puuttuva tai ei
      if (offset1 <= 0.5)
        return theValue2;
      else
        return theValue1;
    }
    else
    {
      // 30.10.2000/Marko Muutin ottamaan huomioon combinedparam jutut ja epälineaariset parametrit
      unsigned long param = theDataIdent.GetParam()->GetIdent();
      if (interp == kByCombinedParam ||
          param == kFmiTotalWindMS)  // HUOM!!!! totalwindillä ei ole tätä arvoa (korjaa!)
      {
        switch (param)
        {
          case kFmiTotalWindMS:
          {  // korjasin käyttämään SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien
             // avulla
            NFmiTotalWind resultWind(itsInfoVersion);
            NFmiTotalWind tempWind1(theValue1, kFmiPackedWind, itsInfoVersion);
            NFmiTotalWind tempWind2(theValue2, kFmiPackedWind, itsInfoVersion);
            resultWind.SetToWeightedMean(
                &tempWind1, offset1, &tempWind2, 1 - offset1, &tempWind1, 0, &tempWind1, 0);
            returnValue = resultWind.TransformedFloatValue();
          }
          break;
          case kFmiWeatherAndCloudiness:
          {
            NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
            NFmiWeatherAndCloudiness tempWeather1(
                theValue1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
            NFmiWeatherAndCloudiness tempWeather2(
                theValue2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

            resultWeather.SetToWeightedMean(&tempWeather1,
                                            offset1,
                                            &tempWeather2,
                                            1 - offset1,
                                            &tempWeather1,
                                            0,
                                            &tempWeather1,
                                            0);
            returnValue = resultWeather.TransformedFloatValue();
          }
          break;
        }
      }
      else if (param ==
               kFmiWindVectorMS)  // windvector laskut interpoloituina, vaikka se on 'nearest'
        returnValue =
            static_cast<float>(NFmiInterpolation::WindVector(1 - offset1, theValue1, theValue2));
      else if (interp != kLinearly)
      {
        if (offset1 <= 0.5)
          returnValue = theValue2;
        else
          returnValue = theValue1;
      }
      else  // muuten lineaarinen interpolointi
      {
        if (param == kFmiWindVectorMS)
        {  // HUOM!! korjaa koodeja niin että kFmiWindDirection -parametrilla ei tätä kutsuta!!!!
          returnValue =
              static_cast<float>(NFmiInterpolation::WindVector(1 - offset1, theValue1, theValue2));
        }
        else if (param == kFmiWindDirection || param == kFmiWaveDirection)
        {  // HUOM!! korjaa koodeja niin että kFmiWindDirection -parametrilla ei tätä kutsuta!!!!
          returnValue =
              static_cast<float>(NFmiInterpolation::ModLinear(1 - offset1, theValue1, theValue2));
        }
        else
          returnValue = float(offset1 * theValue1 + (1.f - offset1) * theValue2);
      }
    }
    return returnValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theOtherInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// 1999.10.18/Marko Yhdistää this-infon ja parametrina annetun infon datan.
// Luo niistä uuden yhdistetyn querydatan. Tein metodin NFmiQueryInfo:oon,
// enkä NFmiQueryData:an, koska näin käytön saa optimoitua (käyttämällä
// NFmiFastQueryInfo:ja).
// Huom!!! Tätä ei ole optimoitu vielä yhtään. Esim. tarkastelemalla
// aika,paikka descriptoreita, voitaisiin tehä huomattavia nopeutuksia

NFmiQueryData *NFmiQueryInfo::CreateCombinedData(NFmiQueryInfo *theOtherInfo)
{
  try
  {
    NFmiQueryInfo *combinedInfo1 = CreateCombinedInfo(theOtherInfo);
    auto *combinedData = new NFmiQueryData(*combinedInfo1);
    combinedData->Init();

    // ongelmia ottaa käyttöön fastqueryinfo sujuvasti pitää nyt luoda tässä tavallinen info

    auto *combinedInfo = new NFmiQueryInfo(combinedData);
    First();
    combinedInfo->First();
    theOtherInfo->First();

    for (combinedInfo->ResetTime(); combinedInfo->NextTime();)
    {
      for (combinedInfo->ResetParam(); combinedInfo->NextParam();)
      {
        for (combinedInfo->ResetLocation(); combinedInfo->NextLocation();)
        {
          if (combinedInfo->IsGrid())
          {
            if (Time(combinedInfo->Time()) && Param(combinedInfo->Param()) &&
                Location(combinedInfo->LatLon()))
            {
              combinedInfo->FloatValue(FloatValue());
            }
            else if (theOtherInfo->Time(combinedInfo->Time()) &&
                     theOtherInfo->Param(combinedInfo->Param()) &&
                     theOtherInfo->Location(combinedInfo->LatLon()))
            {
              combinedInfo->FloatValue(theOtherInfo->FloatValue());
            }
          }
          else
          {
            if (Time(combinedInfo->Time()) && Param(combinedInfo->Param()) &&
                Location(*combinedInfo->Location()))
            {
              combinedInfo->FloatValue(FloatValue());
            }
            else if (theOtherInfo->Time(combinedInfo->Time()) &&
                     theOtherInfo->Param(combinedInfo->Param()) &&
                     theOtherInfo->Location(*combinedInfo->Location()))
            {
              combinedInfo->FloatValue(theOtherInfo->FloatValue());
            }
          }
        }
      }
    }

    delete combinedInfo1;
    delete combinedInfo;
    return combinedData;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theOtherInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiQueryInfo::CreateCombinedInfo(NFmiQueryInfo *theOtherInfo)
{
  try
  {
    return new NFmiQueryInfo(itsParamDescriptor->Combine(theOtherInfo->ParamDescriptor()),
                             itsTimeDescriptor->Combine(theOtherInfo->TimeDescriptor()),
                             itsHPlaceDescriptor->Combine(theOtherInfo->HPlaceDescriptor()),
                             *itsVPlaceDescriptor);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa gridin tapauksessa currentista hilapisteestä offsettien avulla
 * osoitetun paikan paikan. Jos ei ole gridi tai menee hilan ohi, palauttaa
 * pisteen, jonka x ja y koordinaatit ovat kFloatMissing
 *
 * \param theXOffset Undocumented, unused
 * \param theYOffset Undocumented, unused
 * \return Undocumented
 * \todo Not implemented properly
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiQueryInfo::PeekLocationLatLon(int /* theXOffset */, int /* theYOffset */)
{
  try
  {
    if (IsGrid())
    {
      // katso toteutuksesta mallia NFmiFastQueryInfo:sta
      // JA toteuta TÄMÄ sitten!!! (1999.10.28/Marko)
    }
    return NFmiPoint(kFloatMissing, kFloatMissing);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedInterpolation(const NFmiLocationCache &theLocationCache)
{
  try
  {
    if (theLocationCache.NoValue())
      return kFloatMissing;

    // metodin loputtua paikka indeksi ei saa olla muuttunut, indeksi talteen tässä!!!
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    float value = kFloatMissing;
    if (theLocationCache.NoInterpolation())
      value = FloatValue();
    else
    {
      // optimointia, jos ollaan tarpeeksi lähellä hilaa, palautetaan suoraan hilan arvo
      if (fabs(theLocationCache.itsGridPoint.X() - round(theLocationCache.itsGridPoint.X())) <
              gInterpolatedValueEps &&
          fabs(theLocationCache.itsGridPoint.Y() - round(theLocationCache.itsGridPoint.Y())) <
              gInterpolatedValueEps)
        return FloatValue();

      NFmiDataIdent &param = Param();
      FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
      bool wcParam = IsSubParamUsed() && ::IsWeatherSubParam(*this);
      if (wcParam)  // HESSAA, hsade, sateenolomuoto ja sateen tyypit pitää hoitaa erikoistapauksina
        return InterpolatedValueForCombinedParam(theLocationCache.itsGridPoint);
      else if (interp == kNearestPoint || interp == kNoneInterpolation)
      {
        if (theLocationCache.itsLocationIndex == gStrechedGlobalGridIndex)
        {
          unsigned long gridSizeX = Grid()->XNumber();
          double xFraction = theLocationCache.itsGridPoint.X() -
                             static_cast<int>(theLocationCache.itsGridPoint.X());
          // xFraction:ista riippuen otetaan lähin hilapiste joko maailman oikeasta tai vasemmasta
          // reunasta
          int xInd = (xFraction > 0.5) ? 0 : gridSizeX - 1;
          auto yInd = static_cast<int>(
              round(theLocationCache.itsGridPoint
                        .Y()));  // y-indeksi vain pyöristetään lähimpään pisteeseen
          LocationIndex(yInd * gridSizeX + xInd);
          value = FloatValue();
        }
        else
          value = FloatValue();
      }
      else
      {
        std::array<float, 4> values;
        GetCachedValues(theLocationCache, values);
        auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
        value = CachedLocationInterpolatedValue(values, theLocationCache, interp, parId);
      }
    }
    // palautetaan indeksi lopuksi
    LocationIndex(oldLocationIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedInterpolation(const NFmiTimeCache &theTimeCache)
{
  try
  {
    if (theTimeCache.NoValue())
      return kFloatMissing;

    // metodin loputtua aika indeksi ei saa olla muuttunut, indeksi talteen tässä!!!
    unsigned long oldTimeIndex = TimeIndex();
    std::array<float, 2> values;
    GetCachedValues(theTimeCache, values);
    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
    float value = CachedTimeInterpolatedValue(values[0], values[1], theTimeCache, interp, parId);
    // palautetaan indeksi lopuksi
    TimeIndex(oldTimeIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float InterpolateWandC(std::array<float, 4> &theValues,
                              const NFmiPoint &theGridPoint,
                              double theInfoVersion)
{
  try
  {
    NFmiWeatherAndCloudiness bl(theValues[0], kFmiPackedWeather, kFloatMissing, theInfoVersion);
    NFmiWeatherAndCloudiness br(theValues[1], kFmiPackedWeather, kFloatMissing, theInfoVersion);
    NFmiWeatherAndCloudiness tr(theValues[2], kFmiPackedWeather, kFloatMissing, theInfoVersion);
    NFmiWeatherAndCloudiness tl(theValues[3], kFmiPackedWeather, kFloatMissing, theInfoVersion);
    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    NFmiWeatherAndCloudiness weather(theInfoVersion);
    weather.SetToWeightedMean(&bl,
                              float((1. - dx) * (1. - dy)),
                              &br,
                              float(dx * (1. - dy)),
                              &tl,
                              float((1. - dx) * dy),
                              &tr,
                              float(dx * dy));

    return weather.TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float InterpolateTotalWind(std::array<float, 4> &theValues,
                                  const NFmiPoint &theGridPoint,
                                  double theInfoVersion)
{
  try
  {
    NFmiTotalWind bl(theValues[0], kFmiPackedWind, theInfoVersion);
    NFmiTotalWind br(theValues[1], kFmiPackedWind, theInfoVersion);
    NFmiTotalWind tr(theValues[2], kFmiPackedWind, theInfoVersion);
    NFmiTotalWind tl(theValues[3], kFmiPackedWind, theInfoVersion);
    double dx = theGridPoint.X() - int(theGridPoint.X());
    double dy = theGridPoint.Y() - int(theGridPoint.Y());

    NFmiTotalWind wind(theInfoVersion);
    wind.SetToWeightedMean(&bl,
                           float((1. - dx) * (1. - dy)),
                           &br,
                           float(dx * (1. - dy)),
                           &tl,
                           float((1. - dx) * dy),
                           &tr,
                           float(dx * dy));

    return wind.TransformedFloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float InterpolateWindDir(std::array<float, 4> &theWSvalues,
                                std::array<float, 4> &theWDvalues,
                                const NFmiPoint &theGridPoint)
{
  try
  {
    double dx = theGridPoint.X() - floor(theGridPoint.X());
    double dy = theGridPoint.Y() - floor(theGridPoint.Y());
    NFmiInterpolation::WindInterpolator windInterpolator;
    windInterpolator.operator()(theWSvalues[0], theWDvalues[0], (1 - dx) * (1 - dy));
    windInterpolator.operator()(theWSvalues[1], theWDvalues[1], dx *(1 - dy));
    windInterpolator.operator()(theWSvalues[2], theWDvalues[2], dx *dy);
    windInterpolator.operator()(theWSvalues[3], theWDvalues[3], (1 - dx) * dy);

    return static_cast<float>(windInterpolator.Direction());  // meitä kiinnostaa vain tuulen suunta
    // (tämä pitää tehdä jotenkin fiksummin)
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float InterpolateWindVector(std::array<float, 4> &theWindVectorvalues,
                                   const NFmiPoint &theGridPoint)
{
  try
  {
    double dx = theGridPoint.X() - floor(theGridPoint.X());
    double dy = theGridPoint.Y() - floor(theGridPoint.Y());
    return static_cast<float>(NFmiInterpolation::WindVector(dx,
                                                            dy,
                                                            theWindVectorvalues[3],
                                                            theWindVectorvalues[2],
                                                            theWindVectorvalues[0],
                                                            theWindVectorvalues[1]));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedLocationInterpolatedValue(std::array<float, 4> &theValues,
                                                     const NFmiLocationCache &theLocationCache,
                                                     FmiInterpolationMethod theInterpolationMethod,
                                                     FmiParameterName theParId)
{
  try
  {
    float value = kFloatMissing;
    const NFmiPoint &gpoint = theLocationCache.itsGridPoint;

    if (theInterpolationMethod == kByCombinedParam)
    {
      if (theParId == kFmiWeatherAndCloudiness)
        value = ::InterpolateWandC(theValues, theLocationCache.itsGridPoint, InfoVersion());
      else if (theParId == kFmiTotalWindMS)
        value = ::InterpolateTotalWind(theValues, theLocationCache.itsGridPoint, InfoVersion());
    }
    else if (theInterpolationMethod == kNearestPoint)
    {
      value = static_cast<float>(NFmiInterpolation::NearestPoint(gpoint.X() - floor(gpoint.X()),
                                                                 gpoint.Y() - floor(gpoint.Y()),
                                                                 theValues[3],
                                                                 theValues[2],
                                                                 theValues[0],
                                                                 theValues[1]));
    }
    else if (theInterpolationMethod == kNearestNonMissing)
    {
      value =
          static_cast<float>(NFmiInterpolation::NearestNonMissing(gpoint.X() - floor(gpoint.X()),
                                                                  gpoint.Y() - floor(gpoint.Y()),
                                                                  theValues[3],
                                                                  theValues[2],
                                                                  theValues[0],
                                                                  theValues[1]));
    }
    else
    {
      // Nearest interpolaatio on otettu jo ennen tämän kutsua.
      // return InterpolatedValueForCombinedParam(gpoint, xShift, yShift);  // pitääkö nämäkin ottaa
      // erikseen huomioon???
      if (theParId == kFmiWindDirection)
      {  // tehdään wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
        Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
        std::array<float, 4> WSvalues;
        GetCachedValues(theLocationCache, WSvalues);
        Param(theParId);  // palautetaan tuulensuunta takaisin parametriksi
        value = ::InterpolateWindDir(WSvalues, theValues, gpoint);
      }
      else if (theParId == kFmiWindVectorMS)
      {
        value = ::InterpolateWindVector(theValues, gpoint);
      }
      else if (theParId == kFmiWaveDirection)
      {
        // modulo 360 interpolation
        value = static_cast<float>(NFmiInterpolation::ModBiLinear(gpoint.X() - floor(gpoint.X()),
                                                                  gpoint.Y() - floor(gpoint.Y()),
                                                                  theValues[3],
                                                                  theValues[2],
                                                                  theValues[0],
                                                                  theValues[1]));
      }
      else
      {
        value = static_cast<float>(NFmiInterpolation::BiLinear(gpoint.X() - floor(gpoint.X()),
                                                               gpoint.Y() - floor(gpoint.Y()),
                                                               theValues[3],
                                                               theValues[2],
                                                               theValues[0],
                                                               theValues[1]));
      }
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedTimeInterpolatedValue(float theValue1,
                                                 float theValue2,
                                                 const NFmiTimeCache &theTimeCache,
                                                 FmiInterpolationMethod theInterpolationMethod,
                                                 FmiParameterName theParId)
{
  try
  {
    float value = kFloatMissing;
    float offset = theTimeCache.itsOffset;
    if (theInterpolationMethod == kByCombinedParam)
    {
      if (theParId == kFmiTotalWindMS)
      {  // korjasin käyttämään SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien avulla
        NFmiTotalWind resultWind(itsInfoVersion);
        NFmiTotalWind tempWind1(theValue1, kFmiPackedWind, itsInfoVersion);
        NFmiTotalWind tempWind2(theValue2, kFmiPackedWind, itsInfoVersion);
        resultWind.SetToWeightedMean(
            &tempWind1, 1 - offset, &tempWind2, offset, &tempWind1, 0, &tempWind1, 0);
        value = resultWind.TransformedFloatValue();
      }
      else
      {                                                          // kFmiWeatherAndCloudiness:
        NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
        NFmiWeatherAndCloudiness tempWeather1(
            theValue1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
        NFmiWeatherAndCloudiness tempWeather2(
            theValue2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

        resultWeather.SetToWeightedMean(
            &tempWeather1, 1 - offset, &tempWeather2, offset, &tempWeather1, 0, &tempWeather1, 0);
        value = resultWeather.TransformedFloatValue();
      }
    }
    else
    {
      if (theValue1 == kFloatMissing && theValue2 == kFloatMissing)
        value = kFloatMissing;
      else if (theValue1 == kFloatMissing)
      {
        if (offset > 0.5)
          value = theValue2;
        else
          value = kFloatMissing;
      }
      else if (theValue2 == kFloatMissing)
      {
        if (offset <= 0.5)
          value = theValue1;
        else
          value = kFloatMissing;
      }
      else
      {
        if (theInterpolationMethod == kNearestNonMissing)
        {
          if (offset <= 0.5)
          {
            value = theValue1;
            if (value == kFloatMissing)
              value = theValue2;
          }
          else
          {
            value = theValue2;
            if (value == kFloatMissing)
              value = theValue1;
          }
        }
        else if (theInterpolationMethod != kLinearly)
        {
          if (offset <= 0.5)
            value = theValue1;
          else
            value = theValue2;
        }
        else  // muuten lineaarinen interpolointi
        {
          if (theParId == kFmiWindDirection)
          {  // tehdään wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
            unsigned long oldTimeIndex = TimeIndex();
            Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
            TimeIndex(theTimeCache.itsTimeIndex1);
            float ws1 = FloatValue();
            TimeIndex(theTimeCache.itsTimeIndex2);
            float ws2 = FloatValue();
            TimeIndex(oldTimeIndex);
            Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
            NFmiInterpolation::WindInterpolator windInterpolator;
            windInterpolator.operator()(ws1, theValue1, offset);
            windInterpolator.operator()(ws2, theValue2, (1 - offset));
            value =
                static_cast<float>(windInterpolator.Direction());  // meitä kiinnostaa vain tuulen
            // suunta (tämä pitää tehdä
            // jotenkin fiksummin)
          }
          else if (theParId == kFmiWaveDirection)
            value =
                static_cast<float>(NFmiInterpolation::ModLinear(offset, theValue1, theValue2, 360));
          else if (theParId == kFmiWindVectorMS)
            value = static_cast<float>(NFmiInterpolation::WindVector(offset, theValue1, theValue2));
          else
            value = static_cast<float>(NFmiInterpolation::Linear(offset, theValue1, theValue2));
        }
      }
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedInterpolation(const NFmiLocationCache &theLocationCache,
                                         const NFmiTimeCache &theTimeCache)
{
  try
  {
    if (theLocationCache.NoValue() || theTimeCache.NoValue())
      return kFloatMissing;

    // metodin loputtua paikka/aika indeksit eivät saa olla muuttuneita, indeksit talteen tässä!!!
    unsigned long oldTimeIndex = TimeIndex();
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    TimeIndex(theTimeCache.itsTimeIndex1);
    float value = kFloatMissing;
    if (theTimeCache.NoInterpolation())
    {
      if (theLocationCache.NoInterpolation())
        value = FloatValue();
      else
      {
        TimeIndex(theTimeCache.itsTimeIndex1);
        value = CachedInterpolation(theLocationCache);
      }
    }
    else
    {
      if (theLocationCache.NoInterpolation())
        value = CachedInterpolation(theTimeCache);
      else
      {
        // tehdään sitten sekä aika että paikka cached interpolaatio
        NFmiDataIdent &param = Param();
        FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
        if (interp == kNearestPoint || interp == kNoneInterpolation)
        {
          if (theTimeCache.itsOffset > 0.5f)
            TimeIndex(theTimeCache.itsTimeIndex2);
          value = FloatValue();
        }
        else
        {
          std::array<float, 4> values1;
          std::array<float, 4> values2;
          GetCachedValues(theLocationCache, theTimeCache, values1, values2);
          auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
          float value1 = CachedLocationInterpolatedValue(values1, theLocationCache, interp, parId);
          float value2 = CachedLocationInterpolatedValue(values2, theLocationCache, interp, parId);
          value = CachedTimeInterpolatedValue(value1, value2, theTimeCache, interp, parId);
        }
      }
    }
    // palautetaan indeksit lopuksi
    TimeIndex(oldTimeIndex);
    LocationIndex(oldLocationIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseissä:
// 0 -> time1
// 1 -> time2
void NFmiQueryInfo::GetCachedValues(const NFmiTimeCache &theTimeCache,
                                    std::array<float, 2> &theValues)
{
  try
  {
    TimeIndex(theTimeCache.itsTimeIndex1);
    theValues[0] = FloatValue();
    TimeIndex(theTimeCache.itsTimeIndex2);
    theValues[1] = FloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseissä:
// 0-3 -> time1 bl,br,tr,tl
void NFmiQueryInfo::GetCachedValues(const NFmiLocationCache &theLocationCache,
                                    const NFmiTimeCache &theTimeCache,
                                    std::array<float, 4> &theValues1,
                                    std::array<float, 4> &theValues2)
{
  try
  {
    TimeIndex(theTimeCache.itsTimeIndex1);
    GetCachedValues(theLocationCache, theValues1);
    TimeIndex(theTimeCache.itsTimeIndex2);
    GetCachedValues(theLocationCache, theValues2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiLocationCache &theLocationCache,
                                                 const NFmiTimeCache &theTimeCache,
                                                 std::array<float, 4> &theValues1,
                                                 std::array<float, 4> &theValues2)
{
  try
  {
    TimeIndex(theTimeCache.itsTimeIndex1);
    GetCachedPressureLevelValues(P, theLocationCache, theValues1);
    TimeIndex(theTimeCache.itsTimeIndex2);
    GetCachedPressureLevelValues(P, theLocationCache, theValues2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiQueryInfo::DoGlobalWrapFix(const NFmiPoint &theGridPoint) const
{
  try
  {
    if (fIsStrechableGlobalGrid)
    {
      unsigned long xsize = Grid()->XNumber();
      if (theGridPoint.X() > xsize - 1 && theGridPoint.X() <= xsize)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseissä:

void NFmiQueryInfo::GetCachedValues(const NFmiLocationCache &theLocationCache,
                                    std::array<float, 4> &theValues)
{
  try
  {
    const NFmiPoint &gpoint = theLocationCache.itsGridPoint;
    if (theLocationCache.itsLocationIndex == gStrechedGlobalGridIndex)
    {
      unsigned long xsize = Grid()->XNumber();
      // Lasketaan ensin hilan oikean reunan alempi hilapiste
      unsigned long newLocationIndex = static_cast<unsigned long>(gpoint.Y()) * xsize + (xsize - 1);
      LocationIndex(newLocationIndex);
      theValues[0] = PeekLocationValue(0, 0);
      theValues[3] = PeekLocationValue(0, 1);
      // Lasketaan sitten hilan vasemman reunan alempi hilapiste
      newLocationIndex = static_cast<unsigned long>(gpoint.Y()) * xsize + 0;
      LocationIndex(newLocationIndex);
      theValues[1] = PeekLocationValue(0, 0);
      theValues[2] = PeekLocationValue(0, 1);
    }
    else
    {
      LocationIndex(theLocationCache.itsLocationIndex);
      // Otettu hilapisteen liikuttelu koodi InterpolatedValue(latlon) -metodista.
      int xShift = static_cast<int>(::round(gpoint.X())) - static_cast<int>(gpoint.X());
      int yShift = static_cast<int>(::round(gpoint.Y())) - static_cast<int>(gpoint.Y());

      theValues[0] = PeekLocationValue(0 - xShift, 0 - yShift);
      theValues[3] = PeekLocationValue(0 - xShift, 1 - yShift);
      theValues[1] = PeekLocationValue(1 - xShift, 0 - yShift);
      theValues[2] = PeekLocationValue(1 - xShift, 1 - yShift);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiLocationCache &theLocationCache,
                                                 std::array<float, 4> &theValues)
{
  try
  {
    LocationIndex(theLocationCache.itsLocationIndex);
    const NFmiPoint &gpoint = theLocationCache.itsGridPoint;
    // Otettu hilapisteen liikuttelu koodi InterpolatedValue(latlon) -metodista.
    int xShift = static_cast<int>(::round(gpoint.X())) - static_cast<int>(gpoint.X());
    int yShift = static_cast<int>(::round(gpoint.Y())) - static_cast<int>(gpoint.Y());

    if (xShift == 1)
      MoveLeft();
    if (yShift == 1)
      MoveDown();

    theValues[0] = PressureLevelValue(P);
    MoveRight();
    theValues[1] = PressureLevelValue(P);
    MoveLeft();
    MoveUp();
    theValues[3] = PressureLevelValue(P);
    MoveRight();
    theValues[2] = PressureLevelValue(P);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedPressureLevelValue(float P, const NFmiLocationCache &theLocationCache)
{
  try
  {
    if (theLocationCache.NoValue())
      return kFloatMissing;

    // metodin loputtua paikka indeksi ei saa olla muuttunut, indeksi talteen tässä!!!
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    float value = kFloatMissing;
    if (theLocationCache.NoInterpolation())
      value = PressureLevelValue(P);
    else
    {
      // optimointia, jos ollaan tarpeeksi lähellä hilaa, palautetaan suoraan hilan arvo
      if (fabs(theLocationCache.itsGridPoint.X() - round(theLocationCache.itsGridPoint.X())) <
              gInterpolatedValueEps &&
          fabs(theLocationCache.itsGridPoint.Y() - round(theLocationCache.itsGridPoint.Y())) <
              gInterpolatedValueEps)
        return PressureLevelValue(P);

      NFmiDataIdent &param = Param();
      FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
      if (interp == kNearestPoint)
        value = PressureLevelValue(P);
      else
      {
        std::array<float, 4> values;
        GetCachedPressureLevelValues(P, theLocationCache, values);
        auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
        value = CachedLocationInterpolatedValue(values, theLocationCache, interp, parId);
      }
    }
    // palautetaan indeksi lopuksi
    LocationIndex(oldLocationIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedPressureLevelValue(float P, const NFmiTimeCache &theTimeCache)
{
  try
  {
    if (theTimeCache.NoValue())
      return kFloatMissing;

    // metodin loputtua aika indeksi ei saa olla muuttunut, indeksi talteen tässä!!!
    unsigned long oldTimeIndex = TimeIndex();
    std::array<float, 2> values;
    GetCachedPressureLevelValues(P, theTimeCache, values);
    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
    float value = CachedTimeInterpolatedValue(values[0], values[1], theTimeCache, interp, parId);
    // palautetaan indeksi lopuksi
    TimeIndex(oldTimeIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiTimeCache &theTimeCache,
                                                 std::array<float, 2> &theValues)
{
  try
  {
    TimeIndex(theTimeCache.itsTimeIndex1);
    theValues[0] = PressureLevelValue(P);
    TimeIndex(theTimeCache.itsTimeIndex2);
    theValues[1] = PressureLevelValue(P);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiQueryInfo::CachedPressureLevelValue(float P,
                                              const NFmiLocationCache &theLocationCache,
                                              const NFmiTimeCache &theTimeCache)
{
  try
  {
    if (theLocationCache.NoValue() || theTimeCache.NoValue())
      return kFloatMissing;

    // metodin loputtua paikka/aika indeksit eivät saa olla muuttuneita, indeksit talteen tässä!!!
    unsigned long oldTimeIndex = TimeIndex();
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    TimeIndex(theTimeCache.itsTimeIndex1);
    float value = kFloatMissing;
    if (theTimeCache.NoInterpolation())
    {
      if (theLocationCache.NoInterpolation())
        value = PressureLevelValue(P);
      else
      {
        TimeIndex(theTimeCache.itsTimeIndex1);
        value = CachedPressureLevelValue(P, theLocationCache);
      }
    }
    else
    {
      NFmiDataIdent &param = Param();
      FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
      if (theLocationCache.NoInterpolation() || interp == kNearestPoint ||
          interp == kNoneInterpolation)
        value = CachedPressureLevelValue(P, theTimeCache);
      else
      {  // tehdään sitten sekä aika että paikka cached interpolaatio
        std::array<float, 4> values1;
        std::array<float, 4> values2;
        GetCachedPressureLevelValues(P, theLocationCache, theTimeCache, values1, values2);
        auto parId = static_cast<FmiParameterName>(param.GetParamIdent());
        float value1 = CachedLocationInterpolatedValue(values1, theLocationCache, interp, parId);
        float value2 = CachedLocationInterpolatedValue(values2, theLocationCache, interp, parId);
        value = CachedTimeInterpolatedValue(value1, value2, theTimeCache, interp, parId);
      }
    }
    // palautetaan indeksit lopuksi
    TimeIndex(oldTimeIndex);
    LocationIndex(oldLocationIndex);
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Haluttu piste, mihin arvo halutaan.
 * \param theTime Haluttu aika, johon arvo halutaan.
 * \param theMaxMinuteRange Aika rajoitin aikainterpolointiin.
 * \return Haluttuun paikkaan ja aikaan interpoloitu arvo tai
 *   puuttuva arvo, jos interpolointi mahdotonta.
 */
// ----------------------------------------------------------------------

// 1.6.2004/Marko EI toimi vielä täysin!
// InterpolatedValueForCombinedParam ei toimi vielä ajan suhteen, mutta sillä ei ole mielestäni
// väliä
// Koska tällöin haluttaisiin WeatherAndCloudiness ali parametriä interpoloituna ajan ja paikan
// suhteen
// ja sen teen kuntoon jos sitä oikeasti tarvitaan.
// HUOM!!! Tämä voi muuttaa infon locationindexiä!!!!!
float NFmiQueryInfo::InterpolatedValue(const NFmiPoint &theLatLonPoint,
                                       const NFmiMetTime &theTime,
                                       int theMaxMinuteRange)
{
  try
  {
    if (Time(theTime))
      return InterpolatedValue(
          theLatLonPoint);  // jos aika löytyy suoraan, turha tehdä erillistä aikainterpolointia

    float theValue = kFloatMissing;
    NFmiParam *parameter = Param().GetParam();
    if (parameter == nullptr)  // parametria ei ole asetettu kohdalleen!!!!
    {
      assert(parameter);
      return theValue;
    }
    FmiInterpolationMethod interp = parameter->InterpolationMethod();
    auto param = static_cast<FmiParameterName>(parameter->GetIdent());

    bool wcParam = IsSubParamUsed() && ::IsWeatherSubParam(*this);

    if (IsGrid() && ((interp != kNoneInterpolation && interp != kNearestPoint) || wcParam ||
                     param == kFmiWindVectorMS))
    {
      NFmiPoint gpoint;
      if (Location(theLatLonPoint, &gpoint))
      {
        // optimointia, jos ollaan tarpeeksi lähellä hilaa, palautetaan suoraan hilan arvo
        if (fabs(gpoint.X() - round(gpoint.X())) < 0.00001 &&
            fabs(gpoint.Y() - round(gpoint.Y())) < 0.00001)
          return InterpolatedValue(theTime, theMaxMinuteRange);

        // Location(theLatLonPoint)-metodi asettaa lähimpään hilapisteeseen, josta seuraa se,
        // että PeekLocationValue:lle pitää laskea mahdollinen siirtymä
        int xShift = static_cast<int>(round(gpoint.X())) - static_cast<int>(gpoint.X());
        int yShift = static_cast<int>(round(gpoint.Y())) - static_cast<int>(gpoint.Y());

        if (wcParam)  // interp == kByCombinedParam) //16.9.99 tämä haara/Lasse // 1999.09.21/Marko
                      // Miten näistä pääsee eroon?
        {
          SetIsSubParamUsed(false);
          NFmiWeatherAndCloudiness tmpWeather(itsInfoVersion);
          tmpWeather.TransformFromFloatValue(InterpolatedValueForWeatherAndCloudiness(
              theTime, theMaxMinuteRange, gpoint, xShift, yShift));
          SetIsSubParamUsed(true);
          return static_cast<float>(tmpWeather.SubValue(param));

          //				return InterpolatedValueForCombinedParam(gpoint, xShift,
          // yShift);
        }
        else if (interp == kByCombinedParam)
        {
          if (param == kFmiWeatherAndCloudiness)
            return InterpolatedValueForWeatherAndCloudiness(
                theTime, theMaxMinuteRange, gpoint, xShift, yShift);
          else if (param == kFmiTotalWindMS)
            return InterpolatedValueForTotalWind(
                theTime, theMaxMinuteRange, gpoint, xShift, yShift);

          return theValue;
        }
        else
        {
          // tämä liikkuminen on melkoista viritystä
          MoveLeft(xShift);
          MoveDown(yShift);
          float bottomLeftValue = InterpolatedValue(theTime, theMaxMinuteRange);
          MoveRight(1);
          float bottomRightValue = InterpolatedValue(theTime, theMaxMinuteRange);
          MoveUp(1);
          float topRightValue = InterpolatedValue(theTime, theMaxMinuteRange);
          MoveLeft(1);
          float topLeftValue = InterpolatedValue(theTime, theMaxMinuteRange);

          if (param == kFmiWindDirection)
          {  // tehdään wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
            Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
            float tlWS = InterpolatedValue(theTime, theMaxMinuteRange);  // aloitetaan top-leftistä
                                                                         // ja haetaan kaikki nurkat
                                                                         // move-komennoilla
            MoveRight(1);
            float trWS = InterpolatedValue(theTime, theMaxMinuteRange);  // top-right
            MoveDown(1);
            float brWS = InterpolatedValue(theTime, theMaxMinuteRange);  // bottom-right
            MoveLeft(1);
            float blWS = InterpolatedValue(theTime, theMaxMinuteRange);  // bottom-left
            Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
            double dx = gpoint.X() - floor(gpoint.X());
            double dy = gpoint.Y() - floor(gpoint.Y());
            NFmiInterpolation::WindInterpolator windInterpolator;
            windInterpolator.operator()(blWS, bottomLeftValue, (1 - dx) * (1 - dy));
            windInterpolator.operator()(brWS, bottomRightValue, dx *(1 - dy));
            windInterpolator.operator()(trWS, topRightValue, dx *dy);
            windInterpolator.operator()(tlWS, topLeftValue, (1 - dx) * dy);

            theValue = static_cast<float>(windInterpolator.Direction());  // meitä kiinnostaa vain
            // tuulen suunta (tämä pitää
            // tehdä jotenkin fiksummin)
          }
          else if (param == kFmiWaveDirection)
          {
            theValue =
                static_cast<float>(NFmiInterpolation::ModBiLinear(gpoint.X() - floor(gpoint.X()),
                                                                  gpoint.Y() - floor(gpoint.Y()),
                                                                  topLeftValue,
                                                                  topRightValue,
                                                                  bottomLeftValue,
                                                                  bottomRightValue));
          }
          else if (param == kFmiWindVectorMS)
          {
            theValue =
                static_cast<float>(NFmiInterpolation::WindVector(gpoint.X() - floor(gpoint.X()),
                                                                 gpoint.Y() - floor(gpoint.Y()),
                                                                 topLeftValue,
                                                                 topRightValue,
                                                                 bottomLeftValue,
                                                                 bottomRightValue));
          }
          else
          {
            if (interp == kNearestNonMissing)
            {
              theValue = static_cast<float>(
                  NFmiInterpolation::NearestNonMissing(gpoint.X() - floor(gpoint.X()),
                                                       gpoint.Y() - floor(gpoint.Y()),
                                                       topLeftValue,
                                                       topRightValue,
                                                       bottomLeftValue,
                                                       bottomRightValue));
            }
            else
            {
              theValue =
                  static_cast<float>(NFmiInterpolation::BiLinear(gpoint.X() - floor(gpoint.X()),
                                                                 gpoint.Y() - floor(gpoint.Y()),
                                                                 topLeftValue,
                                                                 topRightValue,
                                                                 bottomLeftValue,
                                                                 bottomRightValue));
            }
          }
          return theValue;
        }
      }
    }
    Location(theLatLonPoint);
    return InterpolatedValue(theTime, theMaxMinuteRange);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 * \bug Does not work correctly in all situations
 */
// ----------------------------------------------------------------------

// ei toimi kaikissa tilanteissa oikein, parempi versio fastqueryinfossa

float NFmiQueryInfo::PeekValue(int theTimeOffset, int theXOffset, int theYOffset)
{
  try
  {
    unsigned long paramIndex = itsParamDescriptor->Index();
    unsigned long timeIndex = itsTimeDescriptor->Index() + theTimeOffset;
    unsigned long theVPlaceIndex = itsVPlaceDescriptor->Index();
    unsigned long theHPlaceIndex = itsHPlaceDescriptor->Index();

    if (IsGrid())
      theHPlaceIndex = theHPlaceIndex + theYOffset * Grid()->XNumber() + theXOffset;
    else
      theHPlaceIndex = theHPlaceIndex + theXOffset;

    unsigned long idx =
        ((paramIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
             (itsTimeDescriptor->Size()) +
         (theHPlaceIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
         theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
    return PeekValue(idx);
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

long NFmiQueryInfo::CalcAreaUnCertainty()
{
  try
  {
    double tmpValue = itsAreaUnCertaintyStart + (itsAreaUnCertaintyEnd - itsAreaUnCertaintyStart) *
                                                    TimeIndex() / static_cast<double>(SizeTimes());
    return static_cast<long>(round(tmpValue));  // käytä oikeasti näitä
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

long NFmiQueryInfo::CalcTimeUnCertainty()
{
  try
  {
    return itsTimeUnCertaintyStart;  // käytä oikeasti näitä
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newMask Undocumented
 * \param useNewMask Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiQueryInfo::StaticDataMask(const NFmiBitMask & newMask, bool useNewMask)
{
  delete itsStaticDataMask;
  itsStaticDataMask = new NFmiBitMask(newMask);
  fUseStaticDataMask = useNewMask;
}
*/
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
const NFmiBitMask & NFmiQueryInfo::StaticDataMask()
{
  return *itsStaticDataMask;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiQueryInfo::UseStaticMask(bool newValue)
{
  fUseStaticDataMask = newValue;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param newProducer Undocumented
 */
// ----------------------------------------------------------------------

// vaihtaa tuottajan kaikkiin parametreihin niin infossa kuin querydatassa

void NFmiQueryInfo::SetProducer(const NFmiProducer &newProducer)
{
  try
  {
    if (itsParamDescriptor)
      itsParamDescriptor->SetProducer(newProducer);

    if (RefQueryData())
      RefQueryData()->Info()->SetProducer(newProducer);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theReplaceData Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

// 12.6.2000/Pena Funktio laittaa this-infoon parametrina annetun datan sille aikavälille,
// mikä on annettu parametrina. Jos timeBagia ei anneta, täytetään this-info niin paljon kuin
// parametrina annettua dataa riittää.

bool NFmiQueryInfo::ReplaceData(NFmiQueryInfo *theReplaceData, NFmiTimeBag *theTimeBag)
{
  try
  {
    if (!theReplaceData)
      return false;

    NFmiMetTime TheFirstTime;
    NFmiMetTime TheLastTime;

    First();
    theReplaceData->First();

    if (!theTimeBag)
    {
      TheFirstTime = theReplaceData->TimeDescriptor().FirstTime();
      TheLastTime = theReplaceData->TimeDescriptor().LastTime();
    }
    else
    {
      TheFirstTime = theTimeBag->FirstTime();
      TheLastTime = theTimeBag->LastTime();
    }

    float Float2Value;

    First();
    theReplaceData->First();

    theReplaceData->ResetLevel();
    ResetLevel();

    while (NextLevel())
    {
      if (!theReplaceData->Level(*Level()))
        continue;
      ResetTime();
      while (NextTime() && TheLastTime.DifferenceInHours(ValidTime()) >= 0)
      {
        if (TheFirstTime.DifferenceInHours(ValidTime()) <= 0)
        {
          if (!theReplaceData->Time(Time()))
            continue;

          ResetParam();

          while (NextParam())
          {
            if (!theReplaceData->Param(Param()))
              if (!theReplaceData->Param(*Param().GetParam()))  // 18.8.2000/Marko lisäsi, koska
                                                                // tuottaja ei saa vaikuttaa tässä
                continue;

            ResetLocation();

            while (NextLocation())
            {
              if (theReplaceData->IsGrid())
              {
                Float2Value = theReplaceData->InterpolatedValue(LatLon());
                if (Float2Value != kFloatMissing)
                {
                  FloatValue(Float2Value);
                }
              }
              else
              {
                if (theReplaceData->Location(*Location()))
                {
                  if (theReplaceData->FloatValue() != kFloatMissing)
                  {
                    FloatValue(theReplaceData->FloatValue());
                  }
                }
              }
            }
          }
        }  //  if TheFirstTime
      }    // next Time
    }      // next Level
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
bool NFmiQueryInfo::IsLocationMasked(unsigned long theLocationIndex) const
{
  return !fUseStaticDataMask ? true : itsStaticDataMask->IsMasked(theLocationIndex);
}
*/
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ParamIndex(unsigned long theIndex)
{
  try
  {
    return itsParamDescriptor->Index(theIndex);
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
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::LocationIndex(unsigned long theIndex)
{
  try
  {
    return itsHPlaceDescriptor->Index(theIndex);
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
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::LevelIndex(unsigned long theIndex)
{
  try
  {
    return itsVPlaceDescriptor->Index(theIndex);
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
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::TimeIndex(unsigned long theIndex)
{
  try
  {
    return itsTimeDescriptor->Time(theIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************
// *** HUOM!!! NÄMÄ EIVÄT TOIMI VIELÄ NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
// ****************
// HUOM!! liikkumista ei voi sallia laatikon ulkopuolelle kuten esim. PeekValue-metodissa voi!!!
// Move*** ovat liikkumista varten

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveUp(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveDown(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveLeft(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveRight(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// näillä asetetaan paikka suoraan johonkin laitaan (ei 'laatikon' ulkopuolelle!)

bool NFmiQueryInfo::Top()  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Bottom()  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Left()  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Right()  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// *** HUOM!!! NÄMÄ EIVÄT TOIMI VIELÄ NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
// ****************
// ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************

// ----------------------------------------------------------------------
/*!
 * Etsii 1. halutun avaimen esim.  AVAIN (ei saa laittaa FMI_ etuliitettä!).
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindFirstKey(const NFmiString &theKey)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->FindFirstKey(theKey);

    itsHeaderText->Reset();
    do
    {
      if (IsCurrentKey(theKey))
        return true;
    } while (itsHeaderText->Next());
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindNextKey(const NFmiString &theKey)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->FindNextKey(theKey);

    for (; itsHeaderText->Next();)
    {
      if (IsCurrentKey(theKey))
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
 *  Lisää uuden avaimen theKey=AVAIN ja theValue=ARVO -> FMI_AVAIN=ARVO laitetaan talteen.
 * \param theKey Undocumented
 * \param theValue Undocumented
 * \param fClearDuplicatesFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::AddKey(const NFmiString &theKey,
                           const NFmiString &theValue,
                           bool fClearDuplicatesFirst)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->AddKey(theKey, theValue, fClearDuplicatesFirst);

    if (fClearDuplicatesFirst)
      RemoveAllKeys(theKey);

    NFmiString *keyValueStr = new NFmiString(MakeKeyValueString(theKey, theValue));
    itsHeaderText->Add(keyValueStr);  // stringlist tuhoaa keyValueStr:in
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Jos on löytynyt jokin avain, voi sen arvon asettaa tässä.
 *
 * \param newValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SetCurrentKeyValue(const NFmiString &newValue)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->SetCurrentKeyValue(newValue);

    if (IsCurrentAnyKey())  // pitää tarkistaa ollaako oikeasti minkään avaimen kohdalla ensin
      return ReplaceCurrentKeyValue(newValue);

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Pyydä etsityn avaimen arvo.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::GetCurrentKeyValue()
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->GetCurrentKeyValue();

    if (IsCurrentAnyKey())
      return ExtractCurrentKeyValue();

    NFmiString returnVal;  // palautetaan tyhjää, jos kyseessä ei ole avain
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Poista etsityn avain ja sen arvo, eli poista listasta koko stringi.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveCurrentKey()
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->RemoveCurrentKey();

    if (IsCurrentAnyKey())
      return itsHeaderText->Remove();

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Etsii itsHeaderText:istä kaikki avaimet ja poistaa ne. HUOM! muita ei
 * itsHeaderText otuksesta poisteta kuin avaimet (eli FMI_*= alkuiset stringit).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveAllKeys()
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->RemoveAllKeys();

    if (FindFirstAnyKey())
    {
      do
      {
        // Tämä toinen sisempi luuppi piti laittaa, koska NFmiStringList käyttää voidptrlistaa.
        // En saa voidptrlistasta pois tavaraa, niin että osoitin siirtyisi edelliseen stringiin.
        // Lisäksi jos saisin , ei string-osoitin liikkua alun ohi, eli jos poistettava
        // stringi olisi 1., remove:n jälkeen next heittäisi ohi seuraavan mahd. poistettavan
        // stringin.
        // TEE NFmiStringList käyttäen STL vector:ia!!!!!!!!!!!!!!!!!!!!!!!!!!!
        do
        {
        } while (RemoveCurrentKey());
      } while (FindNextAnyKey());
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
 * Poistaa kaikki halutut avaimet.
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveAllKeys(const NFmiString &theKey)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->RemoveAllKeys(theKey);

    if (FindFirstKey(theKey))
    {
      do
      {
        // Tämä toinen sisempi luuppi piti laittaa, koska NFmiStringList käyttää voidptrlistaa.
        // En saa voidptrlistasta pois tavaraa, niin että osoitin siirtyisi edelliseen stringiin.
        // Lisäksi jos saisin , ei string-osoitin liikkua alun ohi, eli jos poistettava
        // stringi olisi 1., remove:n jälkeen next heittäisi ohi seuraavan mahd. poistettavan
        // stringin.
        // TEE NFmiStringList käyttäen STL vector:ia!!!!!!!!!!!!!!!!!!!!!!!!!!!
        do
        {
        } while (RemoveCurrentKey());
      } while (FindNextKey(theKey));
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
 * Palauttaa listan, joka sisältää kaikki itsHeaderText:in avaimet.
 *
 * \param fRemoveDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiStringList NFmiQueryInfo::GetAllKeys(bool fRemoveDuplicates)
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->GetAllKeys(fRemoveDuplicates);

    NFmiStringList list;
    if (FindFirstAnyKey())
    {
      NFmiString *key = nullptr;
      do
      {
        key = new NFmiString(ExtractCurrentKey());
        if (fRemoveDuplicates && list.Find(*key))
          delete key;
        else
          list.Add(key);

      } while (FindNextAnyKey());
    }
    return list;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Etsinnän jälkeen voi kysyä mikä avain olikaan kyseessä.
 * Palauttaa "" jos ei ole kyseessä avainta.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::GetCurrentKey()
{
  try
  {
    if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, tällöin välitä käsky datalle
      return itsRefQueryData->GetCurrentKey();

    if (IsCurrentAnyKey())
      return ExtractCurrentKey();

    NFmiString returnVal;  // jos current ei ole avain, palauta tyhjä stringi
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Sisäinen apufunktio, joka tekee AVAIN + ARVO -> FMI_AVAIN=ARVO tekstin.
 *
 * \param theKey Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::MakeKeyValueString(const NFmiString &theKey,
                                                   const NFmiString &theValue)
{
  try
  {
    NFmiString returnVal(gInfoHeaderKeyWord);
    returnVal += theKey;
    returnVal += "=";
    returnVal += theValue;
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Etsii ensimmäisen avaimen itsHeaderText:istä. Hakee jokaisen listassa
 * olevan stringin alusta FMI_*= -kuviota (* tarkoittaa mitä tahansa
 * tekstiä, saa olla whitespaceja).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindFirstAnyKey()
{
  try
  {
    itsHeaderText->Reset();
    do
    {
      if (IsCurrentAnyKey())
        return true;
    } while (itsHeaderText->Next());
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Etsii seuraavaa avainta.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindNextAnyKey()
{
  try
  {
    for (; itsHeaderText->Next();)
    {
      if (IsCurrentAnyKey())
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
 * Tarkistaa onko itsHeaderText:in osoittama stringi avain (jotta sille
 * voidaan tehdä operaatioita). Eli onko se jokin avain (FMI_*= ,
 * missä * on sarja merkkejä, saa olla whitespaceja).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsCurrentAnyKey()
{
  try
  {
    NFmiString *currentStr = itsHeaderText->Current();
    if (currentStr)
    {
      NFmiString searchStr(gInfoHeaderKeyWord);
      int pos = 0;
      if ((pos = currentStr->Search(searchStr)) ==
          1)  // tuloksen pitää olla 1 (= etsitty teksti löytyy heti stringin alusta)
      {
        unsigned char test = currentStr->operator[](5);
        if (!isspace(test))  // FMI_ jälkeen ei saa olla whitespacea
        {
          NFmiString searchStr2("=");
          if (currentStr->Search(searchStr2, pos + 2) >
              0)  // +2 tarkoittaa, että etsitään '='-merkkiä FMI_ + yksi merkki jälkeen
            return true;
        }
      }
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
 * Tarkistaa sisältääkö itsHeaderText:in currentti stringi halutun avaimen
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsCurrentKey(const NFmiString &theKey)
{
  try
  {
    NFmiString *currentStr = itsHeaderText->Current();
    if (currentStr)
    {
      NFmiString searchStr(gInfoHeaderKeyWord);
      searchStr += theKey;
      searchStr += "=";
      if (currentStr->Search(searchStr) ==
          1)  // tuloksen pitää olla 1 (= etsitty teksti löytyy heti stringin alusta)
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
 * Vaihda currentin stringin avaimen arvo (se onko kyseessä avain,
 * on jo tarkastettu!).
 * \param newValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ReplaceCurrentKeyValue(const NFmiString &newValue)
{
  try
  {
    NFmiString *currentStr = itsHeaderText->Current();
    if (currentStr)  // tämän pitäisi löytyä, koska se on jo tarkistettu!!
    {
      NFmiString replaceStr(gInfoHeaderKeyWord);
      replaceStr += ExtractCurrentKey();
      replaceStr += "=";
      replaceStr += newValue;
      *currentStr = replaceStr;
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
 * Palauttaa currentin stringin avaimen arvon (se onko kyseessä avain,
 * on jo tarkastettu!).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::ExtractCurrentKeyValue()
{
  try
  {
    NFmiString returnVal;
    NFmiString *currentStr = itsHeaderText->Current();
    if (currentStr)  // tämän pitäisi löytyä, koska se on jo tarkistettu!!
    {
      NFmiString searchStr("=");
      int pos = 0;
      if ((pos = currentStr->Search(searchStr)) > 0)
      {
        int valueStrLength = currentStr->GetLen() - pos;
        returnVal = currentStr->GetChars(pos + 1, valueStrLength);
      }
    }
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin stringin avaimen (se onko kyseessä avain, on
 * jo tarkastettu!).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::ExtractCurrentKey()
{
  try
  {
    NFmiString returnVal;
    NFmiString *currentStr = itsHeaderText->Current();
    if (currentStr)  // tämän pitäisi löytyä, koska se on jo tarkistettu!!
    {
      NFmiString searchStr("=");
      int pos = 0;                                     // '='-merkin paikka
      int startPos = gInfoHeaderKeyWord.GetLen() + 1;  // mistä lähdetään etsimään =-merkkiä
      if ((pos = currentStr->Search(searchStr, startPos)) > 0)
      {
        int valueStrLength = pos - startPos;
        returnVal = currentStr->GetChars(startPos, valueStrLength);
      }
    }
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Infon dataa täytetään annetun gridin arvoilla, jos tietyt ehdot
 * täyttyvät: Info hilamuotoista, Hilat samat, area sama.
 * Datan täyttö tapahtuu currenttiin aikaan, parametriin ja leveliin.
 *
 * \param theSource Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Grid2Info(NFmiGrid &theSource)
{
  try
  {
    if (IsGrid() && Grid()->AreGridsIdentical(theSource))
    {
      if (IsParamUsable() && IsLevelUsable() &&
          IsTimeUsable())  // onko info kelattu paikkaan, mihin voi yleensä laittaa dataa
      {
        for (ResetLocation(), theSource.Reset(); NextLocation() && theSource.Next();)
          FloatValue(static_cast<float>(theSource.FloatValue()));
        return true;
      }
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
 * Hakee listan paikkaindeksi/etäisyys metreinä pareja.
 * Listaan haetaan annettua paikkaa lähimmat datapisteet järjestyksessä
 * lähimmästä kauimpaan. Listaan haetaan joko haluttu määrä lähimpiä pisteitä
 * tai hakua voi myös rajoittaa maksimi etäisyydellä. Jos maksimi määräksi
 * laitetaan -1, haetaan paikkoja niin paljon kuin löytyy (maxEtäisyys rajoitus
 * huomiooon ottaen).
 */
// ----------------------------------------------------------------------

std::vector<std::pair<int, double> > NFmiQueryInfo::NearestLocations(
    const NFmiLocation &theLocation, int theMaxWantedLocations, double theMaxDistance) const
{
  try
  {
    return itsHPlaceDescriptor->NearestLocations(
        theLocation, theMaxWantedLocations, theMaxDistance);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \param theMaxWantedLocations Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::vector<std::pair<int, double> > NFmiQueryInfo::NearestLocations(
    const NFmiPoint &theLatLonPoint, int theMaxWantedLocations, double theMaxDistance) const
{
  try
  {
    NFmiLocation location(theLatLonPoint);
    return NearestLocations(location, theMaxWantedLocations, theMaxDistance);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laitoin uudet descriptorien muutos palikat.
// Huom!!! Muutokset sallitaan ainoastaan saman kokoisille descriptoreille.
void NFmiQueryInfo::SetHPlaceDescriptor(const NFmiHPlaceDescriptor &newDesc)
{
  try
  {
    if (!itsHPlaceDescriptor)
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetHPlaceDescriptor - HPlaceDescriptoria ei oltu alustettu.");
    if (newDesc.Size() != Size())
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetHPlaceDescriptor - ei voi asettaa erikokoista descriptoria.");
    else
      *itsHPlaceDescriptor = newDesc;

    if (itsRefQueryData)  // pitää laittaa sisuskalutkin kuntoon!!!
      itsRefQueryData->SetHPlaceDescriptor(newDesc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::SetVPlaceDescriptor(const NFmiVPlaceDescriptor &newDesc)
{
  try
  {
    if (!itsVPlaceDescriptor)
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetVPlaceDescriptor - VPlaceDescriptoria ei oltu alustettu.");
    if (newDesc.Size() == SizeLevels())
      *itsVPlaceDescriptor = newDesc;
    else
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetVPlaceDescriptor - ei voi asettaa erikokoista descriptoria.");

    if (itsRefQueryData)  // pitää laittaa sisuskalutkin kuntoon!!!
      itsRefQueryData->Info()->SetVPlaceDescriptor(newDesc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::SetTimeDescriptor(const NFmiTimeDescriptor &newDesc)
{
  try
  {
    if (!itsTimeDescriptor)
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetTimeDescriptor - TimeDescriptoria ei oltu alustettu.");
    if (newDesc.Size() == SizeTimes())
      *itsTimeDescriptor = newDesc;
    else
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetTimeDescriptor - ei voi asettaa erikokoista descriptoria.");

    if (itsRefQueryData)  // pitää laittaa sisuskalutkin kuntoon!!!
      itsRefQueryData->Info()->SetTimeDescriptor(newDesc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryInfo::SetParamDescriptor(const NFmiParamDescriptor &newDesc)
{
  try
  {
    if (!itsParamDescriptor)
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetParamDescriptor - ParamDescriptoria ei oltu alustettu.");
    if (newDesc.Size() == SizeParams())
      *itsParamDescriptor = newDesc;
    else
      throw Fmi::Exception(
          BCP, "NFmiQueryInfo::SetParamDescriptor - ei voi asettaa erikokoista descriptoria.");

    if (itsRefQueryData)  // pitää laittaa sisuskalutkin kuntoon!!!
      itsRefQueryData->Info()->SetParamDescriptor(newDesc);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiQueryInfo::FindNearestTime(const NFmiMetTime &theTime,
                                    FmiDirection theDirection,
                                    unsigned long theTimeRangeInMinutes)
{
  try
  {
    return itsTimeDescriptor->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if the given coordinate is in the data
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsInside(const NFmiPoint &theLatLon, double theRadius) const
{
  try
  {
    return itsHPlaceDescriptor->IsInside(theLatLon, theRadius);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Retrun true if the given time is inside the data
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsInside(const NFmiMetTime &theTime) const
{
  try
  {
    return TimeDescriptor().IsInside(theTime);
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

const NFmiTimeBag &NFmiQueryInfo::ValidTimes() const
{
  try
  {
    NFmiTimeBag *bag = itsTimeDescriptor->ValidTimeBag();
    if (bag)
      return *bag;

    throw Fmi::Exception(BCP, "Attempt to access timebag from querydata without one would crash");
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

const NFmiParamBag &NFmiQueryInfo::ParamBag() const
{
  try
  {
    NFmiParamBag *bag = itsParamDescriptor->ParamBag();
    if (bag)
      return *bag;

    throw Fmi::Exception(BCP, "Attempt to access parambag from querydata without one would crash");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Hash value for the grid
 */
// ----------------------------------------------------------------------

std::size_t NFmiQueryInfo::GridHashValue() const
{
  try
  {
    return itsHPlaceDescriptor->HashValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
