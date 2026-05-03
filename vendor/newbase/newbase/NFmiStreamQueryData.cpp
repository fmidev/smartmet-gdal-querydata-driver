// ======================================================================
/*!
 * \file NFmiStreamQueryData.cpp
 * \brief Implementation of class NFmiStreamQueryData
 */
// ======================================================================
/*!
 * \class NFmiStreamQueryData
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiStreamQueryData.h"
#include "NFmiFileSystem.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>
#include <macgyver/FileSystem.h>
#include <cstdio>
#include <fcntl.h>
#include <fstream>

#ifndef UNIX
#include <io.h>
#endif

using namespace std;

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * \brief Local function for finding newest .sqd or .fqd file
 *
 * This was made by copying NFmiFileSystem::NewestFile and
 * making the necessary modifications.
 */
// ----------------------------------------------------------------------

const string find_newest_querydata(const string &thePath)
{
  try
  {
    if (!NFmiFileSystem::DirectoryExists(thePath))
      return "";

    list<string> files = NFmiFileSystem::DirectoryFiles(thePath);
    if (files.empty())
      return "";

    string newestfile;
    time_t newesttime = 0;

    for (list<string>::const_iterator f = files.begin(); f != files.end(); ++f)
    {
      string filename = thePath + '/' + *f;
      if (NFmiFileSystem::FileReadable(filename))
      {
        string suffix = NFmiStringTools::Suffix(*f);
        NFmiStringTools::LowerCase(suffix);

        if (suffix == "sqd" || suffix == "fqd")
        {
          time_t modtime = NFmiFileSystem::FileModificationTime(filename);
          if (modtime > newesttime)
          {
            newesttime = modtime;
            newestfile = *f;
          }
        }
      }
    }

    return newestfile;
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

NFmiStreamQueryData::~NFmiStreamQueryData()
{
  try
  {
    if (!itsOwnerData && itsQueryData)
      delete itsQueryData;

    if (itsQueryDataIter)
      delete itsQueryDataIter;
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

NFmiStreamQueryData::NFmiStreamQueryData()
    : itsQueryData(nullptr), itsQueryDataIter(nullptr), itsOwnerData(false)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theQueryData Undocumented
 */
// ----------------------------------------------------------------------

NFmiStreamQueryData::NFmiStreamQueryData(NFmiQueryData *theQueryData, bool isOwnerData)
    : itsQueryData(theQueryData), itsQueryDataIter(nullptr), itsOwnerData(isOwnerData)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo *NFmiStreamQueryData::QueryInfoIter()
{
  try
  {
    if (itsQueryData)
    {
      if (!itsQueryDataIter)
      {
        itsQueryDataIter = new NFmiFastQueryInfo(itsQueryData);
      }
    }

    return itsQueryDataIter;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theOwnerData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryData *NFmiStreamQueryData::QueryData(bool theOwnerData)
{
  try
  {
    NFmiQueryData *tmp = itsQueryData;  // otetaan väliaikaismuuttujaan talteen siltä varalta jos
                                        // omistajuus vaihtuu ja itsQueryData nollataan
    itsOwnerData = theOwnerData;
    if (itsOwnerData)
      itsQueryData = nullptr;  // pitää nollata pointteri, muuten voi tapahtua kauheita jos luetaan
                               // uusi data sisään
    return tmp;
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

bool NFmiStreamQueryData::IsData()
{
  try
  {
    if (itsQueryData)
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
 * \param theFileName Undocumented
 * \param theQueryData Undocumented
 * \param theLibVersion Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::WriteData(const NFmiString &theFileName,
                                    NFmiQueryData *theQueryData,
                                    long theLibVersion) const
{
  try
  {
    auto version = static_cast<unsigned short>(theLibVersion);
    if (version < 5)
      version = 5;

    ofstream dataFile(theFileName, ios::binary | ios::out);
    if (dataFile)
    {
      if (theQueryData)
      {
        theQueryData->UseBinaryStorage(theLibVersion <= 5 ? false : true);
        theQueryData->InfoVersion(version);
        dataFile << *theQueryData;
      }
      else
      {
        if (itsQueryData)
        {
          itsQueryData->UseBinaryStorage(theLibVersion <= 5 ? false : true);
          itsQueryData->InfoVersion(version);
          dataFile << *itsQueryData;
        }
        else
        {
          cerr << "QueryData-object not found" << endl;
          return false;
        }
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

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::ReadLatestData(const NFmiString &theFileName,
                                         NFmiQueryData **theQueryData)
{
  try
  {
    // If the file is a plain file, read it directly
    if (!NFmiFileSystem::DirectoryExists(theFileName.CharPtr()))
      return ReadData(theFileName, theQueryData);

    string newestfile = NFmiFileSystem::NewestFile(theFileName.CharPtr());
    if (newestfile.empty())
      return false;

    string fullname = theFileName.CharPtr();
    fullname += '/';
    fullname += newestfile;

    return ReadData(NFmiString(fullname), theQueryData);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::SafeReadLatestData(const NFmiString &theFileName,
                                             NFmiQueryData **theQueryData)
{
  try
  {
    // If the file is a plain file, read it directly
    if (!NFmiFileSystem::DirectoryExists(theFileName.CharPtr()))
      return ReadData(theFileName, theQueryData);

    string newestfile = find_newest_querydata(theFileName.CharPtr());
    if (newestfile.empty())
      return false;

    string fullname = theFileName.CharPtr();
    fullname += '/';
    fullname += newestfile;

    return ReadData(NFmiString(fullname), theQueryData);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::ReadData(const NFmiString &theFileName, NFmiQueryData **theQueryData)
{
  try
  {
    if (theFileName ==
        NFmiString(""))  // pitää tarkistaa, ettei tyhjä stringi, muuten kaatuu open:issa
      return false;

    ifstream rawDataFile;

    rawDataFile.open(theFileName, ios::in | ios::binary);
    if (!rawDataFile)
    {
      cerr << "File not found: '" << theFileName.CharPtr() << "'" << endl;
      return false;
    }

    Fmi::IStream dataFile(rawDataFile, theFileName.CharPtr());

    NFmiQueryData *theTempData = static_cast<NFmiQueryData *>(new NFmiQueryData);

    try
    {
      dataFile >> *theTempData;
    }
    // TODO tämä poikkeus käsittely on surkea surkea, koska se tunkee tekstiä vain cerr:iin.
    // Pitäisi tehdä fiksummin (esim. heittää runtime-poikkeus), mutta uskaltaako muuttaa enää tätä
    // toiminnallisuutta?
    catch (char *msg)
    {
      cerr << msg << endl;
      cerr << "Could not open file: " << static_cast<char *>(theFileName) << " for reading."
           << endl;
      delete theTempData;  // siivotaan jäljet kun ongelmia tuli
      theTempData = nullptr;
      return false;
    }
#ifdef FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED
    catch (double eDataMBSize)
    {
      // tee metEditori spesifinen virheilmoitus!!!
      std::string errStr("SmartMet: cannot create large enough continuous array (");
      errStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(eDataMBSize, 1);
      errStr += " MB) for wanted data.";
      delete theTempData;  // siivotaan jäljet kun ongelmia tuli
      theTempData = 0;
      throw Fmi::Exception(BCP, errStr);
    }
#endif  // FMI_MET_EDITOR_CONTINUOIS_MEMORY_ALLOC_FAILED
    catch (...)
    {
      delete theTempData;  // siivotaan jäljet kun ongelmia tuli
      theTempData = nullptr;
      throw;
    }

    itsOwnerData = false;

    delete itsQueryData;
    delete itsQueryDataIter;

    itsQueryData = nullptr;
    itsQueryDataIter = nullptr;

    if (theQueryData)
    {
      itsOwnerData = true;
      *theQueryData =
          theTempData;  // Data ja sen omistus siirtyy argumenttina annettuun qdata-pointteriin.
    }
    else
    {
      itsQueryData = theTempData;
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
 * \param theQueryInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::ReadIn(NFmiQueryInfo *theQueryInfo)
{
  try
  {
#ifndef UNIX
    int result = ::_setmode(_fileno(stdin), _O_BINARY);
    if (result == -1)
    {
      cerr << "Could not set standard input into binary mode!";
      return false;
    }
#endif

    try
    {
      cin >> *theQueryInfo;
    }
    catch (char *msg)
    {
      cerr << msg << endl;
      return false;
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
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::ReadIn(NFmiQueryData *theQueryData)
{
  try
  {
    // Muunnetaan "stdin" binääri moodiin --> pystyy lukemaan binääriä
#ifndef UNIX
    int result = ::_setmode(_fileno(stdin), _O_BINARY);
    if (result == -1)
    {
      cerr << "Could not set standard input into binary mode!";
      return false;
    }
#endif

    NFmiQueryData *theTempData = static_cast<NFmiQueryData *>(new NFmiQueryData);

    try
    {
      cin >> *theTempData;
    }
    catch (char *msg)
    {
      delete theTempData;
      cerr << msg << endl;
      return false;
    }

    itsOwnerData = false;

    delete itsQueryData;
    delete itsQueryDataIter;
    itsQueryData = nullptr;
    itsQueryDataIter = nullptr;

    if (theQueryData)
    {
      itsOwnerData = true;
      theQueryData = theTempData;
    }
    else
    {
      itsQueryData = theTempData;
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
 * \param theQueryData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStreamQueryData::WriteCout(NFmiQueryData *theQueryData) const
{
  try
  {
    NFmiQueryData *tempData = theQueryData ? theQueryData : itsQueryData;
    tempData->UseBinaryStorage(true);
    if (tempData->InfoVersion() < 6.)
      tempData->InfoVersion(DefaultFmiInfoVersion);

      // Asetetaan 'stdout' binääri moodiin --> kirjoittaa binääriä
#ifndef UNIX
    int result = ::_setmode(_fileno(stdout), _O_BINARY);
    if (result == -1)
      cerr << "Could not set standard input into binary mode!";
#endif

    cout << *tempData;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
