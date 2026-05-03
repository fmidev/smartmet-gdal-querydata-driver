// ======================================================================
/*!
 * \file NFmiFileString.cpp
 * \brief Implementation of class NFmiFileString
 */
// ======================================================================
/*!
 * \class NFmiFileString
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiFileString.h"
#include <macgyver/Exception.h>
#include <algorithm>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiFileString::~NFmiFileString() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString() = default;

NFmiFileString& NFmiFileString::operator = (const NFmiFileString&) = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theStr Undocumented
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString(const NFmiString &theStr) : NFmiString(theStr)
{
  try
  {
    NormalizeDelimiter();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString(const NFmiFileString &theFileStr) : NFmiString(theFileStr)
{
  try
  {
    NormalizeDelimiter();
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

bool NFmiFileString::HasExtension() const
{
  try
  {
    unsigned char find[2] = ".";
#ifndef UNIX
    unsigned char slash[2] = "\\";
#else
    unsigned char slash[2] = "/";
#endif

    unsigned long lastSlash = SearchLast(slash);
    return Search(find, std::max(lastSlash, 1ul)) > 0 ? true : false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFileString::IsAbsolutePath() const
{
  try
  {
    if (fChar[0] == '/' || fChar[0] == '\\' || Device() != NFmiString(""))
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

const NFmiString NFmiFileString::Extension() const
{
  try
  {
    unsigned long last;
    unsigned char find[2] = ".";

    last = SearchLast(find);
    if (last)
      return GetChars(last + 1, GetLen() - last);
    else
      return "";  // jos last oli 0, ei löytynyt .-merkkiä  ja ei ole extensiota
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

const NFmiString NFmiFileString::FileName() const
{
  try
  {
    unsigned long last;
    unsigned char find[2] = {kFmiDirectorySeparator, '\0'};

    last = SearchLast(find);

    return GetChars(last + 1, GetLen() - last);
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

const NFmiString NFmiFileString::Header() const
{
  try
  {
    unsigned long first, last;
    unsigned char find1[2] = {kFmiDirectorySeparator, '\0'};
    unsigned char find2[2] = ".";

    first = SearchLast(find1);

    last = HasExtension() ? SearchLast(find2) : 0;
    // last  = SearchLast(find2);

    if (last == 0)
      last = GetLen() + 1;

    return GetChars(first + 1, last - (first + 1));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Ei hoida tilannetta jos filestringin sisältö on:
 *  polku/tiedosto.ext (ilman ./ alkua)
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Path() const
{
  try
  {
    unsigned long first, last;
    unsigned char find[2] = R"(\)";
    unsigned char find2[2] = "/";

    first = Search(find);
    last = SearchLast(find);

    if (first == 0 && last == 0)  // kokeillaan vielä kenoja toisin päin
    {
      first = Search(find2);
      last = SearchLast(find2);
    }

    if (first)
    {
      if (fChar[0] == '.')  // Lasse 23.9.98 suht polku
        return GetChars(1, last);
      else if (IsAbsolutePath() == false)  // suhteellinen polku ilman .-merkkiä alussa
        return GetChars(1, last);
      else
        return GetChars(first, (last + 1) - first);
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

const NFmiString NFmiFileString::Device() const
{
  try
  {
#ifdef UNIX
    return NFmiString();  // linux/unix/mac ei ole devicea, joten palautetaan tyhjä
#else                     // windowsissa on device
    unsigned long last;
    unsigned char find[2] = ":";

    last = Search(find);

    if (last)
      return GetChars(1, last);

    return NFmiString();
#endif
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

const NFmiString NFmiFileString::PathWithoutLastDirectory()
{
  try
  {
    unsigned long first, last, nextLast, pos;
    unsigned char find[2] = {kFmiDirectorySeparator, '\0'};

    first = Search(find);
    last = first;
    nextLast = last;
    pos = Search(find, first + 1);
    while (pos > 0)
    {
      nextLast = last;
      last = pos;
      pos = Search(find, ++pos);
    }

    if (first)
    {
      if (fChar[0] == '.')
        return GetChars(1, nextLast);
      else
        return GetChars(first, (nextLast + 1) - first);
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

const NFmiString NFmiFileString::Directory() const
{
  try
  {
    unsigned long first, last;
    unsigned char backSlash[2] = {kFmiDirectorySeparator, '\0'};

    unsigned long pos = 1;
    pos = Search(backSlash, pos);
    first = pos;
    last = pos;
    pos = Search(backSlash, pos);
    while (pos > 0)
    {
      first = last;
      last = pos;
      pos = Search(backSlash, ++pos);
    }
    if (last > first)
      return GetChars(first + 1, last - (first + 1));
    else
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

void NFmiFileString::NormalizeDelimiter()
{
  try
  {
// 31.8.98 Lehtiohjelmassa on ainakin tarvetta/LW
#ifndef UNIX
    unsigned char slash[2] = "/";
    unsigned char backSlash[2] = "\\";
    ReplaceChars(slash, backSlash);
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDirectory Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::AddDirectory(const NFmiString &theDirectory)
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += Path();
    newStr += theDirectory;
    newStr += kFmiDirectorySeparator;
    newStr += Header();
    if (HasExtension())
    {
      newStr += NFmiString(".");
      newStr += Extension();
    }
    *this = newStr;
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

void NFmiFileString::DeleteDirectory()
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += PathWithoutLastDirectory();
    newStr += Header();
    if (HasExtension())
    {
      newStr += NFmiString(".");
      newStr += Extension();
    }
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDirectory Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::ReplaceDirectory(const NFmiString &theDirectory)
{
  try
  {
    NFmiString newStr(60);

    newStr = Device();
    newStr += PathWithoutLastDirectory();
    newStr += theDirectory;
    newStr += kFmiDirectorySeparator;
    newStr += Header();
    if (HasExtension())
    {
      newStr += NFmiString(".");
      newStr += Extension();
    }
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theExt Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Extension(const char *theExt)
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += Path();
    newStr += Header();
    newStr += ".";
    newStr += theExt;
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theExtension Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::FileName(const NFmiString &theExtension)
{
  try
  {
    FileName(static_cast<char *>(theExtension));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::FileName(const char *theFileName)
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += Path();
    newStr += theFileName;
    *this = newStr;
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

void NFmiFileString::Header(const NFmiString &theExtension)
{
  try
  {
    Header(theExtension.CharPtr());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiFileString::Header(const char *theHeader)
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += Path();
    newStr += theHeader;
    if (HasExtension())
    {
      newStr += ".";
      newStr += Extension();
    }
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param thePath Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Path(const char *thePath)
{
  try
  {
    NFmiString newStr(40);

    newStr = Device();
    newStr += thePath;
    if (newStr.GetChars(newStr.GetLen(), 1) != NFmiString(kFmiDirectorySeparator))
      newStr += kFmiDirectorySeparator;

    newStr += FileName();
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDevice Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Device(const char *theDevice)
{
  try
  {
    NFmiString newStr(40);

    newStr = theDevice;
    newStr += Path();
    newStr += FileName();
    *this = newStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiFileString::Device(const NFmiString &theExtension)
{
  try
  {
    Device(theExtension.CharPtr());
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

void NFmiFileString::ChangeScandinavian()
{
  try
  {
    NFmiString skand("äöåÄÖÅ");
    NFmiString notSkand("aoaAOA");
    for (unsigned int ind = 1; ind <= skand.GetLen(); ind++)
      ReplaceChars(reinterpret_cast<unsigned char *>(skand.GetCharsPtr(ind, 1)),
                   reinterpret_cast<unsigned char *>(notSkand.GetCharsPtr(ind, 1)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
