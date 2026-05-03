// ======================================================================
/*!
 * \file NFmiQueryData.h
 * \brief Interface of class NFmiQueryData
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiQueryInfo.h"
#include "NFmiRawData.h"
#include "NFmiStation.h"
#include "NFmiStringList.h"
#include <memory>
#include <boost/thread.hpp>
#include <boost/thread/once.hpp>
#include <string>
#include <vector>

class NFmiQueryInfo;

//! Undocumented
class NFmiQueryData
{
 public:
 public:
  void Read();                                      // reads from std::cin (standard input)
  void Write(bool forceBinaryFormat = true) const;  // writes to std::cout (standard output)
  void Write(const std::string &filename, bool forceBinaryFormat = true) const;

  virtual ~NFmiQueryData();

  NFmiQueryData();
  NFmiQueryData(const std::string &thePath, bool theMemoryMapFlag = true);
  NFmiQueryData(const NFmiQueryInfo &theInfo);
  NFmiQueryData(const NFmiQueryData &theData);

  // methods
  bool Init(const NFmiQueryInfo &theInfo);
  bool Init();
  bool Init(const std::string &theHeader, const std::string &theFilename, bool fInitialize);
  void Destroy();

  NFmiQueryInfo *Info() const;

  // Poistetaan heti kun mahdollista
  const NFmiString Header1() const;
  const NFmiString Header2() const;
  const NFmiString Header3() const;
  // Poistetaan heti kun mahdollista

  void Reset();
  void ResetParam();
  void ResetLocation();
  void ResetTime();
  void ResetLevel();

  bool IsLocation() const;
  bool IsArea() const;
  bool IsGrid() const;
  bool IsValidTime() const;
  bool IsOriginTime() const;
  bool IsLevel() const;

  bool First();
  bool Next();

  bool NextActiveParam();
  bool NextActiveLocation();
  bool NextActiveTime();

  bool NextParam();
  bool NextLocation();
  bool NextTime();
  bool NextLevel();
  bool PreviousLevel();

  const NFmiDataIdent Param() const;
  const NFmiLocation Location() const;
  bool Location(const NFmiPoint &theLonLatPoint);
  const NFmiStation Station() const;
  const NFmiMetTime Time() const;
  const NFmiMetTime ValidTime() const;
  const NFmiMetTime OriginTime() const;
  unsigned long ForecastPeriod() const;
  const NFmiLevel Level() const;

  unsigned long ParamSize() const;
  float Quality() const;

  bool Param(NFmiDataIdent theDataIdent);
  bool Location(NFmiLocation theLocation);
  bool Station(NFmiStation theStation);
  bool Time(NFmiMetTime theOriginTime);
  bool Level(NFmiLevel theLevel);

  float FloatValue();

  const NFmiGrid &GridInfo();

  bool operator==(NFmiQueryData &theQueryData);

  NFmiQueryData &operator=(const NFmiQueryData &theQueryData);

  void UseBinaryStorage(bool newState) const;
  bool IsBinaryStorageUsed() const;

  //  virtual const NFmiQueryData Combine(const NFmiQueryData &theCombine);
  virtual NFmiQueryData *Clone() const;
  virtual unsigned long ClassId() const;

  // Avain systeemi sijoitetaan NFmiQueryData:n omistamaan NFmiQueryInfo:on.
  // Nämä on vain datan omistaman sisäisen infon käskyttämistä varten
  // tehtyjä välitys funktioita.

  bool FindFirstKey(const NFmiString &theKey);
  bool FindNextKey(const NFmiString &theKey);
  bool AddKey(const NFmiString &theKey,
              const NFmiString &theValue,
              bool fClearDuplicatesFirst = false);
  bool SetCurrentKeyValue(const NFmiString &newValue);
  const NFmiString GetCurrentKeyValue();
  bool RemoveCurrentKey();
  bool RemoveAllKeys();
  bool RemoveAllKeys(const NFmiString &theKey);

  const NFmiStringList GetAllKeys(bool fRemoveDuplicates = false);
  const NFmiString GetCurrentKey();

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  double InfoVersion() const;
  void InfoVersion(double newValue) const;

  std::shared_ptr<std::vector<NFmiPoint> > LatLonCache() const;
  void SetLatLonCache(std::shared_ptr<std::vector<NFmiPoint> > newCache);

  // Unique value for unique grids
  std::size_t GridHashValue() const;

  // Tämä HPlaceDescriptorin asetus funktio on spesiaali funktio.
  // Sillä on tarkoitus muuttaa iteraattori-infon hilan aluetta
  // eikä muuta siten, että datan koko ei kasva. Jos alkuperäisessä
  // descriptorissa hilan koko oli esim. 30 x 40 ja yrittää asettaa
  // hplaceDescriptoria missä on hilana vaikka 50 x 60, lentää POIKKEUS!
  // Muiden descriptorien asetus funktioita ei tarvinnut tehdä queryData
  // -luokkaan, mutta tämä piti tehdä, koska muuten itsLatLonCache
  // ei päivity.
  // Itse (Marko) käytän tätä systeemiä Metkun editorissa
  // 'huijaamaan' qinfoa, että voin laskea makroparametreja
  // karttanäytölle (smarttool-skriptejä, joita visualisoidaan).

  // THIS IS NOT THREAD SAFE!!
  void SetHPlaceDescriptor(const NFmiHPlaceDescriptor &newDesc);

  void swap(NFmiQueryData &theOther);

 protected:
  NFmiHPlaceDescriptor *HPlaceDesc() const;
  NFmiVPlaceDescriptor *VPlaceDesc() const;
  NFmiParamDescriptor *ParamDesc() const;
  NFmiTimeDescriptor *TimeDesc() const;

  bool IsEqual(const NFmiQueryData &theQueryData) const;

  NFmiRawData *itsRawData;

 private:
  bool itsFirst;
  NFmiQueryInfo *itsQueryInfo;

  void MakeLatLonCache() const;
  mutable std::shared_ptr<std::vector<NFmiPoint> > itsLatLonCache;
  mutable boost::once_flag itsLatLonCacheFlag;

  friend class NFmiQueryInfo;
  friend class NFmiTimeSeriesModelDB;
  friend class NFmiRadarPointDB;

};  // class NFmiQueryData

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiQueryInfo *NFmiQueryData::Info() const
{
  return itsQueryInfo;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::ResetParam()
{
  itsQueryInfo->ResetParam();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::ResetLocation()
{
  itsQueryInfo->ResetLocation();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::ResetTime()
{
  itsQueryInfo->ResetTime();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::ResetLevel()
{
  itsQueryInfo->ResetLevel();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsLocation() const
{
  return (itsQueryInfo->IsLocation());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsArea() const
{
  return (itsQueryInfo->IsArea());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsGrid() const
{
  return (itsQueryInfo->IsGrid());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsValidTime() const
{
  return (itsQueryInfo->IsValidTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsOriginTime() const
{
  return (itsQueryInfo->IsOriginTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsLevel() const
{
  return (itsQueryInfo->IsLevel());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextParam()
{
  return (itsQueryInfo->NextParam());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextLocation()
{
  return (itsQueryInfo->NextLocation());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextTime()
{
  return (itsQueryInfo->NextTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextLevel()
{
  return (itsQueryInfo->NextLevel());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::PreviousLevel()
{
  return (itsQueryInfo->PreviousLevel());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextActiveParam()
{
  return (itsQueryInfo->NextActiveParam());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextActiveLocation()
{
  return (itsQueryInfo->NextActiveLocation());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::NextActiveTime()
{
  return (itsQueryInfo->NextActiveTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiDataIdent NFmiQueryData::Param() const
{
  return (itsQueryInfo->Param());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLocation NFmiQueryData::Location() const
{
  return (*(itsQueryInfo->Location()));
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiStation NFmiQueryData::Station() const
{
  return (*(const_cast<NFmiStation *>(static_cast<const NFmiStation *>(itsQueryInfo->Location()))));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime NFmiQueryData::ValidTime() const
{
  return (itsQueryInfo->ValidTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiMetTime NFmiQueryData::OriginTime() const
{
  return (itsQueryInfo->OriginTime());
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryData::ForecastPeriod() const
{
  return (itsQueryInfo->ForecastPeriod());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLevel NFmiQueryData::Level() const
{
  return (*(itsQueryInfo->Level()));
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryData::ParamSize() const
{
  return itsQueryInfo->itsParamDescriptor->Size();
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Param(NFmiDataIdent theDataIdent)
{
  return (itsQueryInfo->Param(theDataIdent));
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Location(NFmiLocation theLocation)
{
  return (itsQueryInfo->Location(theLocation));
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLatPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Location(const NFmiPoint &theLonLatPoint)
{
  return itsQueryInfo->Location(theLonLatPoint);
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Station(NFmiStation theLocation)
{
  return (itsQueryInfo->Location(theLocation));
}

// ----------------------------------------------------------------------
/*!
 * \param theValidTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Time(NFmiMetTime theValidTime)
{
  return (itsQueryInfo->Time(theValidTime));
}

// ----------------------------------------------------------------------
/*!
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::Level(NFmiLevel theLevel)
{
  return (itsQueryInfo->Level(theLevel));
}
// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::UseBinaryStorage(bool newState) const
{
  itsRawData->SetBinaryStorage(newState);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::IsBinaryStorageUsed() const
{
  return itsRawData->IsBinaryStorageUsed();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryData::ClassId() const
{
  return kNFmiQueryData;
}
// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::FindFirstKey(const NFmiString &theKey)
{
  return itsQueryInfo->FindFirstKey(theKey);
}

// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::FindNextKey(const NFmiString &theKey)
{
  return itsQueryInfo->FindNextKey(theKey);
}

// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \param theValue Undocumented
 * \param fClearDuplicatesFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::AddKey(const NFmiString &theKey,
                                  const NFmiString &theValue,
                                  bool fClearDuplicatesFirst)
{
  return itsQueryInfo->AddKey(theKey, theValue, fClearDuplicatesFirst);
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::SetCurrentKeyValue(const NFmiString &newValue)
{
  return itsQueryInfo->SetCurrentKeyValue(newValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiQueryData::GetCurrentKeyValue()
{
  return itsQueryInfo->GetCurrentKeyValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::RemoveCurrentKey()
{
  return itsQueryInfo->RemoveCurrentKey();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::RemoveAllKeys()
{
  return itsQueryInfo->RemoveAllKeys();
}
// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiQueryData::RemoveAllKeys(const NFmiString &theKey)
{
  return itsQueryInfo->RemoveAllKeys(theKey);
}

// ----------------------------------------------------------------------
/*!
 * \param fRemoveDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiStringList NFmiQueryData::GetAllKeys(bool fRemoveDuplicates)
{
  return itsQueryInfo->GetAllKeys(fRemoveDuplicates);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiQueryData::GetCurrentKey()
{
  return itsQueryInfo->GetCurrentKey();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiQueryData::InfoVersion() const
{
  return itsQueryInfo->InfoVersion();
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiQueryData::InfoVersion(double newValue) const
{
  itsQueryInfo->InfoVersion(newValue);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiHPlaceDescriptor *NFmiQueryData::HPlaceDesc() const
{
  return itsQueryInfo->itsHPlaceDescriptor.get();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiVPlaceDescriptor *NFmiQueryData::VPlaceDesc() const
{
  return itsQueryInfo->itsVPlaceDescriptor.get();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiParamDescriptor *NFmiQueryData::ParamDesc() const
{
  return itsQueryInfo->itsParamDescriptor.get();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiTimeDescriptor *NFmiQueryData::TimeDesc() const
{
  return itsQueryInfo->itsTimeDescriptor.get();
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiQueryData
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiQueryData &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiQueryData
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiQueryData &ob)
{
  return ob.Read(file);
}

// ======================================================================
