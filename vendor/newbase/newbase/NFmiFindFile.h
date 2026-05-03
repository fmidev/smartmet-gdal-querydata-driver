// ======================================================================
/*!
 * \file NFmiFindFile.h
 * \brief Interface of class NFmiFindFile
 */
// ======================================================================

#pragma once

#include "NFmiFileString.h"
#include "NFmiGlobals.h"
#include <string>
#ifdef UNIX
#include <glob.h>
#endif

#define ONLY_PATH "*."

//! Undocumented
class NFmiFindFile
{
 public:
  ~NFmiFindFile();
  NFmiFindFile();

  void Path(const NFmiString& thePathName);
  bool Find(const NFmiString& theFileMask = "*.*");
  bool Next();

  const NFmiString FileName();
  const NFmiString PathName();
  const NFmiString PathAndFileName();

 private:
  NFmiFindFile(const NFmiFindFile& theFile);
  NFmiFindFile& operator=(const NFmiFindFile& theFile);

  std::string itsFileName;
  std::string itsFilePath;

  long itsFileFile;

#ifdef UNIX
  unsigned int itsGlobIndex;
  glob_t* itsGlobBuffer;
#endif

};  // class NFmiFindFile

// ======================================================================
