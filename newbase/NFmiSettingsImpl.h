// ======================================================================
/*!
 * \file NFmiSettingsImpl.h
 * \brief Interface of class NFmiSettingsImpl
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

//! A generic settings holder
class NFmiSettingsImpl
{
 public:
  static NFmiSettingsImpl& Instance();
  void Clear();
  bool Init(const std::string& theFile = "") const;
  bool Read(const std::string& theFile) const;

  const std::string Value(const std::string& theName, const std::string& theAlternate = "") const;
  const std::string Require(const std::string& theName) const;
  bool IsSet(const std::string& theName) const;

#undef SearchPath  // win32 macro pitää undefinata
  void SearchPath(const std::string& thePath) const;
  void Set(const std::string& theName, const std::string& theValue, bool modifyOnlyExisting) const;
  void Save() const;
  std::string ToString(const std::string& thePrefix = "") const;
  std::vector<std::string> ListChildren(const std::string& thePrefix) const;

 private:
  //! Destruction is private - this is a singleton
  ~NFmiSettingsImpl();

  void InternalSet(const std::string& theName, const std::string& theValue) const;

  //! Default constructor is private - this is a singleton
  NFmiSettingsImpl();

  //! Copy constructor is disabled - this is a singleton
  NFmiSettingsImpl(const NFmiSettingsImpl& theSettings);

  //! Assignment operator is disabled - this is a singleton
  NFmiSettingsImpl& operator=(const NFmiSettingsImpl& theSettings);

  static void Create();
  static void Die();
  static NFmiSettingsImpl* itsInstance;

  //! The type of the actual data
  typedef std::map<std::string, std::string> DataType;

  static bool itIsDestroyed;        //!< Boolean to detect singleton coupling at exit
  mutable std::string itsFilename;  //!< The file containing the configuration
  mutable DataType itsData;         //!< The actual data
  mutable bool itIsInitialized;     //!< For detecting initialization

  mutable std::list<std::string> itsSearchPath;

  mutable std::list<std::string> itsNamespaces;

  mutable std::set<std::string> itsExpandedVariables;
  mutable std::set<std::string> itsChangedVariables;

  typedef std::map<std::string, std::string> FileMap;
  mutable FileMap itsFilenames;

};  // class NFmiSettingsImpl

// ======================================================================
