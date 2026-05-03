// ======================================================================
/*!
 * \file NFmiLogger.cpp
 * \brief Implementation of class NFmiLogger
 */
// ======================================================================
/*!
 * \class NFmiLogger
 *
 * Luokka joka tallettaa aikaleimattuja viestejä esim. tiedostoon. Voidaan
 * myös käyttää muitakin muotoja, esim. email ja gsm-viesti. Loggerille
 * voidaan asettaa halutunlainen status  filtteri, joka määrittelee,
 * tarvitseeko kulloistakin viestiä logata. Asetustiedot luetaa Init
 * komennossa ja talletetaan Store komennolla tiedostoon.
 *
 */
// ======================================================================

#include "NFmiLogger.h"
#include "NFmiFileSystem.h"
#include "NFmiSettings.h"
#include <macgyver/Exception.h>

#include <fstream>
#ifdef UNIX
#include <unistd.h>  // for getcwd
#endif

#include <cstdlib>
// atoi()

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLogger::~NFmiLogger() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 *
 * \todo Use a proper constant for itsTimeStampStringIntegerFormat
 */
// ----------------------------------------------------------------------

NFmiLogger::NFmiLogger()
    : itsUsedLoggingLevels(127),
      itsLogFilePath(),
      itsAbsolutLogFilePath(),
      itsLogFileBaseName("log.txt"),
      itsTimeStampStringFormat("126"),
      itsTimeStampStringIntegerFormat(126)  // 126 = YYYYMMDDHHmmss ks. NFmiStaticTime.h
      ,
      itsLogFileBackupTimePeriod(kMonthly),
      itsCurrentTime(),
      itsLastLoggedTime(),
      itsUsedLoggingDevices(kFile),
      fOperational(true),
      fUseLocalTime(true),
      fUseBackupSystem(false),
      fCleanBackupFiles(false),
      fLoggerInitialized(false),
      fLogJustMessages(false),
      fNewLogFileInUse(false),
      itsKeepFilesDays(30),
      itsCurrentLogFileName(),
      itsNoLevelLabel("NoLevel"),
      itsImportantInfoLabel("ImportantInfo"),
      itsInfoLabel("Info"),
      itsDebugInfoLabel("DebugInfo"),
      itsMinorWarningLabel("MinorWarning"),
      itsWarningLabel("Warning"),
      itsErrorLabel("Error"),
      itsFatalErrorLabel("FatalError"),
      itsBaseNameSpace()
{
  try
  {
    TimeStampStringFormat(
        itsTimeStampStringFormat);  // asettaa itsTimeStampStringIntegerFormat oikein
    UpdateFileNamesAndPaths(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiLogger::NFmiLogger(const std::string &theLogFilePath,
                       const std::string &theLogFileBaseName,
                       bool theUseBackupSystem,
                       bool theCleanBackupFiles,
                       Period theLoggingPeriod)
    : itsUsedLoggingLevels(127),
      itsLogFilePath(theLogFilePath),
      itsAbsolutLogFilePath(),
      itsLogFileBaseName(theLogFileBaseName),
      itsTimeStampStringFormat("126"),
      itsTimeStampStringIntegerFormat(126)  // 126 = YYYYMMDDHHmmss ks. NFmiStaticTime.h
      ,
      itsLogFileBackupTimePeriod(theLoggingPeriod),
      itsCurrentTime(),
      itsLastLoggedTime(),
      itsUsedLoggingDevices(kFile),
      fOperational(true),
      fUseLocalTime(true),
      fUseBackupSystem(theUseBackupSystem),
      fCleanBackupFiles(theCleanBackupFiles),
      fLoggerInitialized(false),
      fLogJustMessages(false),
      fNewLogFileInUse(false),
      itsKeepFilesDays(30),
      itsCurrentLogFileName(),
      itsNoLevelLabel("NoLevel"),
      itsImportantInfoLabel("ImportantInfo"),
      itsInfoLabel("Info"),
      itsDebugInfoLabel("DebugInfo"),
      itsMinorWarningLabel("MinorWarning"),
      itsWarningLabel("Warning"),
      itsErrorLabel("Error"),
      itsFatalErrorLabel("FatalError"),
      itsBaseNameSpace()
{
  try
  {
    TimeStampStringFormat(
        itsTimeStampStringFormat);  // asettaa itsTimeStampStringIntegerFormat oikein
    UpdateFileNamesAndPaths(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiLogger::InitFromSettings(const std::string &theBaseNameSpace)
{
  try
  {
    itsBaseNameSpace = theBaseNameSpace;

    itsUsedLoggingLevels = NFmiSettings::Require<int>(itsBaseNameSpace + "::UsedLoggingLevels");
    itsLogFilePath = NFmiSettings::Require<string>(itsBaseNameSpace + "::LogFilePath");
    itsLogFileBaseName = NFmiSettings::Require<string>(itsBaseNameSpace + "::LogFileBaseName");
    itsTimeStampStringFormat =
        NFmiSettings::Require<string>(itsBaseNameSpace + "::TimeStampStringFormat");
    TimeStampStringFormat(itsTimeStampStringFormat);  // tämä pitää kutsua!
    itsLogFileBackupTimePeriod = static_cast<Period>(
        NFmiSettings::Require<int>(itsBaseNameSpace + "::LogFileBackupTimePeriod"));
    itsUsedLoggingDevices = NFmiSettings::Require<int>(itsBaseNameSpace + "::UsedLoggingDevices");
    fOperational = NFmiSettings::Require<bool>(itsBaseNameSpace + "::Operational");
    fUseLocalTime = NFmiSettings::Require<bool>(itsBaseNameSpace + "::UseLocalTime");
    fUseBackupSystem = NFmiSettings::Require<bool>(itsBaseNameSpace + "::UseBackupSystem");
    fCleanBackupFiles = NFmiSettings::Require<bool>(itsBaseNameSpace + "::CleanBackupFiles");
    itsKeepFilesDays = NFmiSettings::Require<int>(itsBaseNameSpace + "::KeepFilesDays");
    itsNoLevelLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::NoLevelLabel");
    itsImportantInfoLabel =
        NFmiSettings::Require<string>(itsBaseNameSpace + "::ImportantInfoLabel");
    itsInfoLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::InfoLabel");
    itsDebugInfoLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::DebugInfoLabel");
    itsMinorWarningLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::MinorWarningLabel");
    itsWarningLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::WarningLabel");
    itsErrorLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::ErrorLabel");
    itsFatalErrorLabel = NFmiSettings::Require<string>(itsBaseNameSpace + "::FatalErrorLabel");

    UpdateFileNamesAndPaths(true);  // tämä pitää kutsua!
    fLoggerInitialized = true;      // tämä pitää asettaa!
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiLogger::StoreToSettings()
{
  try
  {
    if (itsBaseNameSpace.empty() == false)
    {
      NFmiSettings::Set(itsBaseNameSpace + "::UsedLoggingLevels",
                        NFmiStringTools::Convert(itsUsedLoggingLevels),
                        true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::LogFilePath", NFmiStringTools::Convert(itsLogFilePath), true);
      NFmiSettings::Set(itsBaseNameSpace + "::LogFileBaseName",
                        NFmiStringTools::Convert(itsLogFileBaseName),
                        true);
      NFmiSettings::Set(itsBaseNameSpace + "::TimeStampStringFormat",
                        NFmiStringTools::Convert(itsTimeStampStringFormat),
                        true);
      NFmiSettings::Set(itsBaseNameSpace + "::LogFileBackupTimePeriod",
                        NFmiStringTools::Convert(static_cast<int>(itsLogFileBackupTimePeriod)),
                        true);
      NFmiSettings::Set(itsBaseNameSpace + "::UsedLoggingDevices",
                        NFmiStringTools::Convert(itsUsedLoggingDevices),
                        true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::Operational", NFmiStringTools::Convert(fOperational), true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::UseLocalTime", NFmiStringTools::Convert(fUseLocalTime), true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::UseBackupSystem", NFmiStringTools::Convert(fUseBackupSystem), true);
      NFmiSettings::Set(itsBaseNameSpace + "::CleanBackupFiles",
                        NFmiStringTools::Convert(fCleanBackupFiles),
                        true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::KeepFilesDays", NFmiStringTools::Convert(itsKeepFilesDays), true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::NoLevelLabel", NFmiStringTools::Convert(itsNoLevelLabel), true);
      NFmiSettings::Set(itsBaseNameSpace + "::ImportantInfoLabel",
                        NFmiStringTools::Convert(itsImportantInfoLabel),
                        true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::InfoLabel", NFmiStringTools::Convert(itsInfoLabel), true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::DebugInfoLabel", NFmiStringTools::Convert(itsDebugInfoLabel), true);
      NFmiSettings::Set(itsBaseNameSpace + "::MinorWarningLabel",
                        NFmiStringTools::Convert(itsMinorWarningLabel),
                        true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::WarningLabel", NFmiStringTools::Convert(itsWarningLabel), true);
      NFmiSettings::Set(
          itsBaseNameSpace + "::ErrorLabel", NFmiStringTools::Convert(itsErrorLabel), true);
      NFmiSettings::Set(itsBaseNameSpace + "::FatalErrorLabel",
                        NFmiStringTools::Convert(itsFatalErrorLabel),
                        true);
    }

    throw Fmi::Exception(BCP, "Error in NFmiLogger::StoreToSettings, unable to store setting.");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
bool NFmiLogger::Init(const string & theFileName)
{
  ifstream in(theFileName.c_str());
  if(in)
        {
          static const int maxLineSize = 256;
          char tmpStr[maxLineSize+1] = "";
          int tmp=0;
          in >> itsUsedLoggingLevels;
          in.getline(tmpStr, maxLineSize); // pitää lukea edellinen rivi 'loppuun'
          in.getline(tmpStr, maxLineSize);
          itsLogFilePath = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsLogFileBaseName = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsTimeStampStringFormat = tmpStr;
          TimeStampStringFormat(itsTimeStampStringFormat);
          in >> tmp;
          itsLogFileBackupTimePeriod = Period(tmp);
          in >> itsUsedLoggingDevices;
          in >> fOperational;
          in >> fUseLocalTime;
          in >> fUseBackupSystem;
          in >> fCleanBackupFiles;
          in >> itsKeepFilesDays;
          in.getline(tmpStr, maxLineSize); // pitää lukea edellinen rivi 'loppuun'
          in.getline(tmpStr, maxLineSize);
          itsNoLevelLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsImportantInfoLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsInfoLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsDebugInfoLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsMinorWarningLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsWarningLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsErrorLabel = tmpStr;
          in.getline(tmpStr, maxLineSize);
          itsFatalErrorLabel = tmpStr;

          UpdateFileNamesAndPaths(true);
          in.close();
          fLoggerInitialized = true;
          return true;
        }
  return false;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
bool NFmiLogger::Store(const string & theFileName)
{
  ofstream out(theFileName.c_str());
  if(out)
        {
          out << itsUsedLoggingLevels << endl;
          out << itsLogFilePath << endl;
          out << itsLogFileBaseName << endl;
          out << itsTimeStampStringFormat << endl;
          out << static_cast<int>(itsLogFileBackupTimePeriod) << endl;
          out << itsUsedLoggingDevices << endl;
          out << fOperational << endl;
          out << fUseLocalTime << endl;
          out << fUseBackupSystem << endl;
          out << fCleanBackupFiles << endl;
          out << itsKeepFilesDays << endl;
          out << itsNoLevelLabel << endl;
          out << itsImportantInfoLabel << endl;
          out << itsInfoLabel << endl;
          out << itsDebugInfoLabel << endl;
          out << itsMinorWarningLabel << endl;
          out << itsWarningLabel << endl;
          out << itsErrorLabel << endl;
          out << itsFatalErrorLabel << endl;

          out.close();
          return true;
        }
  return false;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param theMessage Undocumented
 * \param theMessageLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::LogMessage(const string &theMessage, Level theMessageLevel)
{
  try
  {
    if (fOperational && (itsUsedLoggingLevels & theMessageLevel) && itsUsedLoggingDevices &&
        itsUsedLoggingDevices != kNoDevice)
    {
      itsCurrentTime = NFmiTime();
      bool status = LogMessage(theMessage, theMessageLevel, itsUsedLoggingDevices);
      itsLastLoggedTime = itsCurrentTime;
      return status;
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
 * \param theMessage Undocumented
 * \param theMessageLevel Undocumented
 * \param theDevices Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::LogMessage(const string &theMessage, Level theMessageLevel, int theDevices)
{
  try
  {
    string finalString;
    if (fLogJustMessages)
      finalString = theMessage;
    else
      finalString = MakeFinalMessage(theMessage, theMessageLevel);

    bool status = true;  // Huom! jos yksikin lähetyksistä epäonnistuu, palautuu false
    if (theDevices & kFile)
      status &= Log2File(finalString);
    if (theDevices & kGsm)
      status &= Log2Gsm(finalString);
    if (theDevices & kEmail)
      status &= Log2Email(finalString);
    if (theDevices & kStdOut)
      status &= Log2StdOut(finalString);
    if (theDevices & kStdErr)
      status &= Log2StdErr(finalString);
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theMessage Undocumented
 * \param theMessageLevel Undocumented
 * \param theDevice Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Tekee halutunlaisen viestin haluttuun laitteeseen vaikka asetukset olisivat miten.
// HUOM!! paitsi jos logger ei ole operational-tilassa!!!!

bool NFmiLogger::ForceLogMessage(const string &theMessage, Level theMessageLevel, Device theDevice)
{
  try
  {
    bool status = true;
    int oldDevice = itsUsedLoggingDevices;
    itsUsedLoggingDevices = theDevice;
    int oldLoggongLevels = itsUsedLoggingLevels;
    itsUsedLoggingLevels = theMessageLevel;

    status = LogMessage(theMessage, theMessageLevel);

    itsUsedLoggingLevels = oldLoggongLevels;
    itsUsedLoggingDevices = oldDevice;
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theAddedLevel Undocumented
 */
// ----------------------------------------------------------------------

// Tämä vaikuttaa käytettyyn message leveliin siten, että voit lisätä
// käyttöön esim warning tason (muitten käytettyjen tasojen lisäksi).

void NFmiLogger::AddLevel(Level theAddedLevel)
{
  try
  {
    itsUsedLoggingLevels |= theAddedLevel;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theRemovedLevel Undocumented
 */
// ----------------------------------------------------------------------

// Tällä voit poistaa käytöstä tietyn sanoma tason talletuksen, siten
// että muut käytetyt tasot jäävät voimaan.

void NFmiLogger::RemoveLevel(Level theRemovedLevel)
{
  try
  {
    itsUsedLoggingLevels ^= theRemovedLevel;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theMessage Undocumented
 * \param theMessageLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

string NFmiLogger::MakeFinalMessage(const string &theMessage, Level theMessageLevel)
{
  try
  {
    string returnVal(fUseLocalTime ? itsCurrentTime.ToStr("YYYY MM.DD HH:mm:SS")
                                   : itsCurrentTime.UTCTime().ToStr("YYYY MM.DD HH:mm:SS"));
    returnVal += ": ";
    switch (theMessageLevel)
    {
      case kNoLevel:  // tähän ei kyllä ikinä pitäisi mennä!!!
        returnVal += itsNoLevelLabel;
        break;
      case kImportantInfo:
        returnVal += itsImportantInfoLabel;
        break;
      case kInfo:
        returnVal += itsInfoLabel;
        break;
      case kDebugInfo:
        returnVal += itsDebugInfoLabel;
        break;
      case kMinorWarning:
        returnVal += itsMinorWarningLabel;
        break;
      case kWarning:
        returnVal += itsWarningLabel;
        break;
      case kError:
        returnVal += itsErrorLabel;
        break;
      case kFatalError:
        returnVal += itsFatalErrorLabel;
        break;
    }

    returnVal += ": ";
    returnVal += theMessage;
    return returnVal;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFinalMessage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::Log2File(const string &theFinalMessage)
{
  try
  {
    CheckBackupSystem();  // tekee kaiken backup-tiedostoihin liittyvän
    ofstream out;
    if (OpenFile(itsCurrentLogFileName, itsAbsolutLogFilePath, out))
    {
      out << theFinalMessage << endl;
      out.close();
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
 * \param theFinalMessage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Vili tai joku osaa varmaan toteuttaa tämän!!!

bool NFmiLogger::Log2Gsm(const string & /* theFinalMessage */)
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theFinalMessage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Vili tai joku osaa varmaan toteuttaa tämän!!!
bool NFmiLogger::Log2Email(const string & /* theFinalMessage */)
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theFinalMessage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::Log2StdOut(const std::string &theFinalMessage)
{
  try
  {
    cout << theFinalMessage << endl;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFinalMessage Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::Log2StdErr(const std::string &theFinalMessage)
{
  try
  {
    cerr << theFinalMessage << endl;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFullFileName Undocumented
 * \param thePath Undocumented
 * \param theFile Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Avaa/luo loki-tiedoston, että siihen voidaan lisätä sanoma.
// Jos tiedostoa+hakemistoa ei ole, luo molemmat. Jos ei onnistu,
// palauttaa false, muuten true.

bool NFmiLogger::OpenFile(const string &theFullFileName, const string &thePath, ofstream &theFile)
{
  try
  {
    theFile.open(theFullFileName.c_str(), ios::out | ios::app);
    if (theFile)
      return true;
    else if (DirectoryExist(
                 thePath))  // hakemisto on, mutta tiedostoa ei voi luoda/avata, palauta false
      return false;
    else  // muuten yritä luoda hakemisto ja sitten tiedosto
    {
      if (NFmiFileSystem::CreateDirectory(thePath))
      {
        theFile.clear();  // asettaa bad-bitin pois päältä, että voidaan yrittää tiedoston luomista
                          // uudestaan (ja kysyä onnistuiko)
        theFile.open(theFullFileName.c_str(), ios::out | ios::app);
        if (theFile)
          return true;
        else
          return false;  // edelleenkään tiedostoa ei saatu luotua
      }
      else
        return false;  // hakemistoa ei saatu luotua
    }
    //  return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param thePath Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::DirectoryExist(const std::string &thePath)
{
  try
  {
    return NFmiFileSystem::FileExists(thePath);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

// kun polku annetaan, otetaan siitä fullpath talteen, että loki osoittaa
// aina samaan paikkaan ajon aikana kun tätä muutetaan pitää paivittää
// myös itsCurrentLogFileName:a
// HUOM!! Tässä pitää ottaa huomioon, että currentti working directory
// on oikeassa paikassa.

void NFmiLogger::LogFilePath(const string &value)
{
  try
  {
    itsLogFilePath = value;
    UpdateFileNamesAndPaths(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLogger::LogFileBaseName(const string &value)
{
  try
  {
    itsLogFileBaseName = value;
    UpdateFileNamesAndPaths(false);  // false=älä updeittaa absoluutti polkua
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newFormat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLogger::TimeStampStringFormat(const string &newFormat)
{
  try
  {
    itsTimeStampStringFormat = newFormat;
    itsTimeStampStringIntegerFormat =
        atoi(itsTimeStampStringFormat
                 .c_str());  // tässä vaiheessa vielä käytetään tätä simppeliä formatointia
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

// tekee kaiken mahdollisen liittyen mahdollisiin backup systeemeihin

bool NFmiLogger::CheckBackupSystem()
{
  try
  {
    if (fUseBackupSystem)
    {
      if (IsFileBackUpTime())
      {
        CleanBackupFiles();
        return MakeBackupFile();
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::CleanBackupFiles()
{
  try
  {
    if (fCleanBackupFiles && IsCleaningTime())
    {
      string fileFilter(MakeBackupFileFilter());
      string foundFileName;
      bool continueLoop = true;
      time_t fileTimeStamp = 0;
      do
      {
        fileTimeStamp = NFmiFileSystem::FindFile(
            fileFilter, false, &foundFileName);  // false=etsi vanhin tiedosto
        if (TooOldBackupFileTimeStamp(fileTimeStamp))
        {
          continueLoop = true;
          string removeFileFullName(itsAbsolutLogFilePath);
          removeFileFullName += foundFileName;
          bool removeStatus = NFmiFileSystem::RemoveFile(removeFileFullName);
          if (!removeStatus)
            break;  // breakataan tässä, ettei jää iki-looppiin, jos ei esim oikeuksia tiedoston
                    // poistoon
        }
        else
          continueLoop = false;
      } while (continueLoop);
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Tarkistaa, onko aika tehdä nykyisestä log-filestä aikaleimattu backup-file

bool NFmiLogger::IsFileBackUpTime()
{
  try
  {
    int diffInHours = itsCurrentTime.DifferenceInHours(itsLastLoggedTime);
    if (fUseLocalTime)
    {
      switch (itsLogFileBackupTimePeriod)
      {
        case kHourly:
          if (itsCurrentTime.GetHour() != itsLastLoggedTime.GetHour() || diffInHours >= 1)
            return true;
          break;
        case kDaily:
          if (itsCurrentTime.GetDay() != itsLastLoggedTime.GetDay() || diffInHours >= 24)
            return true;
          break;
        case kMonthly:
          if (itsCurrentTime.GetMonth() != itsLastLoggedTime.GetMonth() || diffInHours >= 31 * 24)
            return true;
          break;
        case kNoPeriod:
          break;
      }
    }
    else
    {
      switch (itsLogFileBackupTimePeriod)
      {
        case kHourly:
          if (itsCurrentTime.UTCTime().GetHour() != itsLastLoggedTime.UTCTime().GetHour() ||
              diffInHours >= 1)
            return true;
          break;
        case kDaily:
          if (itsCurrentTime.UTCTime().GetDay() != itsLastLoggedTime.UTCTime().GetDay() ||
              diffInHours >= 24)
            return true;
          break;
        case kMonthly:
          if (itsCurrentTime.UTCTime().GetMonth() != itsLastLoggedTime.UTCTime().GetMonth() ||
              diffInHours >= 31 * 24)
            return true;
          break;
        case kNoPeriod:
          break;
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// tehdään siivousta vain kerran päivässä (päivän vaihtuessa)

bool NFmiLogger::IsCleaningTime()
{
  try
  {
    int diffInHours = itsCurrentTime.DifferenceInHours(itsLastLoggedTime);
    if (fUseLocalTime)
    {
      if (itsCurrentTime.GetDay() != itsLastLoggedTime.GetDay() || diffInHours >= 24)
        return true;
    }
    else
    {
      if (itsCurrentTime.UTCTime().GetDay() != itsLastLoggedTime.UTCTime().GetDay() ||
          diffInHours >= 24)
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const string NFmiLogger::MakeBackupFileFilter()
{
  try
  {
    string filter(itsCurrentLogFileName);
    filter += ".*";
    return filter;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileTimeStamp Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::TooOldBackupFileTimeStamp(time_t theFileTimeStamp)
{
  try
  {
    if (theFileTimeStamp)
    {
      NFmiTime t(theFileTimeStamp);
      int diffInDays = itsCurrentTime.DifferenceInDays(t);
      if (diffInDays > itsKeepFilesDays)
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
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLogger::MakeBackupFile()
{
  try
  {
    fNewLogFileInUse = true;
    string fileName(itsCurrentLogFileName);
    fileName += ".";
    fileName += MakeBackupFileNameTimeStamp();
    if (NFmiFileSystem::FileExists(fileName))
      fileName +=
          "_2";  // tämä on sitä varten että jos on useita Metkun editoreita yhtäaikaa käynnissä,
    // on toinen editori voinut jo tehdä backupin ja aloittaa uuden loki tiedoston täytön.
    // Sitten toinen haluaa tehdä saman jo kopioi tehdyn backupin päälle uuden vasta aletun loki
    // tiedoston.
    // Nyt alkuperäinen jää talteen, koska tämä mahdollinen tynkä talletetaan _2 nimi jatkeella.
    return NFmiFileSystem::RenameFile(itsCurrentLogFileName, fileName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 */
// ----------------------------------------------------------------------

// kun logger käynnistyy, katsotaan loki-tiedostosta, milloin on viimeksi tehty lokia
void NFmiLogger::GetLastLoggetTimeFromLogFile()
{
  try
  {
    string tmp;
    time_t fileTimeStamp = NFmiFileSystem::FindFile(
        itsCurrentLogFileName, true, &tmp);  // true=etsi uusin tiedosto (samantekevää tässä)
    NFmiTime t(fileTimeStamp);
    itsLastLoggedTime = t;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 */
// ----------------------------------------------------------------------

void NFmiLogger::MakeCurrentLogFileName()
{
  try
  {
    itsCurrentLogFileName = itsAbsolutLogFilePath;
    if (itsCurrentLogFileName[itsCurrentLogFileName.size() - 1] != kFmiDirectorySeparator)
      itsCurrentLogFileName += kFmiDirectorySeparator;

    itsCurrentLogFileName += itsLogFileBaseName;
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

const string NFmiLogger::MakeBackupFileNameTimeStamp()
{
  try
  {
    string str;

    if (fUseLocalTime)
    {
      switch (itsLogFileBackupTimePeriod)
      {
        case kHourly:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime).ToStr(kYYYYMMDDHH);
          break;
        case kDaily:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime).ToStr(kYYYYMMDD);
          break;
        case kMonthly:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime).ToStr(kLongYear + kMonth);
          break;
        case kNoPeriod:
          break;
      }
    }
    else
    {
      switch (itsLogFileBackupTimePeriod)
      {
        case kHourly:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime.UTCTime()).ToStr(kYYYYMMDDHH);
          break;
        case kDaily:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime.UTCTime()).ToStr(kYYYYMMDD);
          break;
        case kMonthly:
          str = static_cast<NFmiStaticTime>(itsLastLoggedTime.UTCTime()).ToStr(kLongYear + kMonth);
          break;
        case kNoPeriod:
          break;
      }
    }
    return str;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param updatePath Undocumented
 */
// ----------------------------------------------------------------------

// tämä päivittää itsAbsolutLogFilePath:in ja itsCurrentLogFileName:n
// HUOM!! Tässä pitää ottaa huomioon, että currentti working directory on oikeassa paikassa.
// UNIX yhteensopivuus ???
// Pitäisikö fullpath-funktio tehdä NFmiFileSystem:iin??????????

void NFmiLogger::UpdateFileNamesAndPaths(bool updatePath)
{
  try
  {
    if (updatePath)
    {
      static const int pathSize = 512;
      char tmpFullPathStr[pathSize] = "";
#ifndef UNIX
      ::_fullpath(tmpFullPathStr, itsLogFilePath.c_str(), pathSize);  // UNIX????
      itsAbsolutLogFilePath = tmpFullPathStr;
#else
      getcwd(tmpFullPathStr, pathSize);
      itsAbsolutLogFilePath = tmpFullPathStr;
      itsAbsolutLogFilePath += kFmiDirectorySeparator;
      itsAbsolutLogFilePath += itsLogFilePath;
#endif
      if (itsAbsolutLogFilePath[itsAbsolutLogFilePath.size() - 1] != kFmiDirectorySeparator)
        itsAbsolutLogFilePath += kFmiDirectorySeparator;
    }
    MakeCurrentLogFileName();
    GetLastLoggetTimeFromLogFile();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
