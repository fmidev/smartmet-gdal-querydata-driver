// ======================================================================
/*!
 * \file NFmiLogger.h
 * \brief Interface of class NFmiLogger
 */
// ======================================================================

#pragma once

#include "NFmiMetTime.h"
#include <string>

//! Undocumented
class NFmiLogger
{
 public:
  //! loggerissa käytettyjä viestien tasoja
  enum Level
  {
    kNoLevel = 0,
    kImportantInfo = 1,
    kInfo = 2,
    kDebugInfo = 4,
    kMinorWarning = 8,
    kWarning = 16,
    kError = 32,
    kFatalError = 64
  };

  //! loggerissa käytettyjä laitteita
  enum Device
  {
    kNoDevice = 0,
    kFile = 1,
    kGsm = 2,
    kEmail = 4,
    kStdOut = 8,
    kStdErr = 16
  };

  //! loggerissa käytettyjä tiedosto backup systeemin tiedoston vaihto taajuus
  enum Period
  {
    kNoPeriod = 0,
    kHourly =
        1,  // lokitiedostosta tehdään tallenne (renametaan log-file) aina kun tunti on vaihtunut
    kDaily =
        2,  // lokitiedostosta tehdään tallenne (renametaan log-file) aina kun päivä on vaihtunut
    kMonthly =
        4  // lokitiedostosta tehdään tallenne (renametaan log-file) aina kun kuukausi on vaihtunut
  };

  ~NFmiLogger();
  NFmiLogger();
  NFmiLogger(const std::string &theLogFilePath,
             const std::string &theLogFileBaseName,
             bool theUseBackupSystem,
             bool theCleanBackupFiles,
             Period theLoggingPeriod);
  void InitFromSettings(const std::string &theBaseNameSpace);
  void StoreToSettings();
  //  bool Init(const std::string & theFileName);
  //  bool Store(const std::string & theFileName);
  bool LogMessage(const std::string &theMessage, Level theMessageLevel);
  bool ForceLogMessage(const std::string &theMessage, Level theMessageLevel, Device theDevice);
  void AddLevel(Level theAddedLevel);
  void RemoveLevel(Level theRemovedLevel);
  void UsedLoggingLevels(int value);
  int UsedLoggingLevels() const;
  void LogFilePath(const std::string &value);
  const std::string &LogFilePath() const;
  void LogFileBaseName(const std::string &value);
  const std::string &LogFileBaseName() const;
  void Operational(bool value);
  bool Operational() const;
  bool UseLocalTime() const;
  void UseLocalTime(bool newValue);
  void LogFileBackupTimePeriod(Period value);
  Period LogFileBackupTimePeriod() const;
  void UsedLoggingDevices(int value);
  int UsedLoggingDevices() const;
  const std::string &TimeStampStringFormat() const;
  void TimeStampStringFormat(const std::string &newFormat);
  bool IsLoggerInitialized() const { return fLoggerInitialized; }
  const std::string &CurrentLogFileName() const { return itsCurrentLogFileName; }
  bool LogJustMessages() const { return fLogJustMessages; }
  void LogJustMessages(bool newValue) { fLogJustMessages = newValue; }
  bool NewLogFileInUse() { return fNewLogFileInUse; }
  void NewLogFileInUse(bool newState) { fNewLogFileInUse = newState; }

 private:
  void UpdateFileNamesAndPaths(
      bool updatePath);  // tämä päivittää itsAbsolutLogFilePath:in ja itsCurrentLogFileName:n
  void MakeCurrentLogFileName();
  const std::string MakeBackupFileFilter();
  bool CheckBackupSystem(
      void);  // tekee kaiken mahdollisen liittyen mahdollisiin backup systeemeihin
  bool CleanBackupFiles();
  bool LogMessage(const std::string &theMessage, Level theMessageLevel, int theDevices);
  std::string MakeFinalMessage(const std::string &theMessage, Level theMessageLevel);
  bool Log2Gsm(const std::string &theFinalMessage);
  bool Log2File(const std::string &theFinalMessage);
  bool Log2Email(const std::string &theFinalMessage);
  bool Log2StdOut(const std::string &theFinalMessage);
  bool Log2StdErr(const std::string &theFinalMessage);
  bool OpenFile(const std::string &theFullFileName,
                const std::string &thePath,
                std::ofstream &theFile);
  bool IsFileBackUpTime();
  bool IsCleaningTime();
  bool MakeBackupFile();
  bool DirectoryExist(const std::string &thePath);
  bool TooOldBackupFileTimeStamp(std::time_t theFileTimeStamp);

  void GetLastLoggetTimeFromLogFile(
      void);  // kun logger käynnistyy, katsotaan loki-tiedostosta, milloin on viimeksi tehty lokia
  const std::string MakeBackupFileNameTimeStamp();

  // Tähän voidaan biteillä tallettaa eritasoiset käytettävät logattavat viestitasot ks. enum Level.
  // esim. kImportantInfo + kError (2 + 64 = 66)
  int itsUsedLoggingLevels;
  std::string itsLogFilePath;
  std::string itsAbsolutLogFilePath;  // tätä käytetään sitten sisäisesti, eikä olla työhakemiston
                                      // muutosten armoilla
  std::string itsLogFileBaseName;
  std::string itsTimeStampStringFormat;  // tähän tulee tulevaisuudessa hieno formatointi, mutta
                                         // tässä vaiheessa sisältää luvun jolla saadaan
                                         // time-luokalta haluttu time-string esim. 120 =
                                         // YYYYMMDDHHMMSS
  int itsTimeStampStringIntegerFormat;  // tähän lasketaan toistaiseksi ed. format juttu valmiiksi
  // kun vielä käytetään simppeliä integer lukua osoittamaan
  // formaatti
  // Tämä kertoo kuika usein on tarkoitus tehdä log-file backupeja. Tunnin, päivän vai kuukauden
  // välein (ks. enum Period)
  Period itsLogFileBackupTimePeriod;
  NFmiTime itsCurrentTime;  // tästä otetaan aikaleima ja mahd. vertailut backup systeemissä
                            // edelliseen aikaan
  // Tämän avulla on tarkoitus katsoa, pitääkö luoda nykyisestä log-filesta backup-file (ja minkä
  // niminen).
  NFmiTime itsLastLoggedTime;
  // Sanomia voidaa tallettaa moniin laitteisiin. Tähän on laitetta bitteinä talteen, mitä laitteita
  // käytetään (ks. enum Device).
  // esim. kFile (=2), jolloin viestit menevät vain tiedostoon
  int itsUsedLoggingDevices;
  // Loggeri voidaan laittaa pois käytöstä jos halutaan.
  bool fOperational;
  bool fUseLocalTime;     // käytetäänkö kaikissa hommissa lokaali aikaa, vai UTC-aikaa?
  bool fUseBackupSystem;  // Tekeekö luokka myös backup loki-tiedostoja (aikaleimoilla varustettuja)
  bool fCleanBackupFiles;   // siivoaako luokka myös backup loki-tiedostoja
  bool fLoggerInitialized;  // tällä voidaan tarkistaa onko loggeri jo ainakin kerran initialisoitu
  bool fLogJustMessages;  // tämä on jälkeen lisätty moodi joka on defaulttina pois päältä ja se
                          // pitää aina laittaa erikseen päälle (ei konffissa ainakaan vielä).
  // Jos siis tämä on päällä, ei laiteta loki-viestiin mitään aika leimoja, pellkä annettu viesti.
  bool fNewLogFileInUse;  // Säädetyin väliajoin käytetty lokitiedosto laitetaan talteen omalla
                          // nimellään ja luodaan uusi
  // tiedosto lokitusta varten. Kun näin tapahtuu, laitetaan tämä lippu päälle.
  int itsKeepFilesDays;  // kuinka monta päivää talletetaan tiedostoja (jos siivotaan)
  // Riittää kun tämä lasketaan kerran, ja sitten sitä voidaan käyttää jatkossa suoraan. Tai jos
  // loki tiedoston nimestä tulee muuttuva
  // (esim. aikaleima), tähän koostetaan se.
  std::string itsCurrentLogFileName;

  // nämä eri tasoisten sanomien labelit kertovat , mita talletettavaan vietiin tulee esim.
  // 20011003153412: ErrorLabel: Itse viesti tulee tähän.
  std::string itsNoLevelLabel;
  std::string itsImportantInfoLabel;
  std::string itsInfoLabel;
  std::string itsDebugInfoLabel;
  std::string itsMinorWarningLabel;
  std::string itsWarningLabel;
  std::string itsErrorLabel;
  std::string itsFatalErrorLabel;

  std::string itsBaseNameSpace;
};  // class NFMiLogger

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiLogger::UsedLoggingLevels(int value)
{
  itsUsedLoggingLevels = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiLogger::UsedLoggingLevels() const
{
  return itsUsedLoggingLevels;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const std::string &NFmiLogger::LogFilePath() const
{
  return itsLogFilePath;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const std::string &NFmiLogger::LogFileBaseName() const
{
  return itsLogFileBaseName;
}
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiLogger::Operational(bool value)
{
  fOperational = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiLogger::Operational() const
{
  return fOperational;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiLogger::UseLocalTime() const
{
  return fUseLocalTime;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiLogger::UseLocalTime(bool newValue)
{
  fUseLocalTime = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiLogger::LogFileBackupTimePeriod(Period value)
{
  itsLogFileBackupTimePeriod = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLogger::Period NFmiLogger::LogFileBackupTimePeriod() const
{
  return itsLogFileBackupTimePeriod;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiLogger::UsedLoggingDevices(int value)
{
  itsUsedLoggingDevices = value;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiLogger::UsedLoggingDevices() const
{
  return itsUsedLoggingDevices;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const std::string &NFmiLogger::TimeStampStringFormat() const
{
  return itsTimeStampStringFormat;
}

// ======================================================================
