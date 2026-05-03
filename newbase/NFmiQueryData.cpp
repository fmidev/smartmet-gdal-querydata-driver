// ======================================================================
/*!
 * \file NFmiQueryData.cpp
 * \brief Implementation of class NFmiQueryData
 */
// ======================================================================
/*!
 * \class NFmiQueryData
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiVersion.h"

#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiQueryData.h"
#include "NFmiQueryInfo.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiStationBag.h"
#include <macgyver/Exception.h>
#include <macgyver/FileSystem.h>

#include <fcntl.h>
#include <fstream>
#include <ios>

#ifndef UNIX
#include <io.h>
#endif

using namespace std;

// lisää 4MB bufferin streamiin, ja poistuessaan scopessa jättää streamin käytettäväksi, mutta
// bufferoimattomaksi linuxkääntäjillä streamin bufferi pitää vaihtaa ennen tiedoston avaamista,
// joten tämä vekotin ei toimi https://en.cppreference.com/w/cpp/io/basic_filebuf/setbuf
struct BufferGuard
{
  static constexpr const size_t bufSize = 1 << 22;  // 4194304

  std::vector<char> buf;
  std::ios &strm;

  BufferGuard(std::ios &stream) : strm(stream)
  {
    try
    {
      buf = std::vector<char>(bufSize);
      strm.rdbuf()->pubsetbuf(buf.data(), bufSize);
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }
  ~BufferGuard() { strm.rdbuf()->pubsetbuf(nullptr, 0); }
};

// Staattiset versiot querydatan luku/kirjoituksesta, ottavat huomioon mm. VC++:n binääri
// asetuksista.
// Voivat heittää poikkeuksia.

static void ForceBinaryFormatWrite(const NFmiQueryData &queryData)
{
  try
  {
    // binaryformat tarkoittaa queryDatan talletus formaattia, käytä aina binääri formaattia
    // tehokkuus syistä, jos mahdollista.
    queryData.UseBinaryStorage(true);
    if (queryData.InfoVersion() < 6.)  // jos datan infoversio on alle 6, pitää muuttaa 6:ksi
      queryData.InfoVersion(6.);  // Huom! ei voi muuttaa 7:ksi tai ylemmäksi, koska se saattaa
                                  // sekoittaaa yhdistelmäparametrien (W&C ja TotWind) arvot
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryData::Read()
{
  try
  {
    // Muunnetaan "stdin" binääri moodiin --> pystyy lukemaan binääriä
#ifdef _MSC_VER
    int result = ::_setmode(_fileno(stdin), _O_BINARY);
    if (result == -1)
      throw Fmi::Exception(
          BCP, "Error in NFmiQueryData::Read: Could not set standard input into binary mode.");
#endif
    cin >> *this;

    if (!cin.good())
      throw Fmi::Exception(BCP, "Error in NFmiQueryData::Read, while reading standard input.");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryData::Write(bool forceBinaryFormat) const
{
  try
  {
    if (forceBinaryFormat)
      ::ForceBinaryFormatWrite(*this);

      // Asetetaan 'stdout' binääri moodiin --> kirjoittaa binääriä
#ifdef _MSC_VER
    int result = ::_setmode(_fileno(stdout), _O_BINARY);
    if (result == -1)
      throw Fmi::Exception(
          BCP, "Error in NFmiQueryData::WriteCout: Could not set standard input into binary mode.");
#endif

    cout << *this;

    if (!cout.good())
      throw Fmi::Exception(BCP,
                           "Error in NFmiQueryData::Write,while writing to the standard output.");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiQueryData::Write(const std::string &filename, bool forceBinaryFormat) const
{
  try
  {
    if (filename == "-")
      Write(forceBinaryFormat);
    else
    {
      if (forceBinaryFormat)
        ::ForceBinaryFormatWrite(*this);

      ofstream dataFile(filename.c_str(), ios::binary | ios::out);

#ifdef WIN32
      auto bg = BufferGuard(dataFile);
#endif

      if (dataFile)
        dataFile << *this;
      else
        throw Fmi::Exception(
            BCP,
            std::string("Error in NFmiQueryData::Write: Could not write in file '") + filename +
                "'.");

      if (!dataFile.good())
        throw Fmi::Exception(BCP,
                             std::string("Error in NFmiQueryData::Write, while writing to file '") +
                                 filename + "'.");
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

NFmiQueryData::~NFmiQueryData()
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
 * Constructor
 */
// ----------------------------------------------------------------------

NFmiQueryData::NFmiQueryData()
    : itsRawData(nullptr),
      itsFirst(false),
      itsQueryInfo(nullptr),
      itsLatLonCache(),
      itsLatLonCacheFlag(BOOST_ONCE_INIT)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo Undocumented
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryData::NFmiQueryData(const NFmiQueryInfo &theInfo)
    : itsRawData(new NFmiRawData()),
      itsFirst(false),
      itsQueryInfo(new NFmiQueryInfo(theInfo)),
      itsLatLonCache(),
      itsLatLonCacheFlag(BOOST_ONCE_INIT)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theData The object being copied
 */
// ----------------------------------------------------------------------

NFmiQueryData::NFmiQueryData(const NFmiQueryData &theData)
    : itsRawData(new NFmiRawData(*theData.itsRawData)),
      itsFirst(theData.itsFirst),
      itsQueryInfo(new NFmiQueryInfo(*theData.itsQueryInfo)),
      itsLatLonCache(theData.itsLatLonCache),
      itsLatLonCacheFlag(theData.itsLatLonCacheFlag)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from file
 */
// ----------------------------------------------------------------------

NFmiQueryData::NFmiQueryData(const string &thePath, bool theMemoryMapFlag)
    : itsRawData(nullptr),
      itsFirst(false),
      itsQueryInfo(nullptr),
      itsLatLonCacheFlag(BOOST_ONCE_INIT)
{
  try
  {
    // Filename "-" implies standard input

    if (thePath == "-")
    {
      Read();
    }
    else
    {
      // If the string is a directory, scan it for the latest querydata

      const string filename = NFmiFileSystem::FindQueryData(thePath);

      try
      {
        ifstream file(filename.c_str(), ios::in | ios::binary);
        if (!file)
          throw Fmi::Exception(BCP, "Could not open '" + filename + "' for reading");

#ifdef WIN32
        auto bg = BufferGuard(file);
#endif

        itsQueryInfo = new NFmiQueryInfo;

        if (Fmi::is_compressed(filename))
        {
          Fmi::IStream filter(file, filename);
          filter >> *itsQueryInfo;
          itsRawData =
              new NFmiRawData(filter, itsQueryInfo->Size(), itsQueryInfo->DoEndianByteSwap());
          if (!filter.good())
            throw Fmi::Exception(BCP, "Error while reading '" + filename + "'");
        }
        else
        {
          // Olion sisäinen infoversio numero jää itsQueryInfo:on talteen.

          file >> *itsQueryInfo;

          bool use_mmap = theMemoryMapFlag;

          if (itsQueryInfo->InfoVersion() < 6)
            use_mmap = false;

          if (itsQueryInfo->DoEndianByteSwap())
            use_mmap = false;

          if (use_mmap)
          {
            itsRawData = new NFmiRawData(filename, file, itsQueryInfo->Size());
          }
          else
          {
            itsRawData =
                new NFmiRawData(file, itsQueryInfo->Size(), itsQueryInfo->DoEndianByteSwap());
          }

          if (!file.good())
            throw Fmi::Exception(BCP, "Error while reading '" + filename + "'");

          file.close();
        }
      }
      catch (...)
      {
        delete itsQueryInfo;
        delete itsRawData;
        throw;
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
 * \brief Swap contents. Not thread safe!
 */
// ----------------------------------------------------------------------

void NFmiQueryData::swap(NFmiQueryData &theOther)
{
  try
  {
    std::swap(itsRawData, theOther.itsRawData);
    std::swap(itsFirst, theOther.itsFirst);
    std::swap(itsQueryInfo, theOther.itsQueryInfo);
    std::swap(itsLatLonCache, theOther.itsLatLonCache);
    std::swap(itsLatLonCacheFlag, theOther.itsLatLonCacheFlag);
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

void NFmiQueryData::Destroy()
{
  try
  {
    if (itsQueryInfo)
      delete itsQueryInfo;

    if (itsRawData)
      delete itsRawData;
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

bool NFmiQueryData::Init()
{
  try
  {
    bool status = (itsQueryInfo && itsRawData->Init(itsQueryInfo->Size()));
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryData::Init(const NFmiQueryInfo &theInfo)
{
  try
  {
    if (itsQueryInfo)
      delete itsQueryInfo;

    itsQueryInfo = theInfo.Clone();
    if (itsRawData)
      delete itsRawData;

    itsRawData = new NFmiRawData();
    itsRawData->Init(itsQueryInfo->Size());

    return (itsQueryInfo != nullptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize memory mapped querydata
 */
// ----------------------------------------------------------------------

bool NFmiQueryData::Init(const std::string &theHeader,
                         const std::string &theFilename,
                         bool fInitialize)
{
  try
  {
    if (!itsQueryInfo)
      return false;

    return itsRawData->Init(itsQueryInfo->Size(), theHeader, theFilename, fInitialize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an std::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiQueryData *NFmiQueryData::Clone() const
{
  try
  {
    return new NFmiQueryData(*this);
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

void NFmiQueryData::Reset()
{
  try
  {
    itsFirst = false;
    itsQueryInfo->Reset();
    //  itsDataPool->Reset();
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

bool NFmiQueryData::First()
{
  try
  {
    Reset();
    NextParam();
    NextTime();
    NextLocation();
    NextLevel();
    itsFirst = true;
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

bool NFmiQueryData::Next()
{
  try
  {
    if (!itsFirst)
      return First();

    if (NextParam())
      return true;

    ResetParam();
    NextParam();
    if (NextLocation())
      return true;

    ResetLocation();
    NextLocation();
    if (NextTime())
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

const NFmiMetTime NFmiQueryData::Time() const
{
  try
  {
    if (itsQueryInfo->itsTimeDescriptor->IsValidTime())
      return itsQueryInfo->itsTimeDescriptor->ValidTime();

    return itsQueryInfo->itsTimeDescriptor->OriginTime();
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

float NFmiQueryData::FloatValue()
{
  try
  {
    return itsRawData->GetValue(itsQueryInfo->Index());
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

const NFmiGrid &NFmiQueryData::GridInfo()
{
  try
  {
    return *Info()->Grid();
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
 * \param theQueryData The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiQueryData::operator==(NFmiQueryData &theQueryData)
{
  try
  {
    return IsEqual(theQueryData);
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
 * \param theQueryData The object being copied
 * \return The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiQueryData &NFmiQueryData::operator=(const NFmiQueryData &theQueryData)
{
  try
  {
    if (&theQueryData != this)
    {
      Destroy();
      itsQueryInfo = new NFmiQueryInfo(*theQueryData.itsQueryInfo);
      itsRawData = new NFmiRawData(*theQueryData.itsRawData);
      itsFirst = theQueryData.itsFirst;
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryData::Quality() const
{
  try
  {
    size_t totalsize = itsRawData->Size();
    size_t oksize = 0;

    for (size_t index = 0; index < totalsize; index++)
    {
      if (itsRawData->GetValue(index) != kFloatMissing)
        oksize++;
    }

    return oksize * 100.0f / totalsize;
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

const NFmiString NFmiQueryData::Header1() const
{
  try
  {
    itsQueryInfo->ResetText();

    if (itsQueryInfo->NextText())
      return itsQueryInfo->Text();

    return NFmiString();
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

const NFmiString NFmiQueryData::Header2() const
{
  try
  {
    itsQueryInfo->ResetText();

    if (itsQueryInfo->NextText())
    {
      if (itsQueryInfo->NextText())
      {
        return itsQueryInfo->Text();
      }
    }
    return NFmiString();
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

const NFmiString NFmiQueryData::Header3() const
{
  try
  {
    itsQueryInfo->ResetText();

    if (itsQueryInfo->NextText())
    {
      if (itsQueryInfo->NextText())
      {
        if (itsQueryInfo->NextText())
        {
          return itsQueryInfo->Text();
        }
      }
    }
    return NFmiString();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryData::IsEqual(const NFmiQueryData &theQueryData) const
{
  try
  {
    return (itsQueryInfo == theQueryData.itsQueryInfo && itsRawData == theQueryData.itsRawData);
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

std::ostream &NFmiQueryData::Write(std::ostream &file) const
{
  try
  {
    if (InfoVersion() >= 6)
      UseBinaryStorage(true);

    file << *itsQueryInfo;
    itsRawData->Write(file);

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

std::istream &NFmiQueryData::Read(std::istream &file)
{
  try
  {
    // Olion sisäinen infoversio numero jää itsQueryInfo:on talteen.
    itsQueryInfo = new NFmiQueryInfo();
    file >> *itsQueryInfo;

    itsRawData = new NFmiRawData(file, itsQueryInfo->Size(), itsQueryInfo->DoEndianByteSwap());

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// THIS IS NOT THREAD SAFE!!
void NFmiQueryData::SetHPlaceDescriptor(const NFmiHPlaceDescriptor &newDesc)
{
  try
  {
    if ((Info()->HPlaceDescriptor() == newDesc) == false)
    {
      itsQueryInfo->SetHPlaceDescriptor(newDesc);
      MakeLatLonCache();  // tämä alustaa latlon cachen uudestaan
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the latlon cache
 *
 * Note: This must not be done in constructors, since smartmet-server
 * reads thousands of radar files and holding the coordinates
 * for all of them would take > 10 GB.
 */
// ----------------------------------------------------------------------

std::shared_ptr<std::vector<NFmiPoint> > NFmiQueryData::LatLonCache() const
{
  try
  {
    // If not already set by SetLatLonCache, initialize once
    if (!itsLatLonCache)
      boost::call_once(boost::bind(&NFmiQueryData::MakeLatLonCache, this), itsLatLonCacheFlag);

    return itsLatLonCache;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the latlon cache
 *
 * Note: smartmet-server caches the created LatLonCaches and uses this
 *       method to set the same value for all equivalent grids.
 *
 * Note: We assume there is no race condition between LatLonCache()
 *       SetLatLonCache(), code should always call the latter before
 *       allowing anything else to request access to the cache.
 */
// ----------------------------------------------------------------------

void NFmiQueryData::SetLatLonCache(std::shared_ptr<std::vector<NFmiPoint> > newCache)
{
  try
  {
    itsLatLonCache = newCache;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Initialize coordinate cache
 *
 * \return Coordinate cache
 */
// ----------------------------------------------------------------------

void NFmiQueryData::MakeLatLonCache() const
{
  try
  {
    std::shared_ptr<std::vector<NFmiPoint> > tmp = std::make_shared<std::vector<NFmiPoint> >();
    HPlaceDesc()->CreateLatLonCache(*tmp);
    std::atomic_store(&itsLatLonCache, tmp);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Unique hash value for the grid
 *
 * Used by smartmet-server to cache LatLonCache objects and set them
 * to newly read NFmiQueryData using SetLatLonCache so that MakeLatLonCache
 * is not needed and data can be shared between similar objects.
 */
// ----------------------------------------------------------------------

std::size_t NFmiQueryData::GridHashValue() const
{
  try
  {
    return itsQueryInfo->GridHashValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
