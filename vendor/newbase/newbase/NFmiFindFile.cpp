// ======================================================================
/*!
 * \file NFmiFindFile.cpp
 * \brief Implementation of class NFmiFindFile
 */
// ======================================================================
/*!
 * \class NFmiFindFile
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiFindFile.h"
#include <macgyver/Exception.h>

#ifndef UNIX
#include <io.h>
#endif

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiFindFile::~NFmiFindFile()
{
  try
  {
#ifdef UNIX
    if (itsGlobBuffer)
      globfree(itsGlobBuffer);
#else
    _findclose(itsFileFile);
#endif
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

NFmiFindFile::NFmiFindFile()
    : itsFileName(),
      itsFilePath(),
      itsFileFile(0)
#ifdef UNIX
      ,
      itsGlobIndex(0),
      itsGlobBuffer(nullptr)
#endif
{
}

// ----------------------------------------------------------------------
/*!
 * \param thePathName Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFindFile::Path(const NFmiString& thePathName)
{
  itsFilePath = thePathName;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFindFile::FileName()
{
  return itsFileName;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFindFile::PathName()
{
  return itsFilePath;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFindFile::PathAndFileName()
{
  try
  {
    std::string thePathAndFileName(itsFilePath);

    thePathAndFileName += kFmiDirectorySeparator;
    thePathAndFileName += itsFileName;

    return thePathAndFileName;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileMask Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFindFile::Find(const NFmiString& theFileMask)
{
  try
  {
    if (theFileMask.GetLen() == 0)
      return false;

#ifndef UNIX
    struct _finddata_t c_file;
#endif

    NFmiFileString theFindFileName;
    theFindFileName.Path(itsFilePath.c_str());
    theFindFileName.FileName(theFileMask);

#ifdef UNIX
    itsGlobIndex = 0;
    if (itsGlobBuffer)
      globfree(itsGlobBuffer);
    itsGlobBuffer = new glob_t;
    glob(theFindFileName, 0, nullptr, itsGlobBuffer);
    if (itsGlobBuffer->gl_pathc == 0)
    {
      globfree(itsGlobBuffer);
      itsGlobBuffer = nullptr;
      return false;
    }
    if (std::string(".") == std::string(itsGlobBuffer->gl_pathv[itsGlobIndex]))
    {
      Next();
      Next();
      return true;
    }
    // Koko tiedostonimi polkuineen
    itsFileName = itsGlobBuffer->gl_pathv[itsGlobIndex];
    // Ilman hakupolkua
    itsFileName = itsFileName.substr(itsFilePath.size(), itsFileName.size() - itsFilePath.size());
    if (itsFileName.size() > 0 && itsFileName[0] == '/')
      itsFileName = itsFileName.substr(1, itsFileName.size() - 1);

#else

    if ((itsFileFile = _findfirst(theFindFileName, &c_file)) == -1L)
    {
      _findclose(itsFileFile);
      return false;
    }
    if (std::string(".") == std::string(c_file.name))
    {
      Next();
      Next();
      return true;
    }
    itsFileName = c_file.name;
#endif

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

bool NFmiFindFile::Next()
{
  try
  {
#ifdef UNIX
    if (!itsGlobBuffer)
      return false;
    if (++itsGlobIndex <= itsGlobBuffer->gl_pathc)
    {
      itsFileName = itsGlobBuffer->gl_pathv[itsGlobIndex];
      return true;
    }
    else
    {
      globfree(itsGlobBuffer);
      itsGlobBuffer = nullptr;
      return false;
    }
#else
    struct _finddata_t c_file;
    while (!_findnext(itsFileFile, &c_file))
    {
      itsFileName = c_file.name;
      return true;
    }
    _findclose(itsFileFile);
    return false;
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
