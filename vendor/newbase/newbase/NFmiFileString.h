// ======================================================================
/*!
 * \file NFmiFileString.h
 * \brief Interface of class NFmiFileString
 */
// ======================================================================

#pragma once

#include "NFmiString.h"

//! Undocumented
class NFmiFileString : public NFmiString
{
 public:
  ~NFmiFileString();
  NFmiFileString();
  NFmiFileString(const NFmiFileString& theFileStr);
  NFmiFileString(const NFmiString& theStr);

  NFmiFileString& operator = (const NFmiFileString&);

  const NFmiString Extension() const;
  const NFmiString FileName() const;
  const NFmiString Header() const;
  const NFmiString Path() const;
  const NFmiString Device() const;
  const NFmiString Directory() const;

  bool HasExtension() const;
  bool IsAbsolutePath() const;

  void Extension(const NFmiString& theExtension);
  void FileName(const NFmiString& theExtension);
  void Header(const NFmiString& theExtension);
  void Path(const NFmiString& theExtension);
  void Device(const NFmiString& theExtension);

  void AddDirectory(const NFmiString& theDirectory);
  void DeleteDirectory();
  void ReplaceDirectory(const NFmiString& theDirectory);

  void Extension(const char* theExtension);
  void FileName(const char* theExtension);
  void Header(const char* theExtension);
  void Path(const char* theExtension);
  void Device(const char* theExtension);

  void NormalizeDelimiter();
  void ChangeScandinavian();

  const NFmiString PathWithoutLastDirectory();

};  // class NFmiFileString

// ======================================================================
